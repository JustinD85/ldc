//===-- abi-ppc64.cpp -----------------------------------------------------===//
//
//                         LDC - the LLVM D compiler
//
// This file is distributed under the BSD-style LDC license. See the LICENSE
// file for details.
//
//===----------------------------------------------------------------------===//
//
// The ABI implementation used for 64 bit little-endian PowerPC targets.
//
// The PowerOpen 64bit ELF v2 ABI can be found here:
// https://members.openpowerfoundation.org/document/dl/576
//===----------------------------------------------------------------------===//

#include "gen/abi.h"
#include "gen/abi-generic.h"
#include "gen/abi-ppc64le.h"
#include "gen/dvalue.h"
#include "gen/irstate.h"
#include "gen/llvmhelpers.h"
#include "gen/tollvm.h"

struct PPC64LETargetABI : TargetABI {
  HFAToArray hfaToArray;
  CompositeToArray64 compositeToArray64;
  IntegerRewrite integerRewrite;

  explicit PPC64LETargetABI() : hfaToArray(8) {}

  bool returnInArg(TypeFunction *tf, bool) override {
    if (tf->isref) {
      return false;
    }

    Type *rt = tf->next->toBasetype();

    if (!isPOD(rt))
      return true;

    return passByVal(tf, rt);
  }

  bool passByVal(TypeFunction *, Type *t) override {
    t = t->toBasetype();
    return t->ty == Tsarray || (t->ty == Tstruct && t->size() > 16 &&
                                !isHFA((TypeStruct *)t, nullptr, 8));
  }

  void rewriteFunctionType(IrFuncTy &fty) override {
    // return value
    if (!fty.ret->byref) {
      rewriteArgument(fty, *fty.ret);
    }

    // explicit parameters
    for (auto arg : fty.args) {
      if (!arg->byref) {
        rewriteArgument(fty, *arg);
      }
    }
  }

  void rewriteArgument(IrFuncTy &fty, IrFuncTyArg &arg) override {
    Type *ty = arg.type->toBasetype();
    if (ty->ty == Tstruct || ty->ty == Tsarray) {
      if (ty->ty == Tstruct && isHFA((TypeStruct *)ty, &arg.ltype, 8)) {
        hfaToArray.applyTo(arg, arg.ltype);
      } else if (canRewriteAsInt(ty, true)) {
        integerRewrite.applyTo(arg);
      } else {
        compositeToArray64.applyTo(arg);
      }
    } else if (ty->isintegral()) {
      arg.attrs.add(ty->isunsigned() ? LLAttribute::ZExt : LLAttribute::SExt);
    }
  }
};

// The public getter for abi.cpp
TargetABI *getPPC64LETargetABI() { return new PPC64LETargetABI(); }

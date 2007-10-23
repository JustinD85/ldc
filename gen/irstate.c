/* DMDFE backend stubs
 * This file contains the implementations of the backend routines.
 * For dmdfe these do nothing but print a message saying the module
 * has been parsed. Substitute your own behaviors for these routimes.
 */

#include <cstdarg>

#include "gen/llvm.h"

#include "mtype.h"
#include "declaration.h"

#include "gen/irstate.h"
#include "tollvm.h"

IRState* gIR = 0;
llvm::TargetData* gTargetData = 0;

//////////////////////////////////////////////////////////////////////////////////////////
IRScope::IRScope()
{
    begin = end = NULL;
}

IRScope::IRScope(llvm::BasicBlock* b, llvm::BasicBlock* e)
{
    begin = b;
    end = e;
    builder.SetInsertPoint(b);
}

//////////////////////////////////////////////////////////////////////////////////////////
IRState::IRState()
{
    dmodule = 0;
    module = 0;
    emitMain = false;
    mainFunc = 0;
    ir.state = this;
}

IRFunction& IRState::func()
{
    assert(!functions.empty() && "Function stack is empty!");
    return functions.back();
}

llvm::Function* IRState::topfunc()
{
    assert(!functions.empty() && "Function stack is empty!");
    return functions.back().func;
}

TypeFunction* IRState::topfunctype()
{
    assert(!functions.empty() && "Function stack is empty!");
    return functions.back().type;
}

llvm::Instruction* IRState::topallocapoint()
{
    assert(!functions.empty() && "AllocaPoint stack is empty!");
    return functions.back().allocapoint;
}

IRStruct& IRState::topstruct()
{
    assert(!structs.empty() && "Struct vector is empty!");
    return structs.back();
}

IRExp* IRState::topexp()
{
    return exps.empty() ? NULL : &exps.back();
}

IRScope& IRState::scope()
{
    assert(!scopes.empty());
    return scopes.back();
}

llvm::BasicBlock* IRState::scopebb()
{
    return scopebegin();
}
llvm::BasicBlock* IRState::scopebegin()
{
    IRScope& s = scope();
    assert(s.begin);
    return s.begin;
}
llvm::BasicBlock* IRState::scopeend()
{
    IRScope& s = scope();
    assert(s.end);
    return s.end;
}
bool IRState::scopereturned()
{
    //return scope().returned;
    return !scopebb()->empty() && scopebb()->back().isTerminator();
}

//////////////////////////////////////////////////////////////////////////////////////////

IRStruct::IRStruct()
 : recty(llvm::OpaqueType::get())
{
    type = 0;
    queueFuncs = true;
}

IRStruct::IRStruct(Type* t)
 : recty(llvm::OpaqueType::get())
{
    type = t;
    queueFuncs = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

IRFinally::IRFinally()
 : bb(NULL), ret(false), retval(NULL)
{
}

IRFinally::IRFinally(llvm::BasicBlock* b)
 : bb(b), ret(false), retval(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////////////////

LLVMBuilder* IRBuilderHelper::operator->()
{
    LLVMBuilder& b = state->scope().builder;
    assert(b.GetInsertBlock() != NULL);
    return &b;
}

//////////////////////////////////////////////////////////////////////////////////////////

IRFunction::IRFunction(FuncDeclaration* fd)
{
    decl = fd;
    Type* t = LLVM_DtoDType(fd->type);
    assert(t->ty == Tfunction);
    type = (TypeFunction*)t;
    func = NULL;
    allocapoint = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

IRExp::IRExp()
{
    e1 = e2 = NULL;
    v = NULL;
}

IRExp::IRExp(Expression* l, Expression* r, llvm::Value* val)
{
    e1 = l;
    e2 = r;
    v = val;
}

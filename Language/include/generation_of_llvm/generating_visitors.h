#pragma once
#include <visitors.h>
#include <llvm_libs.h>
#if 0
template<typename TypeToVisit>
struct generating_visitor : public visitor
{
	virtual void visit(llvm::LLVMContext& ctx, llvm::Module& mod, llvm::IRBuilder<>& builder_, TypeToVisit);
};

#endif
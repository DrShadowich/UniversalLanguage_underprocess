#pragma once
#include <llvm_libs.h>
#include <ulstring.h>
namespace ul::codegen
{
	namespace detail
	{
		struct trio_blocks
		{
			llvm::BasicBlock* then_bb;
			llvm::BasicBlock* else_bb;
			llvm::BasicBlock* merge_bb;
		};
	}
	llvm::StringRef nameof(llvm::Value*);
	llvm::Type* typeof(llvm::Value*);
	llvm::FunctionType* typeof(llvm::Function*);
	llvm::FunctionType* typeof(llvm::FunctionCallee&);

	bool both_is_integers(llvm::Value*, llvm::Value*);
	// First found pointer will be returned
	llvm::Value* any_is_pointer(llvm::Value*, llvm::Value*);

	// Create basic blocks
	// 1. then
	// 2. else
	// 3. merge
	detail::trio_blocks make_then_blocks(llvm::LLVMContext& ctx, llvm::Function* parent, utils::classes::stringi8 name = "");

	llvm::Value* create_string(llvm::Module& mod_, llvm::IRBuilder<>& builder, llvm::LLVMContext& ctx, utils::classes::stringi8 cpp_string);
}
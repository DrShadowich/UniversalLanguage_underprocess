#include <llvm+.h>
namespace ul::codegen
{
	llvm::StringRef nameof(llvm::Value* val)
	{
		return val->getName();
	}

	llvm::Type* typeof(llvm::Value* val)
	{
		return val->getType();
	}

	llvm::FunctionType* typeof(llvm::Function* fn)
	{
		return fn->getFunctionType();
	}
	
	llvm::FunctionType* typeof(llvm::FunctionCallee& fnc)
	{
		return fnc.getFunctionType();
	}

	bool both_is_integers(llvm::Value* left, llvm::Value* right)
	{
		return typeof(left)->isIntegerTy() && typeof(right)->isIntegerTy();
	}
	llvm::Value* any_is_pointer(llvm::Value* left, llvm::Value* right)
	{
		if (typeof(left)->isPointerTy() && not typeof(right)->isPointerTy())
		{
			return left;
		}
		else if (not typeof(left)->isPointerTy() && typeof(right)->isPointerTy())
		{
			return right;
		}
		else
			return nullptr;
	}

	detail::trio_blocks make_then_blocks(llvm::LLVMContext& ctx, llvm::Function* parent, std::string name)
	{
		return { llvm::BasicBlock::Create(ctx, name + "_then", parent),
		llvm::BasicBlock::Create(ctx, name + "_else", parent),
		llvm::BasicBlock::Create(ctx, name + "_merge", parent)};
	}
}
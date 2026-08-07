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

	detail::trio_blocks make_then_blocks(llvm::LLVMContext& ctx, llvm::Function* parent, utils::classes::stringi8 name)
	{
		return { llvm::BasicBlock::Create(ctx, name + "_then", parent),
		llvm::BasicBlock::Create(ctx, name + "_else", parent),
		llvm::BasicBlock::Create(ctx, name + "_merge", parent)};
	}

	llvm::Value* create_string(llvm::Module& mod_, llvm::IRBuilder<>& builder, llvm::LLVMContext& ctx, utils::classes::stringi8 cpp_string)
	{
		auto str_type = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), cpp_string.size() + 1);
		auto str_init = llvm::ConstantDataArray::getString(ctx, cpp_string);
		auto literal_global = new llvm::GlobalVariable
		{
			mod_, str_type, true, llvm::GlobalValue::LinkageTypes::PrivateLinkage, str_init
		};
		literal_global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

		llvm::Value* idx0 = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0);
		llvm::Value* indices[] = { idx0, idx0 };
		llvm::Value* string_ptr = builder.CreateInBoundsGEP(str_type, literal_global, llvm::ArrayRef<llvm::Value*>(indices, 2));
		return string_ptr;
	}
}
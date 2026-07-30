#include <llvm_libs.h>
namespace ul::codegen
{
	struct llvm_aligned_type
	{
		llvm::Type* type;
		llvm::MaybeAlign align;
	};

	struct expanded_llvm_function
	{
		llvm::Function* function;
		bool is_extern;
	};

	// Return type for code_generator
	union llvm_union
	{
		llvm::Value* value;
		llvm::Function* function_info;
		llvm::FunctionCallee f_callee;
		llvm::FunctionType* f_type;
		llvm::Type* type;
		llvm_aligned_type aligned_type;
		llvm::Constant* constant;
		llvm::AllocaInst* alloca_instance;
		llvm_union()
		{}
	};
}
#include <llvm_libs.h>
namespace ul::codegen
{
	struct expanded_llvm_value
	{
		llvm::Value* value;
		bool is_return_value;
	};

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
		expanded_llvm_value value;
		llvm::Function* function_info;
		llvm::FunctionCallee f_callee;
		llvm::FunctionType* f_type;
		llvm::Type* type;
		llvm_aligned_type aligned_type;
		llvm::Constant* constant;
		llvm::AllocaInst* alloca_instance;
		std::vector<llvm::Argument*>* param_list;
		std::vector<llvm::Value*>* args_list;
		llvm_union()
		{}
	};
}
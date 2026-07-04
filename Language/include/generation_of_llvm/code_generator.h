#pragma once
#include <llvm_libs.h>
#include <parser.h>
#include <visitors.h>
#include <name_table.h>
#include <parser.h>

namespace ul::codegen
{
	class code_generator
	{
	private:
		utils::name_table variable_names_;
		utils::name_table type_names_;
		utils::name_table function_names_;

		std::vector<std::unique_ptr<stmt::statement>> AST_;

		llvm::LLVMContext& ctx_;
		llvm::Module& module_;
		llvm::IRBuilder<> builder_;
	public:
		code_generator() = default;
		code_generator(llvm::LLVMContext& ctx, llvm::Module& module, std::vector<std::unique_ptr<stmt::statement>> AST);
		void generate_statements();
		void* generate_expression(expr::expr_node_ptr expr);
	};
}
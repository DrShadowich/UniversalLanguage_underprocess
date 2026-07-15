#pragma once
#include <llvm_union_info.h>
#include <llvm_libs.h>
#include <parser.h>
#include <visitors.h>
#include <name_table.h>

namespace ul::codegen
{
	class code_generator
	{
	private:
		utils::name_table names_table_;

		std::vector<std::unique_ptr<stmt::statement>> AST_;

		llvm::LLVMContext& ctx_;
		llvm::Module& module_;
		llvm::IRBuilder<> builder_;

		uint32_t bb_counter_{};
	public:
		code_generator() = default;
		code_generator(llvm::LLVMContext& ctx, llvm::Module& module, std::vector<std::unique_ptr<stmt::statement>> AST);
		void generate_statements();
		~code_generator();
	private:
		llvm_union generate_expression(expr::expr_node_ptr expr);
		llvm_union generate_statement(stmt::statement_ptr stmt);
		llvm_aligned_type get_aligned_type(const std::string& name);
		void add_depth();
		void sub_depth();
	};
}
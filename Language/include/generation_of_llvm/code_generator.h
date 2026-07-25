#pragma once
#include <llvm_union_info.h>
#include <llvm_libs.h>
#include <parser.h>
#include <visitors.h>
#include <name_table.h>

namespace ul::codegen
{

	struct generator_context
	{
		llvm::BasicBlock* if_statement_space;
		llvm::BasicBlock* current_function_space;
		bool is_return_value = false;
	};

	class code_generator
	{
	private:
		utils::name_table names_table_;

		std::vector<std::unique_ptr<stmt::statement>> AST_;

		llvm::LLVMContext& ctx_;
		llvm::Module& module_;
		llvm::IRBuilder<> builder_;

		generator_context gctx_;

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
		std::pair<llvm::Value*, llvm::Value*> get_binary_ops_operands(expr::expr_node_ptr first, expr::expr_node_ptr second);
		void add_depth();
		void sub_depth();
	};
}
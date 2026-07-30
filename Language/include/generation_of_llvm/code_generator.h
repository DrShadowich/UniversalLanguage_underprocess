#pragma once
#include <llvm_union_info.h>
#include <llvm_libs.h>
#include <parser.h>
#include <visitors.h>
#include <name_table.h>
#include <stack>
#include <marker_generator.h>

namespace ul::codegen
{

	struct generator_context
	{
		llvm::Value* left_buffer = nullptr;
		llvm::BasicBlock* global_space = nullptr;
		llvm::BasicBlock* if_statement_space = nullptr;
		llvm::BasicBlock* current_function_space = nullptr;
		uint32_t inner_block = 0;
		bool is_return_value = false;
		bool is_named_statement = false;
		std::stack<llvm::BasicBlock*> loop_spaces;
		std::vector<llvm::Argument*> parameters_buffer;
		std::vector<llvm::Value*> arguments_buffer;
	};

	class code_generator
	{
	private:
		utils::name_table names_table_;

		std::vector<std::unique_ptr<stmt::statement>> AST_;

		llvm::LLVMContext& ctx_;
		llvm::Module& module_;
		llvm::IRBuilder<> builder_;

		std::unique_ptr<generator_context> gctx_;

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

		void on_return_value(llvm_union ua);
		void add_depth();
		void sub_depth();
	};
}
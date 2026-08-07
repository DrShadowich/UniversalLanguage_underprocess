#pragma once
#include <vector>
#include <expression_info.h>


namespace ul::stmt
{
	struct statement
	{
		virtual ~statement() = default;
	};
	using statement_ptr = std::unique_ptr<statement>;

	struct block_statement : public statement
	{
		std::vector<std::unique_ptr<statement>> statements;
	};
	using block_statement_ptr = std::unique_ptr<block_statement>;

	struct assignment_statement : public statement
	{
		utils::classes::stringi8 variable_name;
		expr::expr_node_ptr rhs_value;
		assignment_statement(utils::classes::stringi8 value_name, expr::expr_node_ptr expr_rhs) :
			variable_name{ std::move(value_name) }, rhs_value{ std::move(expr_rhs) }
		{}
	};
	using assignment_statement_ptr = std::unique_ptr< assignment_statement>;

	struct function_definition : public statement
	{
		std::unique_ptr<block_statement> inner_stmt;
		expr::function_definition_node_ptr function_def;
		explicit function_definition(expr::function_definition_node_ptr func_def, std::unique_ptr<block_statement> block_stmt) :
			inner_stmt{ std::move(block_stmt) }, function_def{ std::move(func_def) } 
		{}
	};
	using function_definition_ptr = std::unique_ptr< function_definition>;

	struct extern_function_declaration : public statement
	{
		expr::function_definition_node_ptr function_def;
		explicit extern_function_declaration(expr::function_definition_node_ptr function_definition) :
			function_def{ std::move(function_definition) }
		{}
	};
	using extern_function_declaration_ptr = std::unique_ptr<extern_function_declaration>;
	struct expression_statement : public statement
	{
		expr::expr_node_ptr expression;
		explicit expression_statement(expr::expr_node_ptr e) :
			expression{ std::move(e) }
		{}
	};
	using expression_statement_ptr = std::unique_ptr<expression_statement>;

	struct return_statement : public statement
	{
		expr::expr_node_ptr return_expression;
		utils::classes::stringi8 type_str;
		explicit return_statement(expr::expr_node_ptr re) :
			return_expression{ std::move(re) }
		{}
		explicit return_statement(expr::expr_node_ptr e, utils::classes::stringi8 return_type) :
			return_expression{ std::move(e) }, type_str{ std::move(return_type) }
		{}
	};
	using return_statement_ptr = std::unique_ptr<return_statement>;
	
	struct elif_statement;

	struct else_statement : public statement
	{
		statement_ptr inner_stmt;
		std::unique_ptr<else_statement> next_cond_stmt;
		explicit else_statement(statement_ptr basic_block) :
			inner_stmt{ std::move(basic_block) }
		{}
		explicit else_statement(std::unique_ptr<else_statement> me, std::unique_ptr<else_statement> next_cond) :
			inner_stmt{ std::move(me->inner_stmt) }, next_cond_stmt{ std::move(next_cond) }
		{}
	};
	using else_statement_ptr = std::unique_ptr<else_statement>;
	
	struct elif_statement : public else_statement
	{
		expr::expr_node_ptr condition;

		explicit elif_statement(expr::expr_node_ptr condition_, statement_ptr basic_block) :
			condition{ std::move(condition_) }, else_statement{std::move(basic_block)}
		{
			next_cond_stmt = nullptr;
		}
		explicit elif_statement(expr::expr_node_ptr condition_, statement_ptr basic_block, else_statement_ptr next_cond) :
			condition{ std::move(condition_) }, else_statement{std::move(basic_block)}
		{
			next_cond_stmt = std::move(next_cond);
		}
		explicit elif_statement(std::unique_ptr<elif_statement> me, else_statement_ptr next_cond) :
			condition{ std::move(me->condition) }, else_statement{ std::move(me->inner_stmt) }
		{
			next_cond_stmt = std::move(next_cond);
		}
	};
	using elif_statement_ptr = std::unique_ptr<elif_statement>;

	struct if_statement : public statement
	{
		expr::expr_node_ptr condition;
		statement_ptr inner_stmt;
		else_statement_ptr next_cond_stmt;
		explicit if_statement(expr::expr_node_ptr condition_, statement_ptr basic_block) :
			condition{ std::move(condition_) }, inner_stmt{ std::move(basic_block) }, next_cond_stmt{ nullptr }
		{}
		explicit if_statement(expr::expr_node_ptr condition_, statement_ptr basic_block, else_statement_ptr next_cond) :
			condition{ std::move(condition_) }, inner_stmt{ std::move(basic_block) }, next_cond_stmt{ std::move(next_cond) }
		{}
	};
	using if_statement_ptr = std::unique_ptr<if_statement>;

	struct loop_statement : public statement
	{
		statement_ptr inner_stmt;
		explicit loop_statement(loop_statement&&) = default;
		explicit loop_statement(std::unique_ptr<loop_statement> loop_ptr) :
			loop_statement{ std::move(*loop_ptr) }
		{}
		explicit loop_statement(statement_ptr basic_block) :
			inner_stmt{ std::move(basic_block) }
		{}
	};
	using loop_statement_ptr = std::unique_ptr<loop_statement>;

	struct while_loop_statement : public loop_statement
	{
		expr::expr_node_ptr condition_expr;
		explicit while_loop_statement(while_loop_statement&&) = default;
		explicit while_loop_statement(std::unique_ptr<while_loop_statement> wloop_ptr) :
			while_loop_statement{ std::move(*wloop_ptr) }
		{}
		explicit while_loop_statement(statement_ptr basic_block, expr::expr_node_ptr condition = nullptr) :
			condition_expr{ std::move(condition) }, loop_statement{ std::move(basic_block) }
		{}
		explicit while_loop_statement(loop_statement_ptr loop) :
			condition_expr{ nullptr }, loop_statement{ std::move(loop) }
		{}
	};
	using while_loop_statement_ptr = std::unique_ptr<while_loop_statement>;

	struct for_loop_statement : public while_loop_statement
	{
		std::vector<expr::variable_assignment_expr_ptr> value_definitions{};
		std::vector<expr::expr_node_ptr> value_interactions{};
		explicit for_loop_statement(statement_ptr basic_block,
			expr::expr_node_ptr condition = nullptr,
			std::vector<expr::variable_assignment_expr_ptr> value_defs = {},
			std::vector<expr::expr_node_ptr> value_inters = {}) :
			value_definitions{ std::move(value_defs) }, 
			value_interactions{ std::move(value_inters) },
			while_loop_statement{ std::move(basic_block), std::move(condition) }
		{}
		explicit for_loop_statement(while_loop_statement_ptr wloop) :
			value_definitions{}, value_interactions{}, while_loop_statement{ std::move(wloop) }
		{}
	};
	using for_loop_statement_ptr = std::unique_ptr<for_loop_statement>;

	struct insert_statement : public statement
	{
		utils::classes::stringi8 file_name;
		explicit insert_statement(utils::classes::stringi8 filen) :
			file_name{ std::move(filen) }
		{}
	};
	using insert_statement_ptr = std::unique_ptr<insert_statement>;
	struct marker_statement : public statement
	{
		utils::classes::stringi8 header;
		utils::classes::stringi8 body;
		utils::classes::stringi8 footer;
		ul::token::TID marker_type;
		explicit marker_statement(utils::classes::stringi8 head, utils::classes::stringi8 bod, utils::classes::stringi8 foot, ul::token::TID mid) :
			header{ std::move(head) }, body{ std::move(bod) }, footer{ std::move(foot) }, marker_type{ mid }
		{}
	};
	using marker_statement_ptr = std::unique_ptr<marker_statement>;

	struct end_of_block_statement : public statement
	{
		std::vector<expr::expr_node_ptr> after_expressions;
		explicit end_of_block_statement(std::vector<expr::expr_node_ptr> after_exprs = {}) :
			after_expressions{ std::move(after_exprs) }
		{}
	};

	else_statement_ptr reverse_if_statements_list(else_statement_ptr root);
}
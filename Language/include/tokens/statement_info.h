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
		std::string variable_name;
		expr::expr_node_ptr rhs_value;
		assignment_statement(std::string value_name, expr::expr_node_ptr expr_rhs) :
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
		std::string type_str;
		explicit return_statement(expr::expr_node_ptr re) :
			return_expression{ std::move(re) }
		{}
		explicit return_statement(expr::expr_node_ptr e, std::string return_type) :
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

	else_statement_ptr reverse_if_statements_list(else_statement_ptr root);
}
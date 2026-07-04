#pragma once
#include <vector>
#include <expression_info.h>


namespace ul::stmt
{
	struct statement
	{
		virtual ~statement() = default;
	};

	struct block_statement : public statement
	{
		std::vector<std::unique_ptr<statement>> statements;
	};

	struct assignment_statement : public statement
	{
		std::string variable_name;
		expr::expr_node_ptr rhs_value;
		assignment_statement(std::string value_name, expr::expr_node_ptr expr_rhs) :
			variable_name{ std::move(value_name) }, rhs_value{ std::move(expr_rhs) }
		{}
	};

	struct function_definition : public statement
	{
		std::unique_ptr<block_statement> inner_stmt;
		std::unique_ptr<expr::function_call_node> function_def;
		explicit function_definition(std::unique_ptr<expr::function_call_node> func_def, std::unique_ptr<block_statement> block_stmt) :
			inner_stmt{ std::move(block_stmt) }, function_def{ std::move(func_def) } 
		{}
	};

	struct expression_statement : public statement
	{
		expr::expr_node_ptr expression;
		expression_statement(expr::expr_node_ptr e) :
			expression{ std::move(e) }
		{}
	};
}
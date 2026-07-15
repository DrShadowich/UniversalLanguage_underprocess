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
		expression_statement(expr::expr_node_ptr e) :
			expression{ std::move(e) }
		{}
	};
	using expression_statement_ptr = std::unique_ptr<expression_statement>;
}
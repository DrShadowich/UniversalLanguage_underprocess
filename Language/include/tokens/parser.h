#pragma once
#include <lexer.h>
#include <token_info.h>
#include <expression_info.h>
#include <statement_info.h>
#include <function_names_memory.h>
namespace ul::parser
{
	constexpr uint32_t max_priority = 300000000;
	class language_parser
	{
	private:
		lexer::language_lexer& lexer_;
		function_names_memory function_table_;
	public:
		explicit language_parser(lexer::language_lexer& lexer);
		// Create AST
		stmt::block_statement_ptr parse_program();
	private:
		// Main parse function.
		// Starts the whole AST's build.
		stmt::statement_ptr parse_statement();
		// Parse field call line.
		expr::field_call_node_ptr parse_field_call(expr::expr_node_ptr parent);
		// Get function_definition expression.
		expr::function_definition_node_ptr parse_function_definition_node();
		// 
		stmt::extern_function_declaration_ptr parse_extern_function();
		// Get function_definition statement.
		// Due huge importance it needs type stmt::statement_ptr instead stmt::function_definition_ptr
		stmt::statement_ptr parse_function_definition();
		// Get assignment statement.
		stmt::assignment_statement_ptr parse_assignment(std::string variable_name);
		// Get block statement aka basic block.
		stmt::block_statement_ptr parse_block_statement();
		// Get arguments
		expr::function_arguments_node_ptr parse_arguments();
		// Get params
		expr::function_parameters_node_ptr parse_parameters();
		// Make function call node ptr from function table
		expr::function_call_node_ptr get_function_call_node(expr::function_node_ptr function_node, expr::function_arguments_node_ptr args);
		// Table of token priority
		uint32_t lbp(token::token_type& tt);
		// New definition
		expr::expr_node_ptr nud(token::token_info& ti);
		// Left denotation
		expr::expr_node_ptr led(token::token_info& ti, expr::expr_node_ptr left);
		expr::expr_node_ptr expression(uint32_t rbp);
		// Whether the token is starting expression
		bool is_expression_start(const token::token_type& tt);
	};
}
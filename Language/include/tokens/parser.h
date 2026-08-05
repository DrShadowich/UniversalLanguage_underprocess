#pragma once
#include <lexer.h>
#include <token_info.h>
#include <expression_info.h>
#include <statement_info.h>
#include <function_names_memory.h>
#include <source_manager.h>
#include <marker_parser.h>

#define OUT_PARSER_EXCEPTION(message)	(throw ul::ex::parser_exception{ (message), "", (source_manager_.get_line_and_column(lexer_.get_current_offset())) })

namespace ul::parser
{
	struct parser_context
	{
		std::string last_expression_lexemes{};
		expr::function_definition_node* current_function{ nullptr };
		std::unordered_map<std::string, uint32_t> current_names{};
		bool in_function = false;
		bool in_if_statement = false;
		uint32_t loop_depth{ 0 };
		uint32_t block_depth{ 0 };
		uint32_t bracket_depth{ 0 };
		parser_context()
		{}
	};
	/*
	* Сделать таблицу для переменных, тобишь scope
	*/
	class language_parser
	{
	private:
		lexer::language_lexer& lexer_;
		source::source_manager source_manager_;
		function_names_memory function_table_;
		std::unique_ptr<parser_context> pctx_;
		std::unique_ptr<marker_parser> marker_parser_;
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
		// Get block statement aka basic block.
		stmt::block_statement_ptr parse_block_statement();
		// Parse inner basic block for loops and conditions
		stmt::statement_ptr parse_inner_statement();
		// Get arguments
		expr::function_arguments_node_ptr parse_arguments();
		// Get params
		expr::function_parameters_node_ptr parse_parameters();
		// Make function call node ptr from function table
		expr::function_call_node_ptr get_function_call_node(expr::function_node_ptr function_node, expr::function_arguments_node_ptr args);

		std::vector<expr::variable_assignment_expr_ptr> parse_value_definitions();
		std::vector<expr::expr_node_ptr> parse_value_differences();

		template<typename IfStatement>
		std::unique_ptr<IfStatement> parse_if_statement(expr::expr_node_ptr condition)
		{
			std::unique_ptr<IfStatement> if_stmt =
				std::make_unique<IfStatement>(std::move(condition), parse_inner_statement());
			return if_stmt;
		}
		
		stmt::else_statement_ptr parse_else_statement();

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
#pragma once
#include <lexer.h>
#include <token_info.h>
#include <expression_info.h>
#include <statement_info.h>
#include <function_names_memory.h>
#include <source_manager.h>

#define OUT_PARSER_EXCEPTION(message)	(throw ul::ex::parser_exception{ (message), "", (source_manager_.get_line_and_column(lexer_.get_current_offset())) })

namespace ul::parser
{
	struct parser_context
	{
		expr::function_definition_node* current_function{ nullptr };
		parser_context()
		{}
	};

	constexpr uint32_t max_priority = 300000000;
	class language_parser
	{
	private:
		lexer::language_lexer& lexer_;
		source::source_manager source_manager_;
		function_names_memory function_table_;
		parser_context parser_ctx_;
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

		template<typename IfStatement>
		std::unique_ptr<IfStatement> parse_if_statement(expr::expr_node_ptr condition)
		{
			std::unique_ptr<IfStatement> if_stmt{ nullptr };
			if (lexer_.not_expect(token::TID::FLBRACKET))
			{
				auto return_block = std::make_unique<stmt::expression_statement>(expression(0));
				if_stmt = std::make_unique<IfStatement>(std::move(condition), std::move(return_block));
				if (lexer_.not_expect(token::TID::SEMICOLON))
					OUT_PARSER_EXCEPTION("Expected \';\' after one line then statement");
				lexer_.next();
			}
			else
			{
				auto return_block = parse_block_statement();
				if_stmt = std::make_unique<IfStatement>(std::move(condition), std::move(return_block));
			}
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
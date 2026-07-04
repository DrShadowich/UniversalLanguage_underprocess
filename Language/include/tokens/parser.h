#pragma once
#include <lexer.h>
#include <token_info.h>
#include <expression_info.h>
#include <statement_info.h>
namespace ul::parser
{
	constexpr uint32_t max_priority = 300000000;
	class language_parser
	{
	private:
		lexer::language_lexer& lexer_;
	public:
		explicit language_parser(lexer::language_lexer& lexer);
		expr::expr_node_ptr parse();
		std::unique_ptr<stmt::block_statement> parse_program();
	private:
		std::unique_ptr<stmt::statement> parse_statement();
		// Table of token priority
		uint32_t lbp(token::token_type& tt);
		// New definition
		expr::expr_node_ptr nud(token::token_info& ti);
		// Left denotation
		expr::expr_node_ptr led(token::token_info& ti, expr::expr_node_ptr left);
		expr::expr_node_ptr expression(uint32_t rbp);
		expr::field_call_node_ptr parse_field_call(expr::expr_node_ptr parent);
		// Whether the token is starting expression
		bool is_expression_start(const token::token_type& tt);
	};
}
#include <parser.h>
#include <expression_info.h>

namespace ul::parser
{

	language_parser::language_parser(lexer::language_lexer& l) :
		lexer_{ l }
	{
	}
	uint32_t language_parser::lbp(token::token_type& tt)
	{
		using namespace token;
		switch (tt.enum_type)
		{
		case TYPE_TOKEN_TYPE::ASSIGNMENT_OPERATOR:
			return 1;
		case TYPE_TOKEN_TYPE::PLUS_OPERATOR:
		case TYPE_TOKEN_TYPE::MINUS_OPERATOR:
			return 2;
		case TYPE_TOKEN_TYPE::SLASH_OPERATOR:
		case TYPE_TOKEN_TYPE::STAR_OPERATOR:
			return 3;
		default:
			return 0;
		}
	}

	bool language_parser::is_expression_start(const token::token_type& tt)
	{
		using namespace token;
		switch (tt.enum_type)
		{
		case TYPE_TOKEN_TYPE::NUMBER_LITERAL:
		case TYPE_TOKEN_TYPE::STRING_LITERAL:
		case TYPE_TOKEN_TYPE::MINUS_OPERATOR:
		case TYPE_TOKEN_TYPE::PLUS_OPERATOR:
		case TYPE_TOKEN_TYPE::LBRACKET:
		case TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER:
		case TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER:
		case TYPE_TOKEN_TYPE::NEWLINE:
		case TYPE_TOKEN_TYPE::MARKER_EXPRESSION:
			return true;
		default:
			return false;
		}
	}

	std::unique_ptr<stmt::statement> language_parser::parse_statement()
	{
		if (lexer_.expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
		{
			lexer_.next();
			auto block = std::make_unique<stmt::block_statement>();
			while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET) &&
				lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
			{
				block->statements.push_back(parse_statement());
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON)) { lexer_.next(); }
			}
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET))
			{
				PARSER_EXCEPTION("Expected \'}\' to close block");
			}
			lexer_.next();
			return block;
		}
		// Bad
		if (lexer_.expect(token::TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER))
		{
			std::string var_name = lexer_.front()->lexeme;
			auto&& function_ = std::make_unique<expr::function_node>(std::move(var_name));
			lexer_.next();
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::LBRACKET))
			{
				lexer_.next();
				auto&& params = std::make_unique<expr::function_arguments_node>();
				while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::END))
						PARSER_EXCEPTION("Unexpected EOF in argument list");
					params->args.push_back(expression(0));
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::COMMA))
						lexer_.next();
				}
				if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					PARSER_EXCEPTION("Expected \')\' to close argument list");
				}
				lexer_.next();
				auto&& function_with_params = std::make_unique<expr::function_call_node>(std::move(function_), std::move(params));
				// func_fn(...);
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
				{
					lexer_.next();
					return std::make_unique<stmt::expression_statement>(std::move(function_with_params));
				}
				while(lexer_.expect(token::TYPE_TOKEN_TYPE::NEWLINE) && lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
				{
					lexer_.next();
				}
				// func_fn(...){ ... }
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
				{
					lexer_.next();
					auto block = std::make_unique<stmt::block_statement>();
					while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET) &&
						lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
					{
						block->statements.push_back(parse_statement());
						if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON)) { lexer_.next(); }
					}
					if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET))
					{
						PARSER_EXCEPTION("Expected \'}\' to close function definition");
					}
					lexer_.next();
					return std::make_unique<stmt::function_definition>(std::move(function_with_params), std::move(block));
				}
				PARSER_EXCEPTION("Expecting semicolon or definition after function name with args/params");
			}
		}
		//
		// Bad
		if (lexer_.expect(token::TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER))
		{
			std::string var_name = lexer_.front()->lexeme;
			auto&& parent = nud(*lexer_.front());
			lexer_.next();
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::ASSIGNMENT_OPERATOR))
			{
				lexer_.next();
				auto&& value_expr = expression(0);
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
				{
					lexer_.next();
					return std::make_unique<stmt::assignment_statement>(std::move(var_name), std::move(value_expr));
				}
			}
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
			{
				lexer_.next();
				return std::make_unique<stmt::expression_statement>(nud(*lexer_.front()));
			}
			/*for(auto&& dot = parse_field_call(); dot.get(); dot = parse_field_call())
			{
				return std::make_unique<stmt::expression_statement>(std::move(dot));
			}*/
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::POINT))
			{
				return std::make_unique<stmt::expression_statement>(std::move(parse_field_call(std::move(parent))));
			}
			PARSER_EXCEPTION("Variable has no purpose");
		}
		//
		if (is_expression_start(lexer_.front()->type))
		{
			auto expr = expression(0);
			return std::make_unique<stmt::expression_statement>(std::move(expr));
		}
		PARSER_EXCEPTION("Unexpected token starting statement");
	}

	expr::field_call_node_ptr language_parser::parse_field_call(expr::expr_node_ptr parent)
	{
		expr::field_call_node_ptr field{ nullptr };
		while (lexer_.expect(token::TYPE_TOKEN_TYPE::POINT))
		{
			lexer_.next();
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER) &&
				lexer_.not_expect(token::TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER)
				)
			{
				PARSER_EXCEPTION("Expected some field after \'.\'");
			}
			auto&& field_n = expression(0);
			lexer_.next();

			field = field ? std::make_unique<expr::field_call_node>(std::move(parent), std::move(field_n), std::move(field)) :
				std::make_unique<expr::field_call_node>(std::move(parent), std::move(field_n));
		}
		return field;
	}

	std::unique_ptr<stmt::block_statement> language_parser::parse_program()
	{
		auto&& program = std::make_unique<stmt::block_statement>();
		while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
		{
			program->statements.push_back(parse_statement());
		}
		return program;
	}

	expr::expr_node_ptr language_parser::nud(token::token_info& ti)
	{
		using namespace token;
		expr::expr_node_ptr left;
		switch (ti.type)
		{
		case TYPE_TOKEN_TYPE::NUMBER_LITERAL:
			left = std::make_unique<expr::number_literal_node>(ti.lexeme);
			break;
		case TYPE_TOKEN_TYPE::STRING_LITERAL:
			left = std::make_unique<expr::string_literal_node>(ti.lexeme);
			break;
		case TYPE_TOKEN_TYPE::MINUS_OPERATOR:
		{
			lexer_.next();
			expr::expr_node_ptr operand = expression(max_priority);
			left = std::make_unique<expr::unary_operator_node>(std::move(ti), std::move(operand));
			break;
		}
		case TYPE_TOKEN_TYPE::KEYWORD_RETURN:
			lexer_.next();
			left = expression(0);
			break;
		case TYPE_TOKEN_TYPE::PLUS_OPERATOR:
		{
			lexer_.next();
			auto inner = expression(0);
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
			{
				PARSER_EXCEPTION("Expected \';\' after return");
			}
			lexer_.next();
			left = std::make_unique<expr::return_value_node>(std::move(inner));
			break;
		}
		case TYPE_TOKEN_TYPE::LBRACKET:
		{
			lexer_.next();
			auto inner = expression(0);
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
			{
				PARSER_EXCEPTION(R"(Expected ')')");
			}
			lexer_.next();
			left = std::move(inner);
			break;
		}
		case TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER:
			left = std::make_unique<expr::variable_node>(ti.lexeme);
			break;
		case TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER:
		{
			std::unique_ptr<expr::function_node> function_ = std::make_unique<expr::function_node>(ti.lexeme);
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::LBRACKET))
			{
				lexer_.next();
				auto&& args = std::make_unique<expr::function_arguments_node>();
				while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::END))
						PARSER_EXCEPTION("Unexpected EOF in argument list");
					args->args.push_back(expression(0));
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::COMMA))
						lexer_.next();
				}
				if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					PARSER_EXCEPTION("Expected \')\' to close argument list");
				}
				lexer_.next();
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
					lexer_.next();
				left = std::make_unique<expr::function_call_node>(std::move(function_), std::move(args));
			}
			else
				left = std::move(function_);
			break;
		}
		case TYPE_TOKEN_TYPE::NEWLINE:
			left = std::make_unique<expr::newline_node>();
			break;
		case TYPE_TOKEN_TYPE::MARKER_EXPRESSION:
			left = std::make_unique<expr::marker_node>(std::move(ti));
			break;
		default:
			PARSER_EXCEPTION("Unexpected token");
		}

		for (;;)
		{
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::POINT))
				break;
			lexer_.next();
			if (lexer_.not_expect(TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER) &&
				lexer_.not_expect(TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER)
				)
			{
				PARSER_EXCEPTION("Expected some field after \'.\'");
			}
			auto&& field_n = expression(0);
			lexer_.next();
			left = std::make_unique<expr::field_call_node>(std::move(left), std::move(field_n));
		}
		return left;
	}
	expr::expr_node_ptr language_parser::led(token::token_info& ti, expr::expr_node_ptr left)
	{
		uint32_t right_binding_priority = lbp(ti.type);
		auto&& right = expression(right_binding_priority);
		return std::make_unique<expr::binary_operator_node>(ti, std::move(left), std::move(right));
	}
	expr::expr_node_ptr language_parser::expression(uint32_t right_binding_priority)
	{
		auto&& cur = lexer_.front();
		if (lexer_.is_now_break_symbol())
		{
			PARSER_EXCEPTION("Empty expression");
		}
		lexer_.next();
		auto&& left = nud(*cur);
		for (;;)
		{
			if (lexer_.is_now_break_symbol())
				break;
			auto&& cur2 = lexer_.front();
			uint32_t cur_left_binding_priority = lbp(cur2->type);
			if (right_binding_priority >= cur_left_binding_priority) break;
			lexer_.next();
			left = led(*cur2, std::move(left));
		}
		return left;
	}
	// Parse statement
	expr::expr_node_ptr language_parser::parse()
	{
		auto node = expression(0);
		if (!lexer_.is_end_symbol())
		{
			PARSER_EXCEPTION("Unexpected token after expression");
		}
		return node;
	}
}
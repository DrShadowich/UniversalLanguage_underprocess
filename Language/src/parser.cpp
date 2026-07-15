#include <parser.h>
#include <expression_info.h>
#include <visitors.h>

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
		case TYPE_TOKEN_TYPE::MARKER_EXPRESSION:
		case TYPE_TOKEN_TYPE::KEYWORD_RETURN:
		case TYPE_TOKEN_TYPE::TRIPLE_POINT:
			return true;
		default:
			return false;
		}
	}

	expr::function_definition_node_ptr language_parser::parse_function()
	{
		std::string function_name = lexer_.front()->lexeme;
		auto&& function_ = std::make_unique<expr::function_node>(std::move(function_name));
		lexer_.next();
		if (lexer_.expect(token::TYPE_TOKEN_TYPE::LBRACKET))
		{
			lexer_.next();
			bool va_args = false;
			auto params = std::make_unique<expr::function_parameters_node>();
			while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
			{
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::END))
					PARSER_EXCEPTION("Unexpected EOF in parameter list");
				auto&& future_param = nud(*lexer_.front());
				lexer_.next();
				if (auto* _ = dynamic_cast<expr::variable_node*>(future_param.get()))
				{
					expr::type_variable_node_ptr type =
						std::make_unique<expr::type_variable_node>((expr::variable_node*)future_param.release());
					std::string type_name = type->name;
					params->types.push_back(std::move(type));
					params->names.emplace_back(std::move(type_name));
				}
				else if (auto* b = dynamic_cast<expr::function_with_va_args_node*>(future_param.get()))
				{
					if (va_args)
						PARSER_EXCEPTION("Va args already defined");
					else va_args = true;
				}
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::COMMA))
					lexer_.next();
			}
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
			{
				PARSER_EXCEPTION("Expected \')\' to close parameters list");
			}
			lexer_.next();
			auto&& function_with_params =
				std::make_unique<expr::function_definition_node>(std::move(function_), std::move(params));
			function_with_params->function_type = std::make_unique<expr::type_node>("");
			function_with_params->va_args = va_args;
			return function_with_params;
		}
		else PARSER_EXCEPTION("Expected \'(\' to open parameters list");
	}

	stmt::statement_ptr language_parser::parse_statement()
	{
		// extern function_name(params) -> _type; || extern function_name(params);
		if (lexer_.expect(token::TYPE_TOKEN_TYPE::KEYWORD_EXTERN))
		{
			lexer_.next();
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER))
				PARSER_EXCEPTION("Expected function identifier after keyword extern");
			auto&& function_definition = parse_function();
			function_definition->function->is_extern = true;
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
			{
				lexer_.next();
				// inserting function
				function_table_.insert_function(*function_definition);
				return std::make_unique<stmt::extern_function_declaration>(std::move(function_definition));
			}
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::POINTER))
			{
				lexer_.next();
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::UNNAMED_CLASS_TYPE))
				{
					std::string type = std::move(utils::get_type_from_name(lexer_.front()->lexeme));
					function_definition->function_type = std::make_unique<expr::type_node>(std::move(type));
					lexer_.next();
					// inserting function
					function_table_.insert_function(*function_definition);
				}
				else
					PARSER_EXCEPTION("Expected unnamed class type. Samples: _int64, _str, _bool");
			}
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
				PARSER_EXCEPTION("Definition of extern function doesn\'t exsist");
			else
				PARSER_EXCEPTION("Unexpected token after declaration");
		}

		// { ... }
		else if (lexer_.expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
		{
			lexer_.next();
			auto block = std::make_unique<stmt::block_statement>();
			while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET) &&
				lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
			{
				block->statements.emplace_back(parse_statement());
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON)) { lexer_.next(); }
			}
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET))
			{
				PARSER_EXCEPTION("Expected \'}\' to close block");
			}
			lexer_.next();
			return block;
		}

		// Везде, где есть функции переписать и отрефакторить код.
		// Сделать для парсера таблицу функций.
		// Если функция уже была, то путь так.
		// Если функции нет, то путь то этак.


		// function_name (params) || function_name(args) || function_name (params) { ... } 
		else if (lexer_.expect(token::TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER))
		{
			expr::function_definition_node_ptr function_definition{ nullptr };
			// function_name(args);
			if (function_table_.contains_name(lexer_.front()->lexeme))
			{
				expr::expr_node_ptr function_call = expression(0);
				return std::make_unique<stmt::expression_statement>(std::move(function_call));
			}
			// function_name(params)
			else
				function_definition = parse_function();
			
			// function_name(params);
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
				PARSER_EXCEPTION("UL doesn\'t support forward-declaration");
			
			// function_name(params){ ... }
			else if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
			{
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::END))
					PARSER_EXCEPTION("Expected \'{\' after function definition");
				lexer_.next();
			}
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::FLBRACKET))
			{
				// inserting function
				function_definition->function->name = function_table_.insert_function(*function_definition);
				//
				lexer_.next();
				auto block = std::make_unique<stmt::block_statement>();
				while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET) &&
					lexer_.not_expect(token::TYPE_TOKEN_TYPE::END))
				{
					stmt::statement_ptr stmt = parse_statement();
					if (auto* n = dynamic_cast<stmt::expression_statement*>(stmt.get()))
					{
						if (auto* b = dynamic_cast<expr::return_value_node*>(n->expression.get()))
						{
							if (function_definition->function_type->type_str != b->type_str && !function_definition->function_type->type_str.empty())
								PARSER_EXCEPTION(std::format("Type {} isn\'t type of {}", b->type_str, function_definition->function_type->type_str));
							function_table_.set_functions_return_type(&function_definition->function->name->full_name, b->type_str);
							function_definition->function_type =
								std::make_unique<expr::type_node>(std::move(b->type_str));
						}
					}
					block->statements.emplace_back(std::move(stmt));
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON)) { lexer_.next(); }
				}
				if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::FRBRACKET))
				{
					PARSER_EXCEPTION("Expected \'}\' to close function definition");
				}
				lexer_.next();
				return std::make_unique<stmt::function_definition>(std::move(function_definition), std::move(block));
			}
			else
				PARSER_EXCEPTION("Expecting definition after function name with arguments");
		}
		// Bad
		// name_type = expr; || name_type.something;
		else if (lexer_.expect(token::TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER))
		{
			std::string var_name = lexer_.front()->lexeme;
			auto&& parent = nud(*lexer_.front());
			lexer_.next();
			// name_type = expr;
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::ASSIGNMENT_OPERATOR))
			{
				lexer_.next();
				auto&& value_expr = expression(0);
				if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
				{
					lexer_.next();
					return std::make_unique<stmt::assignment_statement>(std::move(var_name), std::move(value_expr));
				}
				else
					PARSER_EXCEPTION("Expected \';\' after assignment statement");
			}
			// name_type;
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
			{
				lexer_.next();
				return std::make_unique<stmt::expression_statement>(nud(*lexer_.front()));
			}
			// name_type...;
			else if (lexer_.expect(token::TYPE_TOKEN_TYPE::POINT))
			{
				return std::make_unique<stmt::expression_statement>(std::move(parse_field_call(std::move(parent))));
			}
			else
				PARSER_EXCEPTION("Variable has no purpose");
		}
		// other
		else if (is_expression_start(lexer_.front()->type))
		{
			auto expr = expression(0);
			return std::make_unique<stmt::expression_statement>(std::move(expr));
		}
		else PARSER_EXCEPTION(std::format("Unexpected token {} starting statement", lexer_.front()->lexeme));
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

	stmt::block_statement_ptr language_parser::parse_program()
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
		case TYPE_TOKEN_TYPE::TRIPLE_POINT:
			left = std::make_unique<expr::function_with_va_args_node>();
			break;
		case TYPE_TOKEN_TYPE::NUMBER_LITERAL:
		{
			int64_t integer = std::stoll(ti.lexeme);
			if (integer > std::numeric_limits<uint32_t>().max())
				left = std::make_unique<expr::number_literal_node>(std::move(ti.lexeme), 64);
			else
				left = std::make_unique<expr::number_literal_node>(std::move(ti.lexeme), 32);
			break;
		}
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
		case TYPE_TOKEN_TYPE::PLUS_OPERATOR:
			lexer_.next();
			left = expression(0);
			break;
		case TYPE_TOKEN_TYPE::KEYWORD_RETURN:
		{
			auto inner = expression(0);
			std::string return_type = expr::get_type_of_expression(*inner);
			if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::SEMICOLON))
			{
				PARSER_EXCEPTION("Expected \';\' after return");
			}
			lexer_.next();
			left = return_type.empty() ?
				std::make_unique<expr::return_value_node>(std::move(inner)) :
				std::make_unique<expr::return_value_node>(std::move(inner), std::move(return_type));
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
			expr::function_node_ptr function_ = std::make_unique<expr::function_node>(ti.lexeme);
			if (lexer_.expect(token::TYPE_TOKEN_TYPE::LBRACKET))
			{
				lexer_.next();
				auto&& args = std::make_unique<expr::function_arguments_node>();
				while (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::END))
						PARSER_EXCEPTION("Unexpected EOF in argument list");
					args->args.emplace_back(std::move(expr::make_argument(expression(0))));
					if (lexer_.expect(token::TYPE_TOKEN_TYPE::COMMA))
						lexer_.next();
				}
				if (lexer_.not_expect(token::TYPE_TOKEN_TYPE::RBRACKET))
				{
					PARSER_EXCEPTION("Expected \')\' to close argument list");
				}
				lexer_.next();
				
				if (function_table_.contains_name(function_->name->short_name))
				{
					function_->is_extern = function_table_.name_is_extern(function_->name->short_name);
					auto fn_type = 
						std::make_unique<expr::type_node>(std::move(function_table_.get_return_type_of_function(function_->name->short_name)));
					left = std::make_unique<expr::function_call_node>(std::move(function_), std::move(args), std::move(fn_type));
					auto& n = dynamic_cast<expr::function_call_node&>(*left.get());
					function_name_info::make_full_name_from_call(n);
				}
				else
					PARSER_EXCEPTION(std::format("There is no function named {}", function_->name->short_name));
			}
			else
				left = std::move(function_);
			break;
		}
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
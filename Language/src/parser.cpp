#include <parser.h>
#include <expression_info.h>
#include <stmt_expr_cast.h>

#define PARSER	ul::parser::
#define EXPR	ul::expr::
#define STMT	ul::stmt::

PARSER	language_parser::language_parser(lexer::language_lexer& l) :
	lexer_{ l }, source_manager_{ std::move(l.get_input_information()) }, parser_ctx_{ }
{}

uint32_t PARSER	language_parser::lbp(token::token_type& tt)
{
	using namespace token;
	switch (tt.enum_type)
	{
	case TID::LOGICAL_AND_OPERATOR:
	case TID::LOGICAL_OR_OPERATOR:
		return 1;
	case TID::ASSIGNMENT_OPERATOR:
		return 2;
	case TID::PLUS_OPERATOR:
	case TID::MINUS_OPERATOR:
		return 3;
	case TID::SLASH_OPERATOR:
	case TID::STAR_OPERATOR:
	case TID::LOGICAL_GREATER_OPERATOR:
	case TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR:
	case TID::LOGICAL_LESS_OPERATOR:
	case TID::LOGICAL_LESS_OR_EQUAL_OPERATOR:
	case TID::LOGICAL_EQUAL_OPERATOR:
	case TID::LOGICAL_NOT_EQUAL_OPERATOR:
		return 4;
	default:
		return 0;
	}
}

bool PARSER	language_parser::is_expression_start(const token::token_type& tt)
{
	using namespace token;
	switch (tt.enum_type)
	{
	case TID::NUMBER_LITERAL:
	case TID::STRING_LITERAL:
	case TID::MINUS_OPERATOR:
	case TID::PLUS_OPERATOR:
	case TID::LBRACKET:
	case TID::VARIABLE_IDENTIFIER:
	case TID::FUNCTION_IDENTIFIER:
	case TID::MARKER_EXPRESSION:
	case TID::TRIPLE_POINT:
	case TID::LOGICAL_NOT_OPERATOR:
		return true;
	default:
		return false;
	}
}

STMT else_statement_ptr PARSER language_parser::parse_else_statement()
{
	stmt::else_statement_ptr else_stmt{ nullptr };
	if (lexer_.not_expect(token::TID::FLBRACKET))
	{
		auto return_block = std::make_unique<stmt::expression_statement>(expression(0));
		else_stmt = std::make_unique<stmt::else_statement>(std::move(return_block));
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \';\' after one line then statement");
		lexer_.next();
	}
	else
	{
		auto return_block = parse_block_statement();
		else_stmt = std::make_unique<stmt::else_statement>(std::move(return_block));
	}
	return else_stmt;
}

EXPR function_parameters_node_ptr PARSER language_parser::parse_parameters()
{
	if (lexer_.not_expect(token::TID::LBRACKET))
		OUT_PARSER_EXCEPTION("Expected \'(\'");
	lexer_.next();
	auto params = std::make_unique<expr::function_parameters_node>();
	while (lexer_.not_expect(token::TID::RBRACKET))
	{
		if (lexer_.expect(token::TID::END))
			OUT_PARSER_EXCEPTION("Unexpected EOF in parameter list");
		auto&& future_param = expression(/*lexer_.front()*/0);
		if (ul::dyn_cast<expr::variable_node>(future_param.get()))
		{
			expr::type_variable_node_ptr type =
				std::make_unique<expr::type_variable_node>(ul::dyn_cast<expr::variable_node>(future_param.release()));
			std::string type_name = type->name;
			params->types.emplace_back(std::move(type));
			params->names.emplace_back(std::move(type_name));
		}
		if (auto* b = ul::dyn_cast<expr::function_with_va_args_node>(future_param.get()))
		{
			if (params->va_args)
				OUT_PARSER_EXCEPTION("Va args already defined");
			else params->va_args = true;
		}
		if (lexer_.expect(token::TID::COMMA))
			lexer_.next();
	}
	if (lexer_.not_expect(token::TID::RBRACKET))
		OUT_PARSER_EXCEPTION("Expected \')\' to close parameters list");

	lexer_.next();
	return params;
}



EXPR function_definition_node_ptr PARSER language_parser::parse_function_definition_node()
{
	std::string function_name = lexer_.front()->lexeme;
	auto&& fn = std::make_unique<expr::function_node>(std::move(function_name));
	lexer_.next();
	auto&& params = parse_parameters();
	auto&& function_with_params =
		std::make_unique<expr::function_definition_node>(std::move(fn), std::move(params));
	function_with_params->function_type = std::make_unique<expr::type_node>("");
	return function_with_params;
}

STMT statement_ptr PARSER language_parser::parse_function_definition()
{
	if (lexer_.not_expect(token::TID::FUNCTION_IDENTIFIER))
		OUT_PARSER_EXCEPTION("Expected function identifier", source_manager_.get_line_and_column());
	expr::function_definition_node_ptr function_definition{ nullptr };
	// function_name(args);
	if (function_table_.contains_name(lexer_.front()->lexeme))
	{
		expr::expr_node_ptr function_call = expression(0);
		return dyn_cast(std::move(function_call));
	}
	// function_name(params)
	else
		function_definition = parse_function_definition_node();

	// function_name(params);
	if (lexer_.expect(token::TID::SEMICOLON))
		OUT_PARSER_EXCEPTION("UL doesn\'t support forward-declaration");

	// function_name(params){ ... }
	else if (lexer_.not_expect(token::TID::FLBRACKET))
	{
		if (lexer_.expect(token::TID::END))
			OUT_PARSER_EXCEPTION("Expected \'{\' after function definition");
	}
	else if (lexer_.expect(token::TID::FLBRACKET))
	{
		// inserting function
		function_definition->function->name = function_table_.insert_function(*function_definition);
		parser_ctx_.current_function = function_definition.release();
		//
		lexer_.next();
		auto block = std::make_unique<stmt::block_statement>();
		while (lexer_.not_expect(token::TID::FRBRACKET) &&
			lexer_.not_expect(token::TID::END))
		{
			stmt::statement_ptr stmt = parse_statement();
			if (auto* b = ul::dyn_cast<stmt::return_statement>(stmt.get()))
			{
				if (parser_ctx_.current_function->function_type->type_str != b->type_str && not parser_ctx_.current_function->function_type->type_str.empty())
					OUT_PARSER_EXCEPTION(std::format("Type {} isn\'t type of {}", b->type_str, parser_ctx_.current_function->function_type->type_str));
				function_table_.set_functions_return_type(&parser_ctx_.current_function->function->name->full_name, b->type_str);
				parser_ctx_.current_function->function_type =
					std::make_unique<expr::type_node>(std::move(b->type_str));
			}
			block->statements.emplace_back(std::move(stmt));
			if (lexer_.expect(token::TID::SEMICOLON)) { lexer_.next(); }
		}
		if (lexer_.not_expect(token::TID::FRBRACKET))
			OUT_PARSER_EXCEPTION("Expected \'}\' to close function definition");
		lexer_.next();
		function_definition.reset(parser_ctx_.current_function);
		return std::make_unique<stmt::function_definition>(std::move(function_definition), std::move(block));
	}
	else
		OUT_PARSER_EXCEPTION("Expecting definition after function name with arguments");
}

STMT extern_function_declaration_ptr PARSER language_parser::parse_extern_function()
{
	if (lexer_.not_expect(token::TID::KEYWORD_EXTERN))
		OUT_PARSER_EXCEPTION("Expected keyword extern");
	lexer_.next();
	if (lexer_.not_expect(token::TID::FUNCTION_IDENTIFIER))
		OUT_PARSER_EXCEPTION("Expected function identifier after keyword extern");
	auto&& function_definition = parse_function_definition_node();
	function_definition->function->is_extern = true;
	if (lexer_.expect(token::TID::SEMICOLON))
	{
		lexer_.next();
		// inserting function
		function_table_.insert_function(*function_definition);
		return std::make_unique<stmt::extern_function_declaration>(std::move(function_definition));
	}
	else if (lexer_.expect(token::TID::POINTER))
	{
		lexer_.next();
		if (lexer_.not_expect(token::TID::UNNAMED_CLASS_TYPE))
			OUT_PARSER_EXCEPTION("Expected unnamed class type. Samples: _int64, _str, _bool");
		std::string type = std::move(utils::get_type_from_name(lexer_.front()->lexeme));
		function_definition->function_type = std::make_unique<expr::type_node>(std::move(type));
		lexer_.next();
		// inserting function
		function_table_.insert_function(*function_definition);
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected semicolon after unnamed type in extern function");
		lexer_.next();
		return std::make_unique<stmt::extern_function_declaration>(std::move(function_definition));
	}
	else if (lexer_.expect(token::TID::FLBRACKET))
		OUT_PARSER_EXCEPTION("Definition of extern function doesn\'t exsist");
	else
		OUT_PARSER_EXCEPTION("Unexpected token after declaration");
}

STMT assignment_statement_ptr PARSER language_parser::parse_assignment(std::string variable_name)
{
	if (lexer_.not_expect(token::TID::ASSIGNMENT_OPERATOR))
		OUT_PARSER_EXCEPTION("Expected assignment operator");
	lexer_.next();
	auto&& value_expr = expression(0);
	if (lexer_.expect(token::TID::SEMICOLON))
	{
		lexer_.next();
		return std::make_unique<stmt::assignment_statement>(std::move(variable_name), std::move(value_expr));
	}
	else
		OUT_PARSER_EXCEPTION("Expected \';\' after assignment statement");
}

STMT block_statement_ptr PARSER language_parser::parse_block_statement()
{
	if (lexer_.not_expect(token::TID::FLBRACKET))
		OUT_PARSER_EXCEPTION("Expected \'{\' for block statement");
	lexer_.next();
	auto block = std::make_unique<stmt::block_statement>();
	while (lexer_.not_expect(token::TID::FRBRACKET) &&
		lexer_.not_expect(token::TID::END))
	{
		auto stmt = parse_statement();
		if (auto* b = ul::dyn_cast<stmt::return_statement>(stmt.get()))
		{
			if (!parser_ctx_.current_function)
				OUT_PARSER_EXCEPTION("keyword \'return\' out of function");
			if (parser_ctx_.current_function->function_type->type_str != b->type_str && not parser_ctx_.current_function->function_type->type_str.empty())
				OUT_PARSER_EXCEPTION(std::format("Type {} isn\'t type of {}", b->type_str, parser_ctx_.current_function->function_type->type_str));
			function_table_.set_functions_return_type(&parser_ctx_.current_function->function->name->full_name, b->type_str);
			parser_ctx_.current_function->function_type =
				std::make_unique<expr::type_node>(std::move(b->type_str));
		}
		block->statements.emplace_back(std::move(stmt));
		if (lexer_.expect(token::TID::SEMICOLON)) { lexer_.next(); }
	}
	if (lexer_.not_expect(token::TID::FRBRACKET))
		OUT_PARSER_EXCEPTION("Expected \'}\' to close block");
	lexer_.next();
	return block;
}

STMT statement_ptr PARSER language_parser::parse_statement()
{
	if (lexer_.expect(token::TID::KEYWORD_ELIF))
		OUT_PARSER_EXCEPTION("Keyword \"elif\" doesn\'t refer to \'if\'");
	if(lexer_.expect(token::TID::KEYWORD_ELSE))
		OUT_PARSER_EXCEPTION("Keyword \"else\" doesn\'t refer to \'if\'");
	// extern function_name(params) -> _type; || extern function_name(params);
	if (lexer_.expect(token::TID::KEYWORD_EXTERN))
		return parse_extern_function();

	// { ... }
	else if (lexer_.expect(token::TID::FLBRACKET))
		return parse_block_statement();

	else if (lexer_.expect(token::TID::KEYWORD_RETURN))
	{
		lexer_.next();
		auto return_block = expression(0);
		std::string return_type = expr::get_type_of_expression(*return_block);
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \';\' after return");
		lexer_.next();
		return return_type.empty() ?
			std::make_unique<stmt::return_statement>(std::move(return_block)) :
			std::make_unique<stmt::return_statement>(std::move(return_block), std::move(return_type));
	}

	else if(lexer_.expect(token::TID::KEYWORD_IF))
	{
		lexer_.next();
		auto condition = expression(0);
		stmt::if_statement_ptr if_stmt = parse_if_statement<stmt::if_statement>(std::move(condition));
		stmt::else_statement_ptr else_stmt{ nullptr };
		bool was_else = false;
		while (lexer_.expect(token::TID::KEYWORD_ELIF) ||
			lexer_.expect(token::TID::KEYWORD_ELSE)) 
		{
			if (lexer_.expect(token::TID::KEYWORD_ELSE) && was_else)
				OUT_PARSER_EXCEPTION("else can be used once");
			if (was_else)
				OUT_PARSER_EXCEPTION("Unhandled operation: elif after else");

			if (lexer_.expect(token::TID::KEYWORD_ELSE))
			{
				lexer_.next();
				was_else = true;
				auto another_stmt = parse_else_statement();
				else_stmt = else_stmt ? 
					std::make_unique<stmt::else_statement>(std::move(another_stmt), std::move(else_stmt)) :
					std::move(another_stmt);
			}
			else
			{
				lexer_.next();
				auto elif_condition = expression(0);
				auto another_stmt = parse_if_statement<stmt::elif_statement>(std::move(elif_condition));
				else_stmt = else_stmt ?
					std::make_unique<stmt::elif_statement>(std::move(another_stmt), std::move(else_stmt)) :
					std::move(another_stmt);
			}
		}
		
		if_stmt->next_cond_stmt = stmt::reverse_if_statements_list(std::move(else_stmt));
		return if_stmt;
	}

	// function_name (params) || function_name(args) || function_name (params) { ... } 
	else if (lexer_.expect(token::TID::FUNCTION_IDENTIFIER))
	{
		return parse_function_definition();
	}
	// Bad
	// name_type = expr; || name_type.something;
	else if (lexer_.expect(token::TID::VARIABLE_IDENTIFIER))
	{
		std::string var_name = lexer_.front()->lexeme;
		auto&& parent = nud(*lexer_.front());
		lexer_.next();
		// name_type = expr;
		if (lexer_.expect(token::TID::ASSIGNMENT_OPERATOR))
			return parse_assignment(std::move(var_name));
		// name_type;
		else if (lexer_.expect(token::TID::SEMICOLON))
		{
			lexer_.next();
			return std::make_unique<stmt::expression_statement>(nud(*lexer_.front()));
		}
		// name_type...;
		else if (lexer_.expect(token::TID::POINT))
			return std::make_unique<stmt::expression_statement>(std::move(parse_field_call(std::move(parent))));
		else
			OUT_PARSER_EXCEPTION("Variable has no purpose");
	}
	// expressions
	else if (is_expression_start(lexer_.front()->type))
		return std::make_unique<stmt::expression_statement>(expression(0));
	else
		OUT_PARSER_EXCEPTION(std::format("Unexpected token {} starting statement", lexer_.front()->lexeme));
}

EXPR field_call_node_ptr PARSER language_parser::parse_field_call(expr::expr_node_ptr parent)
{
	expr::field_call_node_ptr field{ nullptr };
	// Переделать
#if 0
	while (lexer_.expect(token::TID::POINT))
	{
		lexer_.next();
		if (lexer_.not_expect(token::TID::VARIABLE_IDENTIFIER) &&
			lexer_.not_expect(token::TID::FUNCTION_IDENTIFIER)
			)
		{
			OUT_PARSER_EXCEPTION("Expected some field after \'.\'");
		}
		auto&& field_n = expression(0);
		lexer_.next();

		field = field ? std::make_unique<expr::field_call_node>(std::move(parent), std::move(field_n), std::move(field)) :
			std::make_unique<expr::field_call_node>(std::move(parent), std::move(field_n));
		parent = std::move(field);
	}
#endif
	return field;
}

EXPR function_arguments_node_ptr PARSER language_parser::parse_arguments()
{
	if (lexer_.not_expect(token::TID::LBRACKET))
		OUT_PARSER_EXCEPTION("Expected left bracket for arguments");
	lexer_.next();
	auto&& args = std::make_unique<expr::function_arguments_node>();
	while (lexer_.not_expect(token::TID::RBRACKET))
	{
		if (lexer_.expect(token::TID::END))
			OUT_PARSER_EXCEPTION("Unexpected EOF in argument list");
		args->args.emplace_back(std::move(expr::make_argument(expression(0))));
		if (lexer_.expect(token::TID::COMMA))
			lexer_.next();
	}
	if (lexer_.not_expect(token::TID::RBRACKET))
		OUT_PARSER_EXCEPTION("Expected \')\' to close argument list");
	lexer_.next();
	return args;
}


EXPR function_call_node_ptr PARSER language_parser::get_function_call_node(expr::function_node_ptr fn_node, expr::function_arguments_node_ptr args)
{
	if (not function_table_.contains_name(fn_node->name->short_name))
		OUT_PARSER_EXCEPTION(std::format("There is no function named {}", fn_node->name->short_name));
	fn_node->is_extern = function_table_.name_is_extern(fn_node->name->short_name);
	auto fn_type =
		std::make_unique<expr::type_node>(std::move(function_table_.get_return_type_of_function(fn_node->name->short_name)));
	auto&& fn_call_ptr = std::make_unique<expr::function_call_node>(std::move(fn_node), std::move(args), std::move(fn_type));
	function_name_info::make_full_name_from_call(*fn_call_ptr);
	return fn_call_ptr;
}

STMT block_statement_ptr PARSER language_parser::parse_program()
{
	auto&& program = std::make_unique<stmt::block_statement>();
	while (lexer_.not_expect(token::TID::END))
	{
		program->statements.emplace_back(parse_statement());
	}
	return program;
}

EXPR expr_node_ptr PARSER language_parser::nud(token::token_info& ti)
{
	using namespace token;
	expr::expr_node_ptr left;
	switch (ti.type)
	{
	case TID::TRIPLE_POINT:
		left = std::make_unique<expr::function_with_va_args_node>();
		break;
	case TID::TRUE:
		left = std::make_unique<expr::number_literal_node>("1", 1);
		break;
	case TID::FALSE:
		left = std::make_unique<expr::number_literal_node>("0", 1);
		break;
	case TID::NUMBER_LITERAL:
	{
		int64_t integer = std::stoll(ti.lexeme);
		if (integer > std::numeric_limits<uint32_t>().max())
			left = std::make_unique<expr::number_literal_node>(std::move(ti.lexeme), 64);
		else
			left = std::make_unique<expr::number_literal_node>(std::move(ti.lexeme), 32);
		break;
	}
	case TID::STRING_LITERAL:
	{
		std::string string_literal;
		for (size_t i{ 0 }, lsize = ti.lexeme.size(); i < lsize; ++i)
		{
			if (ti.lexeme[i] == '\\' && (i + 1) < lsize)
			{
				switch (ti.lexeme[i + 1])
				{
				case 'n':
					string_literal += '\n';
					break;
				case 't':
					string_literal += '\t';
					break;
				case '\\':
					string_literal += '\\';
					break;
				case '\"':
					string_literal += '"';
					break;
				case '\'':
					string_literal += '\'';
					break;
				default:
					OUT_PARSER_EXCEPTION(std::format("Unknown escape sequence: \\{}", ti.lexeme[i + 1]));
				}
				++i;
			}
			else
				string_literal += ti.lexeme[i];
		}
		std::string_view sv{ string_literal };
		sv.remove_prefix(1);
		sv.remove_suffix(1);
		string_literal = std::move(std::string{ sv });
		left = std::make_unique<expr::string_literal_node>(std::move(string_literal));
		break;
	}
	case TID::MINUS_OPERATOR:
	case TID::PLUS_OPERATOR:
	case TID::LOGICAL_NOT_OPERATOR:
	{
		expr::expr_node_ptr operand = expression(0);
		left = std::make_unique<expr::unary_operator_node>(std::move(ti), std::move(operand));
		break;
	}
	case TID::LBRACKET:
	{
		auto inner = expression(0);
		if (lexer_.not_expect(token::TID::RBRACKET))
			OUT_PARSER_EXCEPTION("Expected \')\'");
		lexer_.next();
		left = std::move(inner);
		break;
	}
	case TID::KEYWORD_REF:
	{
		if (lexer_.expect(TID::VARIABLE_IDENTIFIER))
		{
			auto&& var = std::make_unique<expr::variable_node>(lexer_.front()->lexeme);
			left = std::make_unique<expr::variable_reference_node>(std::move(var));
			lexer_.next();
		}
		else
			OUT_PARSER_EXCEPTION("Expected variable after keyword \"ref\"");
		break;

	}
	case TID::VARIABLE_IDENTIFIER:
		left = std::make_unique<expr::variable_node>(ti.lexeme);
		break;
	case TID::FUNCTION_IDENTIFIER:
	{
		expr::function_node_ptr fn_node = std::make_unique<expr::function_node>(ti.lexeme);
		if (lexer_.expect(token::TID::LBRACKET))
			left = get_function_call_node(std::move(fn_node), parse_arguments());
		else
			left = std::move(fn_node);
		break;
	}
	case TID::MARKER_EXPRESSION:
		left = std::make_unique<expr::marker_node>(std::move(ti));
		break;
	default:
		OUT_PARSER_EXCEPTION("Unexpected token");
	}
#if 0
	for (;;)
	{
		if (lexer_.not_expect(token::TID::POINT))
			break;
		lexer_.next();
		if (lexer_.not_expect(TID::VARIABLE_IDENTIFIER) &&
			lexer_.not_expect(TID::FUNCTION_IDENTIFIER)
			)
		{
			OUT_PARSER_EXCEPTION("Expected some field after \'.\'");
		}
		auto&& field_n = expression(0);
		lexer_.next();
		left = std::make_unique<expr::field_call_node>(std::move(left), std::move(field_n));
	}
#endif
	return left;
}
EXPR expr_node_ptr PARSER language_parser::led(token::token_info& ti, expr::expr_node_ptr left)
{
	using namespace token;
	uint32_t right_binding_priority = lbp(ti.type);
	auto&& right = expression(right_binding_priority);
	switch (ti.type)
	{
	case TID::LOGICAL_AND_OPERATOR:
	case TID::LOGICAL_OR_OPERATOR:
	case TID::LOGICAL_EQUAL_OPERATOR:
	case TID::LOGICAL_NOT_EQUAL_OPERATOR:
	case TID::LOGICAL_GREATER_OPERATOR:
	case TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR:
	case TID::LOGICAL_LESS_OPERATOR:
	case TID::LOGICAL_LESS_OR_EQUAL_OPERATOR:
		return std::make_unique<expr::logical_binary_operator_node>(ti, std::move(left), std::move(right));
	default:
		return std::make_unique<expr::binary_operator_node>(ti, std::move(left), std::move(right));
	}
}
EXPR expr_node_ptr PARSER language_parser::expression(uint32_t right_binding_priority)
{
	auto&& cur = lexer_.front();
	if (lexer_.is_now_break_symbol())
	{
		OUT_PARSER_EXCEPTION("Empty expression");
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


#undef OUT_PARSER_EXCEPTION
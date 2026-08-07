#include <parser.h>
#include <expression_info.h>
#include <stmt_expr_cast.h>
#include <iostream>

#define PARSER	ul::parser::
#define EXPR	ul::expr::
#define STMT	ul::stmt::

PARSER	language_parser::language_parser(lexer::language_lexer& l) :
	lexer_{ l }, source_manager_{ std::move(l.get_input_information()) }, pctx_{ std::make_unique<parser_context>() }
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
	case TID::PLUS_ASSIGNMENT_OPERATOR:
	case TID::MINUS_ASSIGNMENT_OPERATOR:
	case TID::SLASH_ASSIGNMENT_OPERATOR:
	case TID::STAR_ASSIGNMENT_OPERATOR:
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
	case TID::KEYWORD_BREAK:
	case TID::KEYWORD_CONTINUE:
	case TID::KEYWORD_NEW:
	case TID::KEYWORD_NULL:
		return true;
	default:
		return false;
	}
}

STMT statement_ptr PARSER language_parser::parse_inner_statement()
{
	stmt::statement_ptr inner_stmt{ nullptr };
	if (lexer_.not_expect(token::TID::FLBRACKET))
	{
		inner_stmt = std::make_unique<stmt::expression_statement>(expression(0));
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \';\' after expression");
		lexer_.next();
	}
	else
		inner_stmt = parse_block_statement();
	return inner_stmt;
}

STMT else_statement_ptr PARSER language_parser::parse_else_statement()
{
	stmt::else_statement_ptr else_stmt = 
		std::make_unique<stmt::else_statement>(parse_inner_statement());
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
			utils::classes::stringi8 type_name = type->name;
			params->types.emplace_back(std::move(type));
			params->names.emplace_back(std::move(type_name));
		}
		if (auto* b = ul::dyn_cast<expr::function_with_va_args_node>(future_param.get()))
		{
			if (params->va_args == true)
			{
				OUT_PARSER_EXCEPTION("Va args already defined");
			}
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
	utils::classes::stringi8 function_name = lexer_.front()->lexeme;
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
		pctx_->in_function = true;
		// inserting function
		function_definition->function->name = function_table_.insert_function(*function_definition);
		pctx_->current_function = function_definition.release();
		//
		lexer_.next();
		auto block = std::make_unique<stmt::block_statement>();
		while (lexer_.not_expect(token::TID::FRBRACKET) &&
			lexer_.not_expect(token::TID::END))
		{
			stmt::statement_ptr stmt = parse_statement();
			if (auto* b = ul::dyn_cast<stmt::return_statement>(stmt.get()))
			{
				if (pctx_->current_function->function_type->type_str != b->type_str && not pctx_->current_function->function_type->type_str.empty())
					OUT_PARSER_EXCEPTION(std::format("Type {} isn\'t type of {}", b->type_str, pctx_->current_function->function_type->type_str));
				function_table_.
					set_functions_return_type(&pctx_->current_function->function->name->full_name, b->type_str);
				pctx_->current_function->function_type =
					std::make_unique<expr::type_node>(std::move(b->type_str));
			}
			block->statements.emplace_back(std::move(stmt));
			if (lexer_.expect(token::TID::SEMICOLON)) { lexer_.next(); }
		}
		if (lexer_.not_expect(token::TID::FRBRACKET))
			OUT_PARSER_EXCEPTION("Expected \'}\' to close function definition");
		lexer_.next();
		function_definition.reset(pctx_->current_function);
		pctx_->in_function = false;
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
		utils::classes::stringi8 type = std::move(lexer_.front()->lexeme.get_type_from_name());
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

STMT block_statement_ptr PARSER language_parser::parse_block_statement()
{
	if (lexer_.not_expect(token::TID::FLBRACKET))
		OUT_PARSER_EXCEPTION("Expected \'{\' for block statement");
	lexer_.next();
	++pctx_->block_depth;
	auto block = std::make_unique<stmt::block_statement>();
	while (lexer_.not_expect(token::TID::FRBRACKET) &&
		lexer_.not_expect(token::TID::END))
	{
		auto stmt = parse_statement();
		if (auto* b = ul::dyn_cast<stmt::return_statement>(stmt.get()))
		{
			if (!pctx_->current_function)
				OUT_PARSER_EXCEPTION("keyword \'return\' out of function");
			if (pctx_->current_function->function_type->type_str != b->type_str && not pctx_->current_function->function_type->type_str.empty())
				OUT_PARSER_EXCEPTION(std::format("Type {} isn\'t type of {}", b->type_str, pctx_->current_function->function_type->type_str));
			function_table_.set_functions_return_type(&pctx_->current_function->function->name->full_name, b->type_str);
			pctx_->current_function->function_type =
				std::make_unique<expr::type_node>(std::move(b->type_str));
		}
		block->statements.emplace_back(std::move(stmt));
		block->statements.emplace_back(std::make_unique<stmt::end_of_block_statement>(std::move(pctx_->end_expressions)));
		if (lexer_.expect(token::TID::SEMICOLON)) { lexer_.next(); }
	}
	if (lexer_.not_expect(token::TID::FRBRACKET))
		OUT_PARSER_EXCEPTION("Expected \'}\' to close block");
	lexer_.next();

	--pctx_->block_depth;
	
	for(auto it_beg = pctx_->current_names.begin(), it_end = pctx_->current_names.end(); it_beg != it_end; ++it_beg)
	{
		if (it_beg->second > pctx_->block_depth)
			it_beg = pctx_->current_names.erase(it_beg);
	}

	return block;
}

std::vector<EXPR variable_assignment_expr_ptr> PARSER language_parser::parse_value_definitions()
{
	std::vector<expr::variable_assignment_expr_ptr> result;
	while(lexer_.not_expect(token::TID::SEMICOLON))
	{
		auto&& expr = expression(0);
		if (lexer_.not_expect(token::TID::COMMA) && lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \",\" or \";\" to end expression or definitions");
		if (lexer_.expect(token::TID::COMMA))
			lexer_.next();
		if (auto* assign = ul::dyn_cast<expr::variable_assignment_expr>(expr.release()))
			result.emplace_back(assign);
		else
			OUT_PARSER_EXCEPTION("Need assignment expression");
	}
	return result;
}
std::vector<EXPR expr_node_ptr> PARSER language_parser::parse_value_differences()
{
	std::vector<expr::expr_node_ptr> result;
	while (lexer_.not_expect(token::TID::RBRACKET))
	{
		auto&& expr = expression(0);
		if (lexer_.not_expect(token::TID::COMMA) && lexer_.not_expect(token::TID::RBRACKET))
			OUT_PARSER_EXCEPTION("Expected \",\" or \")\" to end expression or definitions");
		if (lexer_.expect(token::TID::COMMA))
			lexer_.next();
		result.emplace_back(std::move(expr));
	}
	return result;
}

STMT statement_ptr PARSER language_parser::parse_statement()
{
	if (lexer_.expect(token::TID::KEYWORD_ELIF))
		OUT_PARSER_EXCEPTION("Keyword \"elif\" doesn\'t refer to \"if\"");
	if(lexer_.expect(token::TID::KEYWORD_ELSE))
		OUT_PARSER_EXCEPTION("Keyword \"else\" doesn\'t refer to \"if\"");
	// extern function_name(params) -> _type; || extern function_name(params);
	if (lexer_.expect(token::TID::KEYWORD_EXTERN))
		return parse_extern_function();

	else if(lexer_.expect(token::TID::MARKER_EXPRESSION))
	{
		utils::classes::stringi8 lexeme = std::move(lexer_.front()->lexeme);
		lexer_.next();
		return marker_parser_->parse_marker(std::move(lexeme));
	}

	else if(lexer_.expect(token::TID::KEYWORD_INSERT))
	{
		lexer_.next();
		auto&& file_name = expression(0);
		if (auto* fn = ul::dyn_cast<expr::string_literal_node>(file_name.get()))
			return std::make_unique<stmt::insert_statement>(fn->literal.get_string_literal());
		else
			OUT_PARSER_EXCEPTION("Inserting file requires file name");
	}

	else if(lexer_.expect(token::TID::KEYWORD_FOR))
	{
		++pctx_->loop_depth;
		lexer_.next();
		if (pctx_->loop_depth == 0)
			OUT_PARSER_EXCEPTION("Depth of loop is lower then 0 (0_0)");
		if (lexer_.not_expect(token::TID::LBRACKET))
			OUT_PARSER_EXCEPTION("Expected \"(\" to start definition part");
		lexer_.next();
		auto&& definition = parse_value_definitions();
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \";\" to end start definition part");
		lexer_.next();
		auto&& condition = expression(0);
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \";\" to end condition part");
		if (not ul::dyn_cast<expr::logical_binary_operator_node>(condition.get()))
			OUT_PARSER_EXCEPTION("Expected logical expression");
		lexer_.next();
		auto&& after_iteration = parse_value_differences();
		
		if(lexer_.not_expect(token::TID::RBRACKET))
			OUT_PARSER_EXCEPTION("Expected \")\" to end after_definition part");
		lexer_.next();
		
		auto&& for_statement = std::make_unique<stmt::for_loop_statement>
			(
				parse_inner_statement(),
				std::move(condition),
				std::move(definition),
				std::move(after_iteration)
			);
		--pctx_->loop_depth;
		return for_statement;
	}
	else if (lexer_.expect(token::TID::KEYWORD_WHILE))
	{
		++pctx_->loop_depth;
		lexer_.next();
		auto&& condition = expression(0);
		auto&& wloop_statement = std::make_unique<stmt::while_loop_statement>(parse_inner_statement(), std::move(condition));
		if (pctx_->loop_depth == 0)
			OUT_PARSER_EXCEPTION("Depth of loop is lower then 0 (0_0)");
		--pctx_->loop_depth;
		return wloop_statement;
	}
	else if (lexer_.expect(token::TID::KEYWORD_LOOP))
	{
		++pctx_->loop_depth;
		lexer_.next();
		auto&& loop_statement = std::make_unique<stmt::loop_statement>(parse_inner_statement());
		if (pctx_->loop_depth == 0)
			OUT_PARSER_EXCEPTION("Depth of loop is lower then 0 (0_0)");
		--pctx_->loop_depth;
		return loop_statement;
	}
	
	// { ... }
	else if (lexer_.expect(token::TID::FLBRACKET))
		return parse_block_statement();

	else if (lexer_.expect(token::TID::KEYWORD_RETURN))
	{
		lexer_.next();
		auto return_block = expression(0);
		utils::classes::stringi8 return_type = expr::get_type_of_expression(*return_block);
		if (lexer_.not_expect(token::TID::SEMICOLON))
			OUT_PARSER_EXCEPTION("Expected \';\' after return");
		lexer_.next();
		return return_type.empty() ?
			std::make_unique<stmt::return_statement>(std::move(return_block)) :
			std::make_unique<stmt::return_statement>(std::move(return_block), std::move(return_type));
	}

	else if(lexer_.expect(token::TID::KEYWORD_IF))
	{
		pctx_->in_if_statement = true;
		lexer_.next();
		auto condition = expression(0);
		stmt::if_statement_ptr if_stmt = parse_if_statement<stmt::if_statement>(std::move(condition));
		stmt::else_statement_ptr else_stmt{ nullptr };
		bool was_else = false;
		while (lexer_.expect(token::TID::KEYWORD_ELIF) ||
			lexer_.expect(token::TID::KEYWORD_ELSE)) 
		{
			if (lexer_.expect(token::TID::KEYWORD_ELSE) && was_else)
				OUT_PARSER_EXCEPTION("Keyword \'else\' can\'t be used once");
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
		pctx_->in_if_statement = false;
		return if_stmt;
	}

	// function_name (params) || function_name(args) || function_name (params) { ... } 
	else if (lexer_.expect(token::TID::FUNCTION_IDENTIFIER))
	{
		return parse_function_definition();
	}
	// Bad
	// expressions
	else if (is_expression_start(lexer_.front()->type))
		return std::make_unique<stmt::expression_statement>(expression(0));
	else
		OUT_PARSER_EXCEPTION(std::format("Unexpected token {} starting statement", lexer_.front()->lexeme));
}

EXPR field_call_node_ptr PARSER language_parser::parse_field_call(expr::expr_node_ptr parent)
{
	expr::field_call_node_ptr field{ nullptr };

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
		auto&& arg = expr::make_argument(expression(0));
		if(auto* var = ul::dyn_cast<expr::variable_node>(arg->value.get()))
		{
			if (not pctx_->current_names.contains(var->name))
				OUT_PARSER_EXCEPTION("Variable doesn\'t exsist");
		}
		args->args.emplace_back(std::move(arg));
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
	case TID::KEYWORD_NAMEOF:
	{
		if (lexer_.not_expect(TID::LBRACKET))
			OUT_PARSER_EXCEPTION("Expected \"(\" after keyword \"nameof\"");
		lexer_.next();
		auto name_expr = expression(0);
		if (lexer_.not_expect(TID::RBRACKET))
			OUT_PARSER_EXCEPTION("Expected \")\" after keyword \"nameof\"");
		lexer_.next();
		if (auto* var = ul::dyn_cast<expr::variable_node>(name_expr.release()))
		{
			if (not pctx_->current_names.contains(var->name))
				OUT_PARSER_EXCEPTION("Variable doesn\'t exsist");
			expr::variable_node_ptr varp{ var };
			left = std::make_unique<expr::nameof_expr>(std::move(varp));
		}
		else
			OUT_PARSER_EXCEPTION("Expected variable after keyword \"nameof\"");
		break;
	}
	case TID::KEYWORD_BREAK:
		if (pctx_->loop_depth == 0)
			OUT_PARSER_EXCEPTION("Keyword \"break\" only avaliable in loop statement");
		left = std::make_unique<expr::break_node>();
		break;
	case TID::KEYWORD_CONTINUE:
		if (pctx_->loop_depth == 0)
			OUT_PARSER_EXCEPTION("Keyword \"continue\" only avaliable in loop statement");
		left = std::make_unique<expr::continue_node>();
		break;
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
		utils::classes::stringi8 string_literal;
		try
		{
			string_literal = ti.lexeme.get_string_literal();
		}
		catch(ul::ex::parser_exception& pex)
		{
			OUT_PARSER_EXCEPTION(pex.what());
		}
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
		++pctx_->bracket_depth;
		if (pctx_->bracket_depth >= 256)
			OUT_PARSER_EXCEPTION("Limit of \'(\' (255 lbrackets) was reached");
		auto inner = expression(0);
		if (lexer_.not_expect(token::TID::RBRACKET))
			OUT_PARSER_EXCEPTION("Expected \')\'");
		lexer_.next();
		left = std::move(inner);
		--pctx_->bracket_depth;
		break;
	}
	case TID::KEYWORD_REF:
	{
		if (lexer_.not_expect(TID::VARIABLE_IDENTIFIER))
			OUT_PARSER_EXCEPTION("Expected variable after keyword \"ref\"");
		
		auto&& var = std::make_unique<expr::variable_node>(lexer_.front()->lexeme);
		if (!pctx_->current_names.contains(var->name))
			OUT_PARSER_EXCEPTION("Variable doesn\'t exsist");

		left = std::make_unique<expr::variable_reference_node>(std::move(var));
		lexer_.next();
		break;
	}
	case TID::KEYWORD_NULL:
		left = std::make_unique<expr::null_expr>();
		break;
	case TID::VARIABLE_IDENTIFIER:
		left = std::make_unique<expr::variable_node>(std::move(ti.lexeme));
		break;
	case TID::FUNCTION_IDENTIFIER:
	{
		expr::function_node_ptr fn_node = std::make_unique<expr::function_node>(std::move(ti.lexeme));
		if (lexer_.expect(token::TID::LBRACKET))
			left = get_function_call_node(std::move(fn_node), parse_arguments());
		else
			left = std::move(fn_node);
		break;
	}
	default:
		OUT_PARSER_EXCEPTION(std::format("Word {} is unexpected", ti.lexeme));
	}
	return left;
}
EXPR expr_node_ptr PARSER language_parser::led(token::token_info& ti, expr::expr_node_ptr left)
{
	using namespace token;
	uint32_t right_binding_priority = lbp(ti.type);
	auto&& right = expression(right_binding_priority);

	switch (ti.type)
	{
	case TID::ASSIGNMENT_OPERATOR:
	{
		if (auto* lhsp = ul::dyn_cast<expr::variable_node>(left.release()))
		{
			auto&& lhs = expr::variable_node_ptr{ lhsp };
			pctx_->current_names.insert({ lhs->name, pctx_->block_depth });
			return std::make_unique<expr::variable_assignment_expr>(std::move(lhs), std::move(right));
		}
		else
			OUT_PARSER_EXCEPTION("Left operand isn\'t variable");
	}
	case TID::PLUS_ASSIGNMENT_OPERATOR:
	case TID::MINUS_ASSIGNMENT_OPERATOR:
	case TID::STAR_ASSIGNMENT_OPERATOR:
	case TID::SLASH_ASSIGNMENT_OPERATOR:
	{
		if (auto* lhsp = ul::dyn_cast<expr::variable_node>(left.release()))
		{
			auto&& lhs = expr::variable_node_ptr{ lhsp };
			if (!pctx_->current_names.contains(lhs->name))
				OUT_PARSER_EXCEPTION(std::format("Variable {} doesn\'t exsist", lhs->name));
			return std::make_unique <expr::variable_additional_assignment_expr>
				(std::move(ti.type), std::move(lhs), std::move(right));
		}
		else
			OUT_PARSER_EXCEPTION("Left operand isn\'t variable");
	}
	case TID::LOGICAL_AND_OPERATOR:
	case TID::LOGICAL_OR_OPERATOR:
	case TID::LOGICAL_EQUAL_OPERATOR:
	case TID::LOGICAL_NOT_EQUAL_OPERATOR:
	case TID::LOGICAL_GREATER_OPERATOR:
	case TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR:
	case TID::LOGICAL_LESS_OPERATOR:
	case TID::LOGICAL_LESS_OR_EQUAL_OPERATOR:
		if (auto* lhsp = ul::dyn_cast<expr::variable_node>(left.get()))
		{
			if (!pctx_->current_names.contains(lhsp->name))
				OUT_PARSER_EXCEPTION(std::format("Variable {} doesn\'t exsist", lhsp->name));
		}
		return std::make_unique<expr::logical_binary_operator_node>(std::move(ti.type), std::move(left), std::move(right));
	default:
		if (auto* lhsp = ul::dyn_cast<expr::variable_node>(left.get()))
		{
			if (!pctx_->current_names.contains(lhsp->name))
				OUT_PARSER_EXCEPTION(std::format("Variable {} doesn\'t exsist", lhsp->name));
		}
		return std::make_unique<expr::binary_operator_node>(std::move(ti.type), std::move(left), std::move(right));
	}
}
EXPR expr_node_ptr PARSER language_parser::expression(uint32_t right_binding_priority)
{
	auto&& cur = lexer_.front();
	if (lexer_.is_now_break_symbol())
	{
		OUT_PARSER_EXCEPTION("Expression is empty");
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
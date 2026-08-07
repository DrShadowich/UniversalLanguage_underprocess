#include <expression_info.h>
#include <core_exceptions.h>
namespace ul::expr
{
	namespace detail
	{
		name_info::name_info() noexcept :
			full_name{}, short_name{}, type_str{}
		{}
		name_info::name_info(utils::classes::stringi8 f, utils::classes::stringi8 s, utils::classes::stringi8 t) noexcept :
			full_name{ std::move(f) }, short_name{ std::move(s) }, type_str{ std::move(t) }
		{}
		name_info::name_info(name_info&& rhs) noexcept :
			full_name{ std::move(rhs.full_name) }, short_name{ std::move(rhs.short_name) }, type_str{ std::move(rhs.type_str) }
		{}

		name_info::name_info(utils::classes::stringi8 s) noexcept :
			full_name{}, short_name{ std::move(s) }, type_str{}
		{}

		void name_info::rename(const utils::classes::stringi8& new_name)
		{
			short_name = new_name;
			auto prev_name = full_name.substr(0, full_name.find_first_of('$'));
			if (prev_name == short_name)
				return;
			full_name = short_name + full_name.substr(full_name.find_first_of('$'), full_name.size());
		}
	}


	type_node::type_node(utils::classes::stringi8 type_string) :
		type_str{ std::move(type_string) }
	{}

	number_literal_node::number_literal_node(utils::classes::stringi8&& lexeme_, uint32_t bit_count_) :
		lexeme{ std::move(lexeme_) }, bit_count{ bit_count_ }
	{}
	number_literal_node::number_literal_node(const utils::classes::stringi8& lexeme_, uint32_t bit_count_) :
		lexeme{ lexeme_ }, bit_count{ bit_count_ }
	{}

	string_literal_node::string_literal_node(utils::classes::stringi8&& lexeme_) : literal{ std::move(lexeme_) }
	{}
	string_literal_node::string_literal_node(const utils::classes::stringi8& lexeme_) : literal{ lexeme_ }
	{}

	binary_operator_node::binary_operator_node(token::token_type operator_, expr_node_ptr left_, expr_node_ptr right_) :
		op{ std::move(operator_) }, left{ std::move(left_) }, right{ std::move(right_) }
	{}

	unary_operator_node::unary_operator_node(token::token_info operator_, expr_node_ptr child_) :
		op{ std::move(operator_) }, child{ std::move(child_) }
	{}

	named_node::named_node(utils::classes::stringi8 thing_name) :
		name{ std::move(thing_name) }
	{}

	variable_node::variable_node(utils::classes::stringi8 variable) :
		named_node{ std::move(variable) }
	{}

	type_variable_node::type_variable_node(variable_node* var_ptr) :
		named_node{ std::move(var_ptr->name) }
	{}
	type_variable_node::type_variable_node(variable_node_ptr var) :
		named_node{ std::move(var->name) }
	{}
	type_variable_node::type_variable_node(utils::classes::stringi8 variable) :
		named_node{ std::move(variable) }
	{}

	function_node::function_node(utils::classes::stringi8 variable) :
		name{ std::make_unique<detail::name_info>(std::move(variable)) }
	{}

	argument_node::argument_node() :
		value{ nullptr }, type_str{ "" }
	{}
	argument_node::argument_node(argument_node&& rhs) noexcept :
		value{ std::move(rhs.value) }, type_str{ std::move(rhs.type_str) }
	{}
	argument_node::argument_node(expr_node_ptr value_, utils::classes::stringi8 argument_type_name) :
		value{ std::move(value_) }, type_str{ std::move(argument_type_name) }
	{}

	function_arguments_node::function_arguments_node() :
		args{}
	{}

	marker_node::marker_node(token::token_info marker) :
		marker_expr{ std::move(marker) }
	{}

	function_definition_node::function_definition_node(function_node_ptr func, function_parameters_node_ptr params) :
		function{ std::move(func) }, parameters{ std::move(params) }, function_type{ nullptr }
	{}
	function_definition_node::function_definition_node(function_node_ptr func, function_parameters_node_ptr params, type_node_ptr type) :
		function{ std::move(func) }, parameters{ std::move(params) }, function_type{ std::move(type) }
	{}

	function_call_node::function_call_node(function_definition_node_ptr func_def) :
		arguments{ std::make_unique<function_arguments_node>() }, function{ std::move(func_def->function) }, function_type{ std::move(func_def->function_type) }
	{
		for (auto&& param : func_def->parameters->names)
		{
			auto&& arg = std::make_unique<expr::variable_node>(std::move(param));
			arguments->args.emplace_back(make_argument(std::move(arg)));
		}
	}
	function_call_node::function_call_node(function_node_ptr func, function_arguments_node_ptr args, type_node_ptr ret_function_type) :
		function{ std::move(func) }, arguments{ std::move(args) }, function_type{ std::move(ret_function_type) }
	{}
	function_call_node::function_call_node(function_node_ptr func, function_arguments_node_ptr args) :
		function{ std::move(func) }, arguments{ std::move(args) }, function_type{ nullptr }
	{}

	field_call_node::field_call_node(expr_node_ptr parent_, expr_node_ptr child_) :
		parent{ std::move(parent_) }, child{ std::move(child_) }, next_field_call{ nullptr }
	{}
	field_call_node::field_call_node(expr_node_ptr parent_, expr_node_ptr child_, std::unique_ptr<field_call_node> next_field_call_) :
		parent{ std::move(parent_) }, child{ std::move(child_) }, next_field_call{ std::move(next_field_call_) }
	{}

	variable_reference_node::variable_reference_node(variable_node_ptr var) :
		variable{ std::move(var) }
	{}

	variable_assignment_expr::variable_assignment_expr(variable_node_ptr var, expr_node_ptr right_expression) :
		binary_operator_node{ ul::dictionaries::language_dictionary::match_pattern("="), std::move(var), std::move(right_expression) }
	{}

	dynamic_malloc_expr::dynamic_malloc_expr(expr::expr_node_ptr rhs_) :
		rhs{ std::move(rhs_) }
	{}

	dynamic_malloc_expr::dynamic_malloc_expr(expr::expr_node_ptr rhs_, bool is_single_) :
		rhs{ std::move(rhs_) }, is_single{ is_single_ }
	{}

	nameof_expr::nameof_expr(variable_node_ptr var) :
		variable{ std::move(var) }
	{}

	dynamic_free_expr::dynamic_free_expr(utils::classes::stringi8 var_name) :
		variable_name{ std::move(var_name) }
	{}

	utils::classes::stringi8 get_type_of_expression(expr_node& expression)
	{
		auto* exptr = &expression;
		if (auto* n = dynamic_cast<string_literal_node*>(exptr))
		{
			return "str";
		}
		else if(auto* n = dynamic_cast<nameof_expr*>(exptr))
		{
			return "str";
		}
		else if(auto* n = dynamic_cast<variable_reference_node*>(exptr))
		{
			return "ptr";
		}
		else if (auto* n = dynamic_cast<number_literal_node*>(exptr))
		{
			return std::format("int{}", n->bit_count);
		}
		else if (auto* n = dynamic_cast<variable_node*>(exptr))
		{
			return n->name.get_type_from_name();
		}
		else if(auto* n = dynamic_cast<binary_operator_node*>(exptr))
		{
			if(get_type_of_expression(*n->left) == get_type_of_expression(*n->right))
			{
				return get_type_of_expression(*n->left);
			}
			else
			{
				EXCEPTION("Left and right operands of binary operator have different types");
			}
		}
		else
			EXCEPTION("Expression of this type have not any type");
	}
	expr::argument_node_ptr make_argument(expr_node_ptr expression)
	{
		return std::make_unique<argument_node>
			(std::move(expression), std::move(get_type_of_expression(*expression)));
	}

}
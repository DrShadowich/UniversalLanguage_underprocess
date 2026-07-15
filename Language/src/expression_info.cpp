#include <expression_info.h>
#include <core_exceptions.h>
namespace ul::expr
{
	std::string get_type_of_expression(expr_node& expression)
	{
		auto* exptr = &expression;
		if (auto* n = dynamic_cast<string_literal_node*>(exptr))
		{
			return "str";
		}
		else if (auto* n = dynamic_cast<number_literal_node*>(exptr))
		{
			return std::format("int{}", n->bit_count);
		}
		else if (auto* n = dynamic_cast<variable_node*>(exptr))
		{
			return utils::get_type_from_name(n->name);
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


	function_call_node::function_call_node(function_definition_node_ptr func_def) :
		arguments{ std::make_unique<function_arguments_node>() }, function{ std::move(func_def->function) }, function_type{ std::move(func_def->function_type) }
	{
		for (auto&& param : func_def->parameters->names)
		{
			auto&& arg = std::make_unique<expr::variable_node>(std::move(param));
			arguments->args.emplace_back(make_argument(std::move(arg)));
		}
	}

}
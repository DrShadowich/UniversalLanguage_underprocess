#pragma once
#include <token_info.h>
#include <memory>
namespace ul::expr
{
	struct expr_node
	{
		virtual ~expr_node() = default;
	};
	using expr_node_ptr = std::unique_ptr<expr_node>;

	struct number_literal_node : public expr_node
	{
		std::string lexeme;
		explicit number_literal_node(std::string&& lexeme_) : lexeme{ std::move(lexeme_) }
		{}
		explicit number_literal_node(const std::string& lexeme_) : lexeme{ lexeme_ }
		{}
	};
	using number_literal_node_ptr = std::unique_ptr<number_literal_node>;
	
	struct string_literal_node : public expr_node
	{
		std::string lexeme;
		explicit string_literal_node(std::string&& lexeme_) : lexeme{ std::move(lexeme_) }
		{}
		explicit string_literal_node(const std::string& lexeme_) : lexeme{ lexeme_ }
		{}
	};
	using string_literal_node_ptr = std::unique_ptr<string_literal_node>;
	
	struct binary_operator_node : public expr_node
	{
		token::token_info op;
		expr_node_ptr left;
		expr_node_ptr right;
		explicit binary_operator_node(token::token_info& operator_, expr_node_ptr left_, expr_node_ptr right_) :
			op{ operator_ }, left{ std::move(left_) }, right{ std::move(right_) }
		{}
	};
	using binary_operator_node_ptr = std::unique_ptr<binary_operator_node>;

	struct unary_operator_node : public expr_node
	{
		token::token_info op;
		expr_node_ptr child;
		explicit unary_operator_node(token::token_info operator_, expr_node_ptr child_) :
			op{ std::move(operator_) }, child { std::move(child_) }
		{}

	};
	using unary_operator_node_ptr = std::unique_ptr<unary_operator_node>;

	// !!! DO NOT USE "named_node" AS INDEPENDENT NODE !!!
	struct named_node : public expr_node
	{
		std::string name;
		named_node(std::string thing_name) :
			name{ std::move(thing_name) }
		{}
	};
	using named_node_ptr = std::unique_ptr<named_node>;

	struct variable_node : public named_node
	{
		explicit variable_node(std::string variable) : 
			named_node{ std::move(variable) }
		{}
	};
	using variable_node_ptr = std::unique_ptr<variable_node>;

	struct function_node : public named_node
	{
		explicit function_node(std::string variable) :
			named_node{ std::move(variable) }
		{}
	};
	using function_node_ptr = std::unique_ptr<function_node>;

	struct function_arguments_node : public expr_node
	{
		std::vector<expr::expr_node_ptr> args;
	};
	using function_arguments_node_ptr = std::unique_ptr<function_arguments_node>;

	struct function_call_node : public expr_node
	{
		std::unique_ptr<function_arguments_node> arguments;
		std::unique_ptr<function_node> function;
		explicit function_call_node(std::unique_ptr<function_node> func, std::unique_ptr<function_arguments_node> args) :
			function{ std::move(func) }, arguments{ std::move(args) } 
		{}
	};
	using function_call_node_ptr = std::unique_ptr<function_call_node>;

	struct newline_node : public expr_node
	{};
	struct marker_node : public expr_node
	{
		token::token_info marker_expr;
		explicit marker_node(token::token_info marker) :
			marker_expr{ std::move(marker) }
		{}
	};

	struct field_call_node : public expr_node
	{
		expr_node_ptr parent;
		expr_node_ptr child;
		std::unique_ptr<field_call_node> next_field_call;
		explicit field_call_node(expr_node_ptr parent_, expr_node_ptr child_) :
			parent{ std::move(parent_) }, child{ std::move(child_) }, next_field_call{ nullptr }
		{}
		explicit field_call_node(expr_node_ptr parent_, expr_node_ptr child_, std::unique_ptr<field_call_node> next_field_call_) :
			parent{ std::move(parent_) }, child{ std::move(child_) }, next_field_call{ std::move(next_field_call_) }
		{}
	};
	using field_call_node_ptr = std::unique_ptr<field_call_node>;

	struct return_value_node : public expr_node
	{
		expr_node_ptr value;
		explicit return_value_node(expr_node_ptr value_) :
			value{ std::move(value_) }
		{}
	};
	using return_value_node_ptr = std::unique_ptr<return_value_node>;

}
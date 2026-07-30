#pragma once
#include <token_info.h>
#include <simple_variable.h>
#include <source_location_ul.h>
#include <memory>
#include <lexer.h>
namespace ul::expr
{
	namespace detail
	{
		// Bridge functin_name_info -> name_info
		struct name_info
		{
			// full name from function_name_info
			std::string full_name;
			// only name
			std::string short_name;
			// return type
			std::string type_str;
			name_info() noexcept(std::is_nothrow_constructible_v<std::string>);
			name_info(std::string f, std::string s, std::string t) noexcept(std::is_nothrow_constructible_v<std::string, std::string&&>);
			name_info(std::string s) noexcept(std::is_nothrow_constructible_v<std::string>);
			name_info(name_info&& rhs) noexcept;
			void rename(const std::string& new_name);
			operator std::string() { return full_name; }
		};
		using name_info_ptr = std::unique_ptr<name_info>;
	}
	// Basic main node
	struct expr_node
	{
		expr_node() = default;
		virtual ~expr_node() = default;
	};
	using expr_node_ptr = std::unique_ptr<expr_node>;

	struct type_node : public expr_node
	{
		std::string type_str;
		explicit type_node(std::string type_string);
	};
	using type_node_ptr = std::unique_ptr<type_node>;

	struct number_literal_node : public expr_node
	{
		std::string lexeme;
		// Переделай в enum
		uint32_t bit_count;
		explicit number_literal_node(std::string&& lexeme_, uint32_t bit_count_);
		explicit number_literal_node(const std::string& lexeme_, uint32_t bit_count_);
	};
	using number_literal_node_ptr = std::unique_ptr<number_literal_node>;
	
	struct string_literal_node : public expr_node
	{
		std::string literal;
		explicit string_literal_node(std::string&& lexeme_);
		explicit string_literal_node(const std::string& lexeme_);
	};
	using string_literal_node_ptr = std::unique_ptr<string_literal_node>;
	
	struct binary_operator_node : public expr_node
	{
		token::token_type op;
		expr_node_ptr left;
		expr_node_ptr right;
		explicit binary_operator_node(token::token_type operator_, expr_node_ptr left_, expr_node_ptr right_);
	};
	using binary_operator_node_ptr = std::unique_ptr<binary_operator_node>;

	struct logical_binary_operator_node : public binary_operator_node
	{
		using binary_operator_node::binary_operator_node;
	};
	using logical_binary_operator_node_ptr = std::unique_ptr<logical_binary_operator_node>;

	struct unary_operator_node : public expr_node
	{
		token::token_info op;
		expr_node_ptr child;
		explicit unary_operator_node(token::token_info operator_, expr_node_ptr child_);
	};
	using unary_operator_node_ptr = std::unique_ptr<unary_operator_node>;

	// !!! DO NOT USE "named_node" AS INDEPENDENT NODE !!!
	struct named_node : public expr_node
	{
		std::string name;
		explicit named_node(std::string thing_name);
	};
	using named_node_ptr = std::unique_ptr<named_node>;

	struct variable_node : public named_node
	{
		explicit variable_node(std::string variable);
	};
	using variable_node_ptr = std::unique_ptr<variable_node>;

	struct type_variable_node : public named_node
	{
		explicit type_variable_node(variable_node* var_ptr);
		explicit type_variable_node(variable_node_ptr var);
		explicit type_variable_node(std::string variable);
	};
	using type_variable_node_ptr = std::unique_ptr<type_variable_node>;

	struct function_node : public expr_node
	{
		bool is_extern = false;
		detail::name_info_ptr name{ nullptr };
		explicit function_node(std::string variable);
	};
	using function_node_ptr = std::unique_ptr<function_node>;

	struct function_parameters_node : public expr_node
	{
		std::vector<std::string> names;
		std::vector<expr::type_variable_node_ptr> types;
		bool va_args = false;
	};
	using function_parameters_node_ptr = std::unique_ptr< function_parameters_node>;
	

	struct argument_node : public expr_node
	{
		expr_node_ptr value;
		std::string type_str;
		argument_node();
		argument_node(argument_node&& rhs) noexcept;
		explicit argument_node(expr_node_ptr value_, std::string argument_type_name);
	};
	using argument_node_ptr = std::unique_ptr<argument_node>;

	struct function_arguments_node : public expr_node
	{
		std::vector<argument_node_ptr> args;
		explicit function_arguments_node();
	};
	using function_arguments_node_ptr = std::unique_ptr<function_arguments_node>;

	struct newline_node : public expr_node{};
	struct break_node : public expr_node{};
	struct continue_node : public expr_node{};
	
	struct marker_node : public expr_node
	{
		token::token_info marker_expr;
		explicit marker_node(token::token_info marker);
	};
	using marker_node_ptr = std::unique_ptr<marker_node>;


	struct function_with_va_args_node : public expr_node
	{
	};
	using function_with_va_args_node_ptr = std::unique_ptr<function_with_va_args_node>;

	struct function_definition_node : public expr_node
	{
		function_parameters_node_ptr parameters;
		function_node_ptr function;
		type_node_ptr function_type;
		explicit function_definition_node(function_node_ptr func, function_parameters_node_ptr params);
		explicit function_definition_node(function_node_ptr func, function_parameters_node_ptr params, type_node_ptr type);
	};
	using function_definition_node_ptr = std::unique_ptr<function_definition_node>;
	
	struct function_call_node : public expr_node
	{
		function_arguments_node_ptr arguments;
		function_node_ptr function;
		type_node_ptr function_type;

		// Transform definition with params to call with named args
		explicit function_call_node(function_definition_node_ptr func_def);

		explicit function_call_node(function_node_ptr func, function_arguments_node_ptr args, type_node_ptr ret_function_type);
		explicit function_call_node(function_node_ptr func, function_arguments_node_ptr args);
	};
	using function_call_node_ptr = std::unique_ptr<function_call_node>;

	struct field_call_node : public expr_node
	{
		expr_node_ptr parent;
		expr_node_ptr child;
		std::unique_ptr<field_call_node> next_field_call;
		explicit field_call_node(expr_node_ptr parent_, expr_node_ptr child_);
		explicit field_call_node(expr_node_ptr parent_, expr_node_ptr child_, std::unique_ptr<field_call_node> next_field_call_);
	};
	using field_call_node_ptr = std::unique_ptr<field_call_node>;

	struct variable_reference_node : public expr_node
	{
		variable_node_ptr variable;
		explicit variable_reference_node(variable_node_ptr var);
	};
	using variable_reference_node_ptr = std::unique_ptr<variable_reference_node>;

	struct variable_assignment_expr : public binary_operator_node
	{
		explicit variable_assignment_expr(variable_node_ptr var, expr_node_ptr right_expression);
	};
	struct variable_additional_assignment_expr : public binary_operator_node
	{
		using binary_operator_node::binary_operator_node;
	};
	using variable_assignment_expr_ptr = std::unique_ptr<variable_assignment_expr>;
	// Get type if expression is var, string lexeme, number lexeme.
	std::string get_type_of_expression(expr_node& expression);
	expr::argument_node_ptr make_argument(expr_node_ptr expression);
}
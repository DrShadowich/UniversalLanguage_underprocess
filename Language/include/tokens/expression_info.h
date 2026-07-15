#pragma once
#include <token_info.h>
#include <simple_variable.h>
#include <memory>
namespace ul::expr
{
	namespace detail
	{
		struct name_info
		{
			// full name from function_name_info
			std::string full_name;
			// only name
			std::string short_name;
			// return type
			std::string type_str;
			name_info(std::string f, std::string s, std::string t) :
				full_name{ std::move(f) }, short_name{ std::move(s) }, type_str{ std::move(t) }
			{}
			name_info() :
				full_name{}, short_name{}, type_str{}
			{}
			name_info(name_info&& rhs) noexcept :
				full_name{ std::move(rhs.full_name) }, short_name{ std::move(rhs.short_name) }, type_str{ std::move(rhs.type_str) }
			{}
			void rename(const std::string& new_name)
			{
				short_name = new_name;
				auto prev_name = full_name.substr(0, full_name.find_first_of('$'));
				if (prev_name == short_name)
					return;
				full_name = short_name + full_name.substr(full_name.find_first_of('$'), full_name.size());
			}
			operator std::string() { return full_name; }
		};
		using name_info_ptr = std::unique_ptr<name_info>;
	}
	struct expr_node
	{
		virtual ~expr_node() = default;
	};
	using expr_node_ptr = std::unique_ptr<expr_node>;

	struct type_node : public expr_node
	{
		std::string type_str;
		explicit type_node(std::string type_string) :
			type_str{ std::move(type_string) }
		{
		}
	};
	using type_node_ptr = std::unique_ptr<type_node>;

	struct number_literal_node : public expr_node
	{
		std::string lexeme;
		// Переделай в enum
		uint32_t bit_count;
		explicit number_literal_node(std::string&& lexeme_, uint32_t bit_count_) :
			lexeme{ std::move(lexeme_) }, bit_count{ bit_count_ }
		{}
		explicit number_literal_node(const std::string& lexeme_, uint32_t bit_count_) :
			lexeme{ lexeme_ }, bit_count{ bit_count_ }
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

	struct type_variable_node : public named_node
	{
		explicit type_variable_node(variable_node* var_ptr) :
			named_node{ std::move(var_ptr->name) }
		{}
		explicit type_variable_node(variable_node_ptr var) :
			named_node{ std::move(var->name) }
		{}
		explicit type_variable_node(std::string variable) :
			named_node{ std::move(variable) }
		{}
	};
	using type_variable_node_ptr = std::unique_ptr<type_variable_node>;

	struct function_node : public expr_node
	{
		bool is_extern = false;
		detail::name_info_ptr name{ nullptr };
		explicit function_node(std::string variable) :
			name{ std::make_unique<detail::name_info>() }
		{
			name->short_name = std::move(variable);
		}
	};
	using function_node_ptr = std::unique_ptr<function_node>;

	struct function_parameters_node : public expr_node
	{
		std::vector<std::string> names;
		std::vector<expr::type_variable_node_ptr> types;
	};
	using function_parameters_node_ptr = std::unique_ptr< function_parameters_node>;
	

	struct argument_node : public expr_node
	{
		expr_node_ptr value;
		std::string type_str;
		argument_node() : 
			value{ nullptr }, type_str{ "" }
		{}
		argument_node(argument_node&& rhs) noexcept :
			value{ std::move(rhs.value) }, type_str{ std::move(rhs.type_str) }
		{}
		explicit argument_node(expr_node_ptr value_, std::string argument_type_name) :
			value{ std::move(value_) }, type_str{ std::move(argument_type_name) }
		{}
	};
	using argument_node_ptr = std::unique_ptr<argument_node>;

	struct function_arguments_node : public expr_node
	{
		std::vector<argument_node_ptr> args;
		explicit function_arguments_node(){}
	};
	using function_arguments_node_ptr = std::unique_ptr<function_arguments_node>;

	

	struct newline_node : public expr_node
	{};
	struct marker_node : public expr_node
	{
		token::token_info marker_expr;
		explicit marker_node(token::token_info marker) :
			marker_expr{ std::move(marker) }
		{}
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
		bool va_args = false;
		explicit function_definition_node(function_node_ptr func, function_parameters_node_ptr params) :
			function{ std::move(func) }, parameters{ std::move(params) }, function_type{ nullptr }
		{}
		explicit function_definition_node(function_node_ptr func, function_parameters_node_ptr params, type_node_ptr type) :
			function{ std::move(func) }, parameters{ std::move(params) }, function_type{ std::move(type) }
		{}
	};
	using function_definition_node_ptr = std::unique_ptr<function_definition_node>;
	
	struct function_call_node : public expr_node
	{
		function_arguments_node_ptr arguments;
		function_node_ptr function;
		type_node_ptr function_type;

		explicit function_call_node(function_definition_node_ptr func_def);

		explicit function_call_node(function_node_ptr func, function_arguments_node_ptr args, type_node_ptr ret_function_type) :
			function{ std::move(func) }, arguments{ std::move(args) }, function_type{ std::move(ret_function_type) }
		{}
		explicit function_call_node(function_node_ptr func, function_arguments_node_ptr args) :
			function{ std::move(func) }, arguments{ std::move(args) }, function_type{ nullptr }
		{}
	};
	using function_call_node_ptr = std::unique_ptr<function_call_node>;

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
		std::string type_str;
		explicit return_value_node(expr_node_ptr value_) :
			value{ std::move(value_) }
		{}
		explicit return_value_node(expr_node_ptr value_, std::string return_type_name) :
			value{ std::move(value_) }, type_str{ std::move(return_type_name) }
		{}
	};
	using return_value_node_ptr = std::unique_ptr<return_value_node>;

	// Get type if expression is var, string lexeme, number lexeme.
	std::string get_type_of_expression(expr_node& expression);
	expr::argument_node_ptr make_argument(expr_node_ptr expression);
}
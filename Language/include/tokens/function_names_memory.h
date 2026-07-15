#pragma once
#include <function_name_info.h>
#include <expression_info.h>
#include <unordered_map>
#include <vector>
#include <set>

namespace ul::parser
{
	struct function_names_memory final
	{
	private:
		// function pattern:
		// function_fn$res_type|argument_type|argument2_type|...%
		std::unordered_map<std::string, std::vector<function_name_info>> function_names{};
	public:
		// Set all same named functions with type;
		void set_functions_return_type(std::string* fn_name, const std::string& fn_type);
		std::string get_return_type_of_function(const std::string& fn_name);
		std::vector<std::string> get_function(const std::string& fn_name);
		expr::detail::name_info_ptr insert_function(expr::function_definition_node& fn_def);
		bool contains_name(const std::string& fn_name);
		bool name_is_extern(const std::string& fn_name);
	};
}
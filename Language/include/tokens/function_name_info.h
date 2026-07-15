#pragma once
#include <string>
#include <vector>
#include <expression_info.h>
namespace ul::parser
{
	struct function_name_info
	{
	private:
		void inname(expr::function_definition_node& fn_def);
	public:
		// get name of function from innamed string
		static std::string get_name(const std::string&);
		// Set full name for function
		static void make_full_name_from_call(expr::function_call_node& fn_calle);
		static expr::function_node_ptr make_named_function(std::string fn_name);
		std::string fn_name;
		std::string fn_type;
		std::vector<std::string> fn_types;
		bool is_extern = false;
		function_name_info();
		function_name_info(std::string full_name);
		function_name_info(expr::function_definition_node& fn_def);
		function_name_info(const function_name_info&);
		function_name_info(function_name_info&&) noexcept;
		function_name_info& operator=(const function_name_info&);
		function_name_info& operator=(function_name_info&&) noexcept;
		std::string get_full_name();
		bool operator==(expr::function_definition_node& fn_def);
		bool operator==(const std::string& full_name);
		bool operator==(const function_name_info& rhs);
		operator std::string();
		operator expr::detail::name_info();
	};
}
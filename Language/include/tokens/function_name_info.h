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
		static utils::classes::stringi8 get_name(const utils::classes::stringi8&);
		// Set full name for function
		static void make_full_name_from_call(expr::function_call_node& fn_calle);
		static expr::function_node_ptr make_named_function(utils::classes::stringi8 fn_name);
		utils::classes::stringi8 fn_name;
		utils::classes::stringi8 fn_type;
		std::vector<utils::classes::stringi8> fn_types;
		bool is_extern = false;
		function_name_info();
		function_name_info(utils::classes::stringi8 full_name);
		function_name_info(expr::function_definition_node& fn_def);
		function_name_info(const function_name_info&);
		function_name_info(function_name_info&&) noexcept;
		function_name_info& operator=(const function_name_info&);
		function_name_info& operator=(function_name_info&&) noexcept;
		utils::classes::stringi8 get_full_name();
		bool operator==(expr::function_definition_node& fn_def);
		bool operator==(const utils::classes::stringi8& full_name);
		bool operator==(const function_name_info& rhs);
		operator utils::classes::stringi8();
		operator expr::detail::name_info();
	};
}
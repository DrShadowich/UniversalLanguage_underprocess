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
		std::unordered_map<utils::classes::stringi8, std::vector<function_name_info>> function_names{};
	public:
		// Set all same named functions with type;
		void set_functions_return_type(utils::classes::stringi8* fn_name, const utils::classes::stringi8& fn_type);
		utils::classes::stringi8 get_return_type_of_function(const utils::classes::stringi8& fn_name);
		std::vector<utils::classes::stringi8> get_function(const utils::classes::stringi8& fn_name);
		expr::detail::name_info_ptr insert_function(expr::function_definition_node& fn_def);
		bool contains_name(const utils::classes::stringi8& fn_name);
		bool name_is_extern(const utils::classes::stringi8& fn_name);
	};
}
#include <function_names_memory.h>
#include <core_exceptions.h>
#include <format>
namespace ul::parser
{

	void function_names_memory::set_functions_return_type(utils::classes::stringi8* fn_name, const utils::classes::stringi8& fn_type)
	{
		utils::classes::stringi8 fn_short_name = std::move(function_name_info::get_name(*fn_name));
		if (!function_names.contains(fn_short_name))
			PARSER_EXCEPTION(std::format("There\'s no function named ", fn_short_name));
		for(auto& ofn : function_names[fn_short_name])
		{
			if (!ofn.fn_type.empty() && fn_type != ofn.fn_type)
				PARSER_EXCEPTION("Not empty type already set and not equal to previous type");
			else
			{
				ofn.fn_type = fn_type;
				*fn_name = std::move(ofn.get_full_name());
			}
		}
	}

	expr::detail::name_info_ptr function_names_memory::insert_function(expr::function_definition_node& fn_def)
	{
		expr::detail::name_info_ptr name = std::make_unique< expr::detail::name_info>();
		function_name_info info{ fn_def };
		info.is_extern = fn_def.function->is_extern;

		name->full_name = std::move(info.get_full_name());
		name->short_name = info.fn_name;
		name->type_str = info.fn_type;

		if (!function_names.contains(info.fn_name))
		{
			utils::classes::stringi8 name_ = info.fn_name;
			std::vector<function_name_info> vec;
			vec.emplace_back(std::move(info));
			function_names.insert({ std::move(name_), std::move(vec)});
		}
		else
			function_names[info.fn_name].emplace_back(std::move(info));
		return name;
	}
	
	std::vector<utils::classes::stringi8> function_names_memory::get_function(const utils::classes::stringi8& fn_name)
	{
		std::vector<utils::classes::stringi8> ret_vec;
		for(auto&& ofn : function_names[fn_name])
		{
			ret_vec.push_back(ofn);
		}
		return ret_vec;
	}

	bool function_names_memory::contains_name(const utils::classes::stringi8& fn_name)
	{
		return function_names.contains(fn_name);
	}

	utils::classes::stringi8 function_names_memory::get_return_type_of_function(const utils::classes::stringi8& fn_name)
	{
		utils::classes::stringi8 prev_fn_type{ function_names[fn_name][0].fn_type };
		for(auto&& fn : function_names[fn_name])
		{
			if (prev_fn_type != fn.fn_type)
				PARSER_EXCEPTION("Type of function isn\'t same");
			prev_fn_type = fn.fn_type;
		}
		return prev_fn_type;
	}

	bool function_names_memory::name_is_extern(const utils::classes::stringi8& fn_name)
	{
		bool prev_flag = function_names[fn_name][0].is_extern;
		for(auto&& fn : function_names[fn_name])
		{
			if(fn.is_extern != prev_flag)
				PARSER_EXCEPTION("Function\'s extern flag isn\'t same");
			prev_flag = fn.is_extern;
		}
		return prev_flag;
	}
}
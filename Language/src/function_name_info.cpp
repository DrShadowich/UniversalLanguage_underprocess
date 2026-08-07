#include <function_name_info.h>
#include <expression_info.h>
#include <format>
#include <core_exceptions.h>

namespace ul::parser
{
	utils::classes::stringi8 function_name_info::get_name(const utils::classes::stringi8& name)
	{
		if (name.empty())
			PARSER_EXCEPTION("Expected name");
		size_t pos = name.find_first_of('$');
		utils::classes::stringi8 ret_value = std::move(name.substr(0, pos));
		if (ret_value.empty())
			PARSER_EXCEPTION("Got empty name");
		return ret_value;
	}

	void function_name_info::make_full_name_from_call(expr::function_call_node& fn_calle)
	{
		function_name_info fni;

		fni.fn_name = fn_calle.function->name->short_name;
		fni.fn_type = fn_calle.function_type->type_str;

		for (auto&& arg : fn_calle.arguments->args)
		{
			fni.fn_types.push_back(arg->type_str);
		}

		fn_calle.function->name->full_name = std::move(fni.get_full_name());
	}

	utils::classes::stringi8 function_name_info::get_full_name()
	{
		utils::classes::stringi8 full_name = std::format("{}$|{}", fn_name, fn_type);
		for (auto&& type_str : fn_types)
			full_name += std::format("|{}", type_str);
		full_name.push_back('!');
		return full_name;
	}
	function_name_info::function_name_info() :
		fn_type{}, fn_name{}, fn_types{}
	{}
	void function_name_info::inname(expr::function_definition_node& fn_def)
	{
		if (fn_def.function->name->short_name.empty())
			PARSER_EXCEPTION("Unable to inname function definition while function name is empty");
		fn_name = fn_def.function->name->short_name;
		fn_type = fn_def.function_type->type_str;
		fn_def.function->name->full_name += std::move(std::format("{}$|{}", fn_name, fn_type));
		for (auto&& type_str : fn_def.parameters->names)
		{
			auto type = type_str.get_type_from_name();
			fn_types.push_back(type);
			fn_def.function->name->full_name += std::move(std::format("|{}", std::move(type)));
		}
		if(fn_def.parameters->va_args)
		{
			fn_def.function->name->full_name += "|...!";
			fn_types.emplace_back("...");
		}
		else
		{
			fn_def.function->name->full_name.push_back('!');
		}
	}
	function_name_info::function_name_info(expr::function_definition_node& fn_def)
	{
		this->inname(fn_def);
	}
	function_name_info::function_name_info(utils::classes::stringi8 full_name)
	{
		size_t func_name_end_symbol_pos = full_name.find_first_of('$');
		fn_name = std::move(full_name.substr(0, func_name_end_symbol_pos));
		utils::classes::stringi8 types = full_name.substr(func_name_end_symbol_pos + 2, full_name.size());
		utils::classes::stringi8 type;
		size_t return_type_pos = types.find_first_of('|');
		if (!return_type_pos)
			fn_type = "";
		else fn_type = std::move(types.substr(0, return_type_pos));
		types = std::move(types.substr(return_type_pos + 1, types.size()));
		for(char c : types)
		{
			switch(c)
			{
			case '|': 
				fn_types.emplace_back(std::move(type));
				type = "";
				continue;
			case '!':
				fn_types.emplace_back(std::move(type));
				type = "";
				return;
			default:
				type += c;
				continue;
			}
		}
	}

	function_name_info::function_name_info(const function_name_info& rhs) :
		fn_types{ rhs.fn_types }, fn_name{ rhs.fn_name }, fn_type{ rhs.fn_type }, is_extern{ rhs.is_extern }
	{}
	function_name_info::function_name_info(function_name_info&& rhs) noexcept :
		fn_types{ std::move(rhs.fn_types) }, fn_name{ std::move(rhs.fn_name) }, fn_type{ std::move(rhs.fn_type) }, is_extern{ rhs.is_extern }
	{}
	function_name_info& function_name_info::operator=(const function_name_info& rhs)
	{
		fn_type = rhs.fn_type;
		fn_name = rhs.fn_name;
		fn_types = rhs.fn_types;
		is_extern = rhs.is_extern;
		return *this;
	}
	function_name_info& function_name_info::operator=(function_name_info&& rhs) noexcept
	{
		fn_type = std::move(rhs.fn_type);
		fn_name = std::move(rhs.fn_name);
		fn_types = std::move(rhs.fn_types);
		is_extern = rhs.is_extern;
		return *this;
	}
	bool function_name_info::operator==(const utils::classes::stringi8& full_name)
	{
		return this->get_full_name() == full_name;
	}
	bool function_name_info::operator==(const function_name_info& rhs)
	{
		return fn_name == rhs.fn_name && fn_type == rhs.fn_type && fn_types == rhs.fn_types;
	}
	bool function_name_info::operator==(expr::function_definition_node& fn_def)
	{
		return get_full_name() == fn_def.function->name->full_name;
	}
	function_name_info::operator utils::classes::stringi8()
	{
		return this->get_full_name();
	}
	function_name_info::operator expr::detail::name_info()
	{
		return expr::detail::name_info{ get_full_name(), fn_name, fn_type };
	}
}
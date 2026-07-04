#pragma once
#include <string>
#include <string_view>
#include <core_exceptions.h>
namespace utils
{
	std::string get_type_from_name(std::string_view name)
	{
		size_t type_pos = name.find_last_of('_');
		if (type_pos == std::string::npos)
			EXCEPTION(std::format("No any type in {}", name));
		std::string type_str = std::move(static_cast<std::string>(name.substr(type_pos + 1)));
		return type_str;
	}
}
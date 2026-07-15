#pragma once
#include <string>
#include <string_view>
#include <core_exceptions.h>
namespace ul::utils
{
	namespace detail
	{
		size_t get_type_and_name_delimeter(std::string_view name);
	}
	bool has_type(std::string_view name);
	std::string get_type_from_name(std::string_view name);
	std::string get_name_without_type(std::string_view name);
}
#include <simple_variable.h>
namespace ul::utils
{
	size_t detail::get_type_and_name_delimeter(std::string_view name)
	{
		if (name.empty())
			EXCEPTION("\'name\' is empty");
		size_t type_pos = name.find_last_of('_');
		if (type_pos == std::string::npos)
			EXCEPTION(std::format("No any type in {}", name));
		return type_pos;
	}
	std::string get_type_from_name(std::string_view name)
	{
		std::string type_str = std::move(static_cast<std::string>(name.substr(detail::get_type_and_name_delimeter(name) + 1)));
		return type_str;
	}

	std::string get_name_without_type(std::string_view name)
	{
		std::string name_str = std::move(static_cast<std::string>(name.substr(0, detail::get_type_and_name_delimeter(name))));
		return name_str;
	}

	bool has_type(std::string_view name)
	{
		return name.find_last_of('_') != std::string::npos;
	}
}
#include <simple_variable.h>
#include <core_exceptions.h>
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

	std::string get_string_literal(std::string literal)
	{
		std::string ret;
		for (size_t i{ 0 }, lsize = literal.size(); i < lsize; ++i)
		{
			if (literal[i] == '\\' && (i + 1) < lsize)
			{
				switch (literal[i + 1])
				{
				case 'n':
					ret += '\n';
					break;
				case 't':
					ret += '\t';
					break;
				case '\\':
					ret += '\\';
					break;
				case '\"':
					ret += '"';
					break;
				case '\'':
					ret += '\'';
					break;
				default:
					PARSER_EXCEPTION(std::format("Unknown escape sequence: \\{}", literal[i + 1]));
				}
				++i;
			}
			else
				ret	+= literal[i];
		}
		std::string_view sv{ ret };
		sv.remove_prefix(1);
		sv.remove_suffix(1);
		return static_cast<std::string>(sv);
	}
}
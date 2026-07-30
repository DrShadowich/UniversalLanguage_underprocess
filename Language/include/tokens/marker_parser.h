#pragma once
#include <regex>
#include <statement_info.h>

#define PARSER ul::parser::

namespace ul::parser
{

	class marker_parser
	{
	private:
		static inline const std::regex marker_regex = std::regex(R"(%(\w*)([\w\S\s]*?)%(\w*))");
	public:
		stmt::marker_statement_ptr parse_marker(std::string marker_lexeme);
	};
}
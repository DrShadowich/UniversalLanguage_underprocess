#include <marker_parser.h>
#include <core_exceptions.h>
#include <sstream>
ul::stmt::marker_statement_ptr PARSER marker_parser::parse_marker(std::string marker_lexeme)
{
	std::smatch match;
	if (!std::regex_search(marker_lexeme, match, marker_regex))
		EXCEPTION("Marker parser get not marker");

	std::string head = match[1];
	std::string body = match[2];
	std::string foot = match[3];
	ul::token::TID marker_type = dictionaries::ul_marker_types.at(head);
	switch(marker_type)
	{
	case token::TID::PYTHON_MARKER:
	{
		std::istringstream is{ body };
		std::string real_code;
		std::string line;

		size_t tab_counter{ 0 };
		do
		{
			std::getline(is, line);
		} while (line.empty());
		auto tab_pos = line.find_first_of('\t');
		while (tab_pos != std::string::npos)
		{
			line = line.substr(tab_pos + 1);
			++tab_counter;
			tab_pos = line.find_first_of('\t');
		}
		real_code += std::format("{}\n", line);


		while (is.good())
		{
			std::getline(is, line);
			
			std::string real_line{};
			if (tab_counter >= line.size())
				continue;
			real_line = line.substr(tab_counter);
			real_code += std::format("{}\n", std::move(real_line));
		}
		body = std::move(real_code);
		break;
	}
	}
	return std::make_unique<stmt::marker_statement>(std::move(head), std::move(body), std::move(foot), marker_type);
}
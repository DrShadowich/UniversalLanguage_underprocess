#include <marker_parser.h>
#include <core_exceptions.h>
ul::stmt::marker_statement_ptr PARSER marker_parser::parse_marker(std::string marker_lexeme)
{
	std::smatch match;
	if (!std::regex_search(marker_lexeme, match, marker_regex))
		EXCEPTION("Marker parser get not marker");

	std::string head = match[1];
	std::string body = match[2];
	std::string foot = match[3];
	ul::token::MID marker_type = dictionaries::ul_marker_types.at(head);
	return std::make_unique<stmt::marker_statement>(std::move(head), std::move(body), std::move(foot), marker_type);
}
#pragma once
#include <string_view>
namespace utils
{
	bool starts_with(char symbol, std::string_view lexeme);
	bool end_with(char symbol, std::string_view lexeme);
	bool covered_with(char symbol, std::string_view lexeme);
}
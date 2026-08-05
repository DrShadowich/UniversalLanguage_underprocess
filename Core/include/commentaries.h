#pragma once
#include <string_view>
#include <string>
namespace ul::utils
{
	bool starts_with(char symbol, std::string_view lexeme);
	bool starts_with(std::string_view symbol, std::string_view lexeme);
	bool end_with(char symbol, std::string_view lexeme);
	bool end_with(std::string_view symbol, std::string_view lexeme);
	bool covered_with(char symbol, std::string_view lexeme);
	std::string trim(std::string_view lexeme);
	std::string trim_assginment(std::string_view lexeme);
	std::string remove_all(char symbol, std::string_view lexeme);
}
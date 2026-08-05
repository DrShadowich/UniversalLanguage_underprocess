#include <commentaries.h>
namespace ul::utils
{
	std::string remove_all(char symbol, std::string_view lexeme)
	{
		if (lexeme.empty())
			return "";
		std::string result;
		for (char c : lexeme)
		{
			if (symbol != c)
				result += c;
		}
		return result;
	}

	bool starts_with(char symbol, std::string_view lexeme)
	{
		return *lexeme.begin() == symbol;
	}
	bool starts_with(std::string_view symbol, std::string_view lexeme)
	{
		if (lexeme.empty())
			return symbol.empty();

		auto last_symbol = lexeme.find_first_of(symbol);
		auto lexemes_symbol = lexeme.substr(last_symbol, symbol.size());
		return symbol == lexemes_symbol;
	}
	bool end_with(char symbol, std::string_view lexeme)
	{
		return *lexeme.rbegin() == symbol;
	}
	bool end_with(std::string_view symbol, std::string_view lexeme)
	{
		if (lexeme.empty())
			return symbol.empty();
		auto last_symbol = lexeme.find_first_of(symbol);
		auto lexemes_symbol = lexeme.substr(last_symbol, lexeme.size());
		return symbol == lexemes_symbol;
	}
	bool covered_with(char symbol, std::string_view lexeme)
	{
		return starts_with(symbol, lexeme) && end_with(symbol, lexeme);
	}
	std::string trim(std::string_view lexeme)
	{
		if (lexeme.empty())
			return "";
		std::string result;
		for(char symbol : lexeme)
		{
			if (not std::isspace(static_cast<unsigned char>(symbol)))
				result += symbol;
		}
		return result;
	}
	std::string trim_assginment(std::string_view lexeme)
	{
		if (lexeme.empty())
			return "";
		std::string result;
		bool is_string = false;
		for (char symbol : lexeme)
		{
			if (symbol == '"')
				is_string = not is_string;
			if (not std::isspace(static_cast<unsigned char>(symbol)) or is_string)
				result += symbol;
		}
		return result;
	}
}
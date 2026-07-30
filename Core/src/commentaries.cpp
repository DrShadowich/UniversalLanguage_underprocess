#include <commentaries.h>
namespace ul::utils
{
	bool starts_with(char symbol, std::string_view lexeme)
	{
		return *lexeme.begin() == symbol ? true : false;
	}
	bool end_with(char symbol, std::string_view lexeme)
	{
		return *lexeme.rbegin() == symbol ? true : false;
	}
	bool covered_with(char symbol, std::string_view lexeme)
	{
		return starts_with(symbol, lexeme) && end_with(symbol, lexeme);
	}
	std::string trim(std::string_view lexeme)
	{
		std::string result;
		for(char symbol : lexeme)
		{
			if (not std::isspace(static_cast<unsigned char>(symbol)))
				result += symbol;
		}
		return result;
	}
}
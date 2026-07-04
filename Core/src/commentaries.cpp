#include <commentaries.h>
namespace utils
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
}
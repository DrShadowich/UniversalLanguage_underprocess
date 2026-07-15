#include <dictionaries.h>
namespace ul::dictionaries
{
	token::token_type language_dictionary::match_pattern(const std::string& lexeme)
	{
		bool state{ false };
		for (auto&& tt : raw)
		{
			state = tt.is_pattern_regex ? std::regex_match(lexeme, *tt.regex_token_pattern) : !tt.token_pattern->compare(lexeme);
			if (state)
			{
				return tt;
			}
		}
		return token::token_type{ token::TYPE_TOKEN_TYPE::NO_TOKEN, "NO_TOKEN", "" };
	}
}
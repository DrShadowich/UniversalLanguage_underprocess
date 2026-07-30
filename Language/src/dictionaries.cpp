#include <dictionaries.h>
#include <core_exceptions.h>
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
		LEXER_EXCEPTION("Token didn\'t get any match");
	}

	std::smatch language_dictionary::search_pattern(const std::string& lexeme)
	{
		std::smatch match;
		auto marker_regex = std::regex(R"(%(\w*)([\w\S\s]*?)%(\w*))");
		
		if (!std::regex_search(lexeme, match, marker_regex))
			LEXER_EXCEPTION("Marker statement wasn\'t found");
		return match;
	}
}
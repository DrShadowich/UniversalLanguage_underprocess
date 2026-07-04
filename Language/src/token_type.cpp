#include <token_type.h>
namespace ul::token
{
	token_type::token_type(){}
	token_type::token_type(const token_type& rhs) :
		is_pattern_regex{ rhs.is_pattern_regex },
		enum_type_string{ rhs.enum_type_string },
		enum_type{ rhs.enum_type }
	{
		if (is_pattern_regex)
			this->regex_token_pattern = std::make_unique<std::regex>(*rhs.regex_token_pattern);
		else
			this->token_pattern = std::make_unique<std::string>(*rhs.token_pattern);
	}
	token_type::token_type(token_type&& rhs) noexcept :
		token_pattern{ std::move(rhs.token_pattern) },
		regex_token_pattern{ std::move(rhs.regex_token_pattern) },
		enum_type_string{ std::move(rhs.enum_type_string) },
		enum_type{ std::move(rhs.enum_type) }
	{}
	token_type& token_type::operator=(token_type&& rhs) noexcept
	{
		this->token_pattern = std::move(rhs.token_pattern);
		this->enum_type = std::move(rhs.enum_type);
		this->enum_type_string = std::move(rhs.enum_type_string);
		this->is_pattern_regex = rhs.is_pattern_regex;
		this->regex_token_pattern = std::move(rhs.regex_token_pattern);
		return *this;
	}
	token_type& token_type::operator=(const token_type& rhs)
	{
		this->enum_type = rhs.enum_type;
		this->enum_type_string = rhs.enum_type_string;
		this->is_pattern_regex = rhs.is_pattern_regex;
		if (this->is_pattern_regex)
			this->regex_token_pattern = std::make_unique<std::regex>(*rhs.regex_token_pattern);
		else
			this->token_pattern = std::make_unique<std::string>(*rhs.token_pattern);
		return *this;
	}
	token_type::token_type(ul::token::TYPE_TOKEN_TYPE token_type_, const char* type_str, std::string pattern) :
		token_pattern{ std::make_unique<std::string>(std::move(pattern)) },
		regex_token_pattern{ nullptr },
		is_pattern_regex{ false },
		enum_type{ token_type_ },
		enum_type_string{ type_str }
	{}
	token_type::token_type(ul::token::TYPE_TOKEN_TYPE token_type_, const char* type_str, std::regex pattern) :
		regex_token_pattern{ std::make_unique<std::regex>(std::move(pattern)) },
		token_pattern{ nullptr },
		is_pattern_regex{ true },
		enum_type{ token_type_ },
		enum_type_string{ type_str }
	{}
}
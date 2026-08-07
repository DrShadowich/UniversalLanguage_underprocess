#pragma once
#include <string>
#include <token_type.h>
namespace ul::token 
{
	struct token_info
	{
		utils::classes::stringi8 lexeme{};
		ul::token::token_type type{};
		int64_t additional_value{};
		token_info(){}
		template<typename StringType>
		token_info(StringType&& lexeme_, token::token_type t_type) :
			lexeme{ std::forward<StringType>(lexeme_) },
			type{ std::move(t_type) }
		{}
		token_info(const  token_info& rhs) : lexeme{ rhs.lexeme }, type{ rhs.type }{}
		token_info(token_info&& rhs) noexcept : lexeme{ std::move(rhs.lexeme) }, type{ std::move(rhs.type) } {}
		token_info& operator=(const token_info& rhs);
		token_info& operator=(token_info&& rhs) noexcept;
		
		
		bool operator==(const token_info& rhs) const { return lexeme == rhs.lexeme && type.enum_type == rhs.type.enum_type; }
		bool operator!=(const token_info& rhs) const { return !(*this == rhs); }
		bool operator==(const token::TID& rhs) const { return type.enum_type == rhs; }
		bool operator!=(const token::TID& rhs) const { return type.enum_type != rhs; }
		bool operator==(const utils::classes::stringi8& rhs) const { return lexeme == rhs; }
		bool operator!=(const utils::classes::stringi8& rhs) const { return !(*this == rhs); }
		
	};
}
#pragma once
#include "tokenizer.h"
#include "separator.h"
#include "token_info.h"
#include "dictionaries.h"
namespace ul::lexer
{
	class language_lexer
	{
	private:
		using tok_info_vector = std::vector<ul::token::token_info>;
		using tok_info_vector_it = tok_info_vector::iterator;
		using tok_info_vector_rit = tok_info_vector::reverse_iterator;
		using tok_info_vector_crit = tok_info_vector::const_reverse_iterator;
		using tok_info_vector_cit = tok_info_vector::const_iterator;
		ul::utils::classes::tokenizer tokenizer_;
		std::string input_information_;
		tok_info_vector ready_tokens_;
		uint32_t cursor_{ 0 };
	public:
		language_lexer() :
			input_information_{ "" }, tokenizer_{ std::move(utils::separator{ " \t\r", "()/+-*:;{},.\n" }) }
		{}
		template<typename StringType>
		language_lexer(StringType&& input) : 
			input_information_{ std::forward<StringType>(input) }, tokenizer_{ std::move(utils::separator{ " \t\r", "()/+-*:;{},.\n" }) }
		{
			get_tokens();
			ready_tokens_.emplace_back("", token::token_type{ token::TYPE_TOKEN_TYPE::END, "END", ""} );
		}
		tok_info_vector_it front();
		void next();
		void flush_cursor();
		tok_info_vector_it begin();
		tok_info_vector_it end();
		tok_info_vector_cit begin() const;
		tok_info_vector_cit end() const;
		tok_info_vector_rit rbegin();
		tok_info_vector_rit rend();
		tok_info_vector_crit rbegin() const;
		tok_info_vector_crit rend() const;
		bool expect(token::TYPE_TOKEN_TYPE);
		bool not_expect(token::TYPE_TOKEN_TYPE);
		bool is_now_break_symbol();
		bool is_end_symbol();
	private:
		void get_tokens();
		template<typename StringType>
		void get_tokens(StringType&& information) 
		{ 
			input_information_ = std::format<StringType>(information);
			get_tokens();
		}

	};
}
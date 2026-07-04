#include <lexer.h>
#include <tokenizer.h>
namespace ul::lexer
{
	void language_lexer::get_tokens()
	{
		token::token_type tt{};
		if (input_information_.empty()) return;
		this->tokenizer_.tokenize(input_information_);
		for(auto&& lexeme : tokenizer_)
		{
			if (lexeme.empty()) continue;
			tt = dictionaries::language_dictionary::match_pattern(lexeme);
			this->ready_tokens_.emplace_back(lexeme, tt);
		}
	}

	language_lexer::tok_info_vector_it language_lexer::front()
	{
		if (this->begin() + cursor_ != this->end())
			return this->ready_tokens_.begin() + cursor_;
		else
			LEXER_EXCEPTION("Lexer reached end of tokens abnormaly");
	}
	void language_lexer::next()
	{
		if(this->begin() + (++cursor_) == this->end())
		{
			LEXER_EXCEPTION("Lexer reached end of tokens abnormaly");
		}
	}
	void language_lexer::flush_cursor() { cursor_ = 0; }
	// Is front token type equal to expected token type
	bool language_lexer::expect(token::TYPE_TOKEN_TYPE ttt)
	{
		return this->front()->type.enum_type == ttt;
	}
	// Is front token type equal to expected token type
	bool language_lexer::not_expect(token::TYPE_TOKEN_TYPE ttt)
	{
		return !expect(ttt);
	}
	bool language_lexer::is_now_break_symbol()
	{
		return expect(token::TYPE_TOKEN_TYPE::END) ||
			expect(token::TYPE_TOKEN_TYPE::RBRACKET) || 
			expect (token::TYPE_TOKEN_TYPE::FRBRACKET) ||
			expect(token::TYPE_TOKEN_TYPE::SEMICOLON);
	}
	bool language_lexer::is_end_symbol()
	{
		return expect(token::TYPE_TOKEN_TYPE::END);
	}
	language_lexer::tok_info_vector_it language_lexer::begin() { return this->ready_tokens_.begin(); }
	language_lexer::tok_info_vector_it language_lexer::end() { return this->ready_tokens_.end(); }
	language_lexer::tok_info_vector_cit language_lexer::begin() const { return this->ready_tokens_.begin(); }
	language_lexer::tok_info_vector_cit language_lexer::end() const { return this->ready_tokens_.end(); }
	language_lexer::tok_info_vector_rit language_lexer::rbegin() { return this->ready_tokens_.rbegin(); }
	language_lexer::tok_info_vector_rit language_lexer::rend() { return this->ready_tokens_.rend(); }
	language_lexer::tok_info_vector_crit language_lexer::rbegin() const { return this->ready_tokens_.rbegin(); }
	language_lexer::tok_info_vector_crit language_lexer::rend() const { return this->ready_tokens_.rend(); }
}
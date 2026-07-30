#include <lexer.h>
#include <tokenizer.h>

#define LEXER	ul::lexer::

LEXER language_lexer::language_lexer(std::string input) :
	input_information_{ std::move(input) }, tokenizer_{ std::move(utils::separator{ " \t\r", "!():;{},.\n" }) }
{
	get_tokens();
	ready_tokens_.emplace_back("", token::token_type{ token::TID::END, "END", "" });
}

LEXER language_lexer::language_lexer() :
	input_information_{ "" }, tokenizer_{ std::move(utils::separator{ " \t\r", "!():;{},.\n" }) }
{}

std::string LEXER language_lexer::get_input_information()
{
	return input_information_;
}

uint64_t LEXER language_lexer::get_current_offset()
{
	uint64_t offset{ 0 };
	for (uint64_t i{ 0 }; i < cursor_; ++i)
		offset += ready_tokens_[i].lexeme.size();
	return offset;
}

void LEXER language_lexer::determine_va_arg()
{
	if (ready_tokens_[ready_tokens_.size() - 1] == ready_tokens_[ready_tokens_.size() - 2] && ready_tokens_[ready_tokens_.size() - 2] == ready_tokens_[ready_tokens_.size() - 3])
	{
		if ((ready_tokens_.end() - 1)->type.enum_type == token::TID::POINT)
		{
			ready_tokens_.erase((ready_tokens_.end() - 3), ready_tokens_.end());
			ready_tokens_.emplace_back("...", dictionaries::language_dictionary::match_pattern("..."));
		}
	}
}

void LEXER language_lexer::determine_not_equal()
{
	if(ready_tokens_[ready_tokens_.size() -1] == token::TID::ASSIGNMENT_OPERATOR && ready_tokens_[ready_tokens_.size() - 2] == token::TID::LOGICAL_NOT_OPERATOR)
	{
		ready_tokens_.erase((ready_tokens_.end() - 2), ready_tokens_.end());
		ready_tokens_.emplace_back("!=", dictionaries::language_dictionary::match_pattern("!="));
	}
}

void LEXER language_lexer::get_tokens()
{
	token::token_type tt{};
	if (input_information_.empty()) return;
	this->tokenizer_.tokenize(input_information_);
	ready_tokens_.clear();
	for (auto&& lexeme : tokenizer_)
	{
		if (lexeme.empty()) continue;
		tt = dictionaries::language_dictionary::match_pattern(lexeme);
		if (tt == token::TID::NEWLINE)
			continue;
		this->ready_tokens_.emplace_back(lexeme, tt);
		determine_va_arg();
		determine_not_equal();
	}
}

LEXER language_lexer::tok_info_vector_it LEXER language_lexer::front()
{
	if (this->begin() + cursor_ != this->end())
		return this->ready_tokens_.begin() + cursor_;
	else
		LEXER_EXCEPTION("Lexer reached end of tokens abnormaly");
}
void LEXER language_lexer::next()
{
	if (this->begin() + (++cursor_) == this->end())
		LEXER_EXCEPTION("Lexer reached end of tokens abnormaly");
}
void LEXER language_lexer::flush_cursor() { cursor_ = 0; }
// Is front token type equal to expected token type
bool LEXER language_lexer::expect(token::TID ttt)
{
	return this->front()->type.enum_type == ttt;
}
// Is front token type equal to expected token type
bool LEXER language_lexer::not_expect(token::TID ttt)
{
	return not expect(ttt);
}
bool LEXER language_lexer::is_now_break_symbol()
{
	return expect(token::TID::END) ||
		expect(token::TID::RBRACKET) ||
		expect(token::TID::FRBRACKET) ||
		expect(token::TID::COMMA) ||
		expect(token::TID::SEMICOLON);
}
bool LEXER language_lexer::is_end_symbol()
{
	return expect(token::TID::END);
}
LEXER language_lexer::tok_info_vector_it LEXER language_lexer::begin() { return this->ready_tokens_.begin(); }
LEXER language_lexer::tok_info_vector_it LEXER language_lexer::end() { return this->ready_tokens_.end(); }
LEXER language_lexer::tok_info_vector_cit LEXER language_lexer::begin() const { return this->ready_tokens_.begin(); }
LEXER language_lexer::tok_info_vector_cit LEXER language_lexer::end() const { return this->ready_tokens_.end(); }
LEXER language_lexer::tok_info_vector_rit LEXER language_lexer::rbegin() { return this->ready_tokens_.rbegin(); }
LEXER language_lexer::tok_info_vector_rit LEXER language_lexer::rend() { return this->ready_tokens_.rend(); }
LEXER language_lexer::tok_info_vector_crit LEXER language_lexer::rbegin() const { return this->ready_tokens_.rbegin(); }
LEXER language_lexer::tok_info_vector_crit LEXER language_lexer::rend() const { return this->ready_tokens_.rend(); }


// Сделать понятие Source Location.
// Добавить понятие колонны и символа строки.
// Каждый токен хранит свой сдвиг в строке и номер строки.
// Токен может обратиться в source location и получить номер строки и колонны.


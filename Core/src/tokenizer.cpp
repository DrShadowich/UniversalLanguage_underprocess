#include <tokenizer.h>
#include <string_view>
namespace ul::utils::classes
{
	tokenizer::tokenizer() :
		sep_{ "" }, tokenized_{}
	{}
	tokenizer::tokenizer(const separator& sep) :
		sep_{ sep }, tokenized_{}
	{}
	tokenizer::tokenizer(separator&& sep) :
		sep_{ std::move(sep) }, tokenized_{}
	{}
	tokenizer::tokenizer(std::string_view data, const separator& separator_) : 
		sep_{ separator_ }, tokenized_{}
	{
		tokenize(data);
	}
	tokenizer::tokenizer(std::string_view data, separator&& separator_) :
		sep_{ std::move(separator_) }, tokenized_{}
	{
		tokenize(data);
	}
	void tokenizer::tokenize(std::string_view data)
	{
		std::string line;
		bool string_literal = false;
		bool commentaries = false;
		bool remains = false;
		char kept_symbol{};
		for(char symbol : data)
		{
			if(symbol == '%' && !commentaries && !string_literal)
			{
				remains = !remains;
			}
			else if(symbol == '#' && !remains)
			{
				commentaries = !commentaries;
			}
			else if(symbol == '\"' && !commentaries)
			{
				string_literal = !string_literal;
			}
			if(commentaries || string_literal || remains)
			{
				line += symbol;
				continue;
			}
			else if ((kept_symbol = sep_.check_symbol(symbol)) != utils::no_char)
			{
				tokenized_.emplace_back(std::move(line));
				if (kept_symbol != utils::without_char)
					tokenized_.emplace_back(1, kept_symbol);
				line = "";
			}
			else line += symbol;
		}
		if(string_literal)
		{
			TOKENIZER_EXCEPTION("No closing \"");
		}
		if(commentaries)
		{
			TOKENIZER_EXCEPTION("No closing #");
		}
		tokenized_.emplace_back(std::move(line));
	}
	tokenizer::tok_vector_it tokenizer::begin() { return tokenized_.begin(); }
	tokenizer::reverse_tok_vector_it tokenizer::rbegin() { return tokenized_.rbegin(); }
	tokenizer::tok_vector_it tokenizer::end() { return tokenized_.end(); }
	tokenizer::reverse_tok_vector_it tokenizer::rend() { return tokenized_.rend(); }
}
#pragma once
#include <separator.h>
#include <commentaries.h>
#include <core_exceptions.h>
#include <vector>
namespace utils::classes
{
	class tokenizer
	{
	private:
		using tok_vector = std::vector<std::string>;
		using tok_vector_it = tok_vector::iterator;
		using reverse_tok_vector_it = tok_vector::reverse_iterator;
		separator sep_;
		tok_vector tokenized_;
	public:
		tokenizer();
		tokenizer(std::string_view data, const separator& sep_);
		tokenizer(std::string_view data, separator&& sep_);
		tokenizer(const separator& sep_);
		tokenizer(separator&& sep_);
		tok_vector_it begin();
		reverse_tok_vector_it rbegin();
		tok_vector_it end();
		reverse_tok_vector_it rend();
		void tokenize(std::string_view data);
	private:
		bool check_char(char symbol);
	};
}
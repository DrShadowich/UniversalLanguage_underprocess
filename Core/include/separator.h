#pragma once
#include <string>
namespace utils
{
	constexpr int without_char = 0;
	constexpr int no_char = -1;
	struct separator
	{
	private:
		// These separators will be kept
		std::string keep_sep_;
		// These separators won't be kept
		std::string out_sep_;
	public:
		template<typename StringTypeF, typename StringTypeS>
		separator(StringTypeF&& out_separator, StringTypeS&& keep_separator) :
			keep_sep_{ std::forward<StringTypeS>(keep_separator) }, out_sep_{ std::forward<StringTypeF>(out_separator) }
		{}
		template<typename StringType>
		separator(StringType&& out_separator) :
			keep_sep_{ "" }, out_sep_{ std::forward<StringType>(out_separator) }
		{}
		separator(const separator& rhs);
		separator(separator&& rhs) noexcept;
		char check_symbol(char symbol);
	};
}
#include <separator.h>
namespace ul::utils
{
	separator::separator(const separator& rhs) :
		out_sep_{ rhs.out_sep_ }, keep_sep_{ rhs.keep_sep_ }
	{}
	separator::separator(separator&& rhs) noexcept :
		out_sep_{ std::move(rhs.out_sep_) }, keep_sep_{ std::move(rhs.keep_sep_) }
	{}
	char separator::check_symbol(char symbol)
	{
		size_t pos = out_sep_.find_first_of(symbol);
		if (pos == std::string::npos)
		{
			pos = keep_sep_.find_first_of(symbol);
			if (pos == std::string::npos) return no_char;
			else return symbol;
		}
		return without_char;
	}
}
#include <token_info.h>
namespace ul::token
{
	token_info& token_info::operator=(const token_info& rhs)
	{
		this->lexeme = rhs.lexeme;
		this->type = rhs.type;
		return *this;
	}
	token_info& token_info::operator=(token_info&& rhs) noexcept
	{
		this->lexeme = std::move(rhs.lexeme);
		this->type = std::move(rhs.type);
		return *this;
	}
}
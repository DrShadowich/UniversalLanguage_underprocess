#include <ulstring.h>
#include <format>
#include <core_exceptions.h>

#define CLASSES ul::utils::classes::


CLASSES stringi8::operator std::string()
{
	return *this;
}


CLASSES stringi8::stringi8() :
	std::string{ "" }
{}

CLASSES stringi8::stringi8(const stringi8& rhs) :
	std::string{ rhs }
{}

CLASSES stringi8::stringi8(stringi8&& rhs) noexcept :
	std::string{ std::move(rhs) }
{}

CLASSES stringi8::stringi8(const std::string& rhs) :
	std::string{ rhs }
{}
CLASSES stringi8::stringi8(std::string&& rhs) noexcept :
	std::string{ std::move(rhs) }
{}


bool CLASSES stringi8::starts_with(char symbol) const
{
	return empty() ? false : *begin() == symbol;
}
bool CLASSES stringi8::starts_with(std::string_view symbols) const
{
	if (symbols.empty())
		return empty();

	// "abcd"
	// "abfewf"
	auto last_pos = find_first_of(*symbols.rbegin());
	auto found = substr(0, last_pos + 1);
	return found == symbols;
}

bool CLASSES stringi8::end_with(char symbol) const
{
	return empty() ? false : *rbegin() == symbol;
}
bool CLASSES stringi8::end_with(std::string_view symbols) const
{
	if (symbols.empty())
		return empty();
	auto last_pos = find_last_of(*symbols.begin());
	auto found = substr(last_pos);
	return found == symbols;
}
bool CLASSES stringi8::covered_with(char symbol) const
{
	return empty() ? false : *begin() == symbol && symbol == *rbegin();
}


CLASSES stringi8& CLASSES stringi8::trim()
{
	if (empty())
		return *this;

	std::string result;
	for(unsigned char i : *this)
	{
		if (not std::isspace(i))
			result += i;
	}
	this->assign(std::move(result));
	return *this;
}
CLASSES stringi8& CLASSES stringi8::trim_assginment()
{
	if (empty())
		return *this;

	std::string result;
	bool is_string = false;
	for (char symbol : *this)
	{
		if (symbol == '"')
			is_string = not is_string;
		if (not is_useless_space(symbol) or is_string)
			result += symbol;
	}
	this->assign(std::move(result));
	return *this;
}
CLASSES stringi8& CLASSES stringi8::remove_all(char symbol)
{
	std::string result;
	for(char s : *this)
	{
		if (s != symbol)
			result += s;
	}
	this->assign(std::move(result));
	return *this;
}
CLASSES stringi8& CLASSES stringi8::cover_this_with(std::string_view symbols)
{
	this->assign(std::format("{}{}{}", symbols, c_str(), symbols));
	return *this;
}

CLASSES stringi8& CLASSES stringi8::uncover_this_with(char symbol)
{
	if (not covered_with(symbol))
		EXCEPTION("{} isn\'t covered by {}", c_str(), symbol);

	this->assign(substr(1, size() - 2));
	return *this;
}
 
bool CLASSES stringi8::has_type() const
{
	return find_last_of('_') != std::string::npos;
}

CLASSES stringi8 CLASSES stringi8::get_string_literal() const
{
	if (empty())
		return *this;
	if (not covered_with('\"'))
		EXCEPTION(std::format("{} is not a string literal", c_str()));

	std::string ret;
	for (size_t i{ 0 }, lsize = size(); i < lsize; ++i)
	{
		if (this->operator[](i) == '\\' && (i + 1) < lsize)
		{
			switch (this->operator[](i + 1))
			{
			case 'n':
				ret += '\n';
				break;
			case 't':
				ret += '\t';
				break;
			case '\\':
				ret += '\\';
				break;
			case '\"':
				ret += '"';
				break;
			case '\'':
				ret += '\'';
				break;
			default:
				EXCEPTION(std::format("Unknown escape sequence: \\{}", this->operator[](i)));
			}
			++i;
		}
		else
			ret += this->operator[](i);
	}
	return ret.substr(1, ret.size() - 2);
}
CLASSES stringi8& CLASSES stringi8::from_string_literal()
{
	this->assign(get_string_literal());
	return *this;
}

std::filesystem::path CLASSES stringi8::make_path() const
{
	return std::filesystem::path{ this->c_str() };
}


CLASSES stringi8 CLASSES stringi8::get_type_from_name() const
{
	if (empty())
		EXCEPTION("string is empty to get type from name");
	if (not has_type())
		EXCEPTION(std::format("{} hasn\'t any type", c_str()));

	return substr(find_last_of('_') + 1);
}
CLASSES stringi8 CLASSES stringi8::get_name_without_type() const
{
	return substr(0, find_last_of('_'));
}
CLASSES stringi8 CLASSES stringi8::cover_with(std::string_view symbols) const
{
	return std::format("{}{}{}", symbols, c_str(), symbols);
}

CLASSES stringi8& CLASSES stringi8::operator=(stringi8&& rhs) noexcept
{
	this->operator=(static_cast<std::string>(std::move(rhs)));
	return *this;
}
CLASSES stringi8& CLASSES stringi8::operator=(const stringi8& rhs)
{
	this->operator=(static_cast<const std::string&>(rhs));
	return *this;
}

bool CLASSES stringi8::is_useless_space(char symbol)
{
	return symbol == '\t' || symbol == '\r' || symbol == ' ';
}

bool CLASSES stringi8::operator == (const std::string& rhs)
{
	if (rhs.size() != this->size())
		return false;
	return !memcmp(rhs.data(), this->data(), rhs.size());
}
bool CLASSES stringi8::operator !=(const std::string& rhs)
{
	return not (*this == rhs);
}
bool CLASSES stringi8::operator ==(const char* rhs)
{
	if (strlen(rhs) != this->size())
		return false;
	return !memcmp(rhs, this->data(), size());
}
bool CLASSES stringi8::operator !=(const char* rhs)
{
	return not(*this == rhs);
}
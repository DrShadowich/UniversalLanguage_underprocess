#pragma once
#include <string>
#include <format>
#include <filesystem>

namespace ul::utils::classes
{
	class stringi8 final : public std::string
	{
	public:
		using std::string::basic_string;
		stringi8();
		stringi8(const stringi8&);
		stringi8(stringi8&&) noexcept;
		stringi8(const std::string&);
		stringi8(std::string&&) noexcept;
	private:
		size_t get_type_and_name_delimeter() const;
		bool is_useless_space(char symbol);
	public:
		bool starts_with(char symbol) const;
		bool starts_with(std::string_view symbol) const;
		bool end_with(char symbol) const;
		bool end_with(std::string_view symbol) const;
		bool covered_with(char symbol) const;
		bool has_type() const;

		stringi8& trim();
		stringi8& trim_assginment();
		stringi8& remove_all(char symbol);
		stringi8& cover_this_with(std::string_view symbols);
		stringi8& uncover_this_with(char symbol);

		stringi8 get_type_from_name() const;
		stringi8 get_name_without_type() const;
		stringi8 cover_with(std::string_view symbols) const;
	
		stringi8 get_string_literal() const;
		stringi8& from_string_literal();

		operator std::string();

		std::filesystem::path make_path() const;

		using std::string::operator=;
		using std::string::operator+=;
		using std::string::operator[];
		stringi8& operator=(stringi8&&) noexcept;
		stringi8& operator=(const stringi8&);
		bool operator == (const std::string&);
		bool operator !=(const std::string&);
		bool operator ==(const char*);
		bool operator !=(const char*);
	};
};


template <>
struct std::formatter<ul::utils::classes::stringi8> {
	// Correct {}
	constexpr auto parse(std::format_parse_context& ctx) {
		auto it = ctx.begin();
		if (it != ctx.end() && *it != '}') {
			throw std::format_error("String must be followed by '}'");
		}
		return ++it;
	}

	auto format(const ul::utils::classes::stringi8& p, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}", p.c_str());
	}
};


template <>
struct std::hash<ul::utils::classes::stringi8> {
	size_t operator()(const ul::utils::classes::stringi8& p) const {
		size_t res = std::hash<std::string>()(static_cast<const std::string&>(p));
		return res;
	}
};
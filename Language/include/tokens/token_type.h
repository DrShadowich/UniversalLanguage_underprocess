#pragma once
#define enum_to_string(x)	#x
#include <memory>
#include <regex>
namespace ul::token
{
	enum class TID : uint16_t
	{
		NO_TOKEN = 0,
		// \n
		NEWLINE,
		// [a-zA-Z0-9_]+_fn
		FUNCTION_IDENTIFIER,
		// [a-zA-Z0-9_]+_([a-zA-Z0-9]+)
		VARIABLE_IDENTIFIER,
		// [a-zA-Z]
		CLASS_TYPE,
		// _[a-zA-Z]
		UNNAMED_CLASS_TYPE,
		// ...
		TRIPLE_POINT,
		// (
		LBRACKET,
		// )
		RBRACKET,
		// [
		SLBRACKET,
		// ]
		SRBRACKET,
		// +
		PLUS_OPERATOR,
		// ++
		INCREMENT_OPERATOR,
		// --
		DECREMENT_OPERATOR,
		// -
		MINUS_OPERATOR,
		// *
		STAR_OPERATOR,
		// /
		SLASH_OPERATOR,
		// =
		ASSIGNMENT_OPERATOR,
		// +=
		PLUS_ASSIGNMENT_OPERATOR,
		// -=
		MINUS_ASSIGNMENT_OPERATOR,
		// /=
		SLASH_ASSIGNMENT_OPERATOR,
		// *=
		STAR_ASSIGNMENT_OPERATOR,
		// ;
		SEMICOLON,
		// {
		FLBRACKET,
		// }
		FRBRACKET,
		// 0-9+
		NUMBER_LITERAL,
		// "[^"\\]*(\\.[^"\\]*)*"
		STRING_LITERAL,
		// 0-9+.0-9*
		FLOAT_LITERAL,
		// if
		KEYWORD_IF,
		// else
		KEYWORD_ELSE,
		// while
		KEYWORD_WHILE,
		// for
		KEYWORD_FOR,
		// return
		KEYWORD_RETURN,
		// struct
		KEYWORD_STRUCT,
		// enum
		KEYWORD_ENUM,
		// else if (elif)
		KEYWORD_ELIF,
		// nameof
		KEYWORD_NAMEOF,
		// switch
		KEYWORD_SWITCH,
		// case
		KEYWORD_CASE,
		// default
		KEYWORD_DEFAULT,
		// break
		KEYWORD_BREAK,
		// continue
		KEYWORD_CONTINUE,
		// loop
		KEYWORD_LOOP,
		// class
		KEYWORD_CLASS,
		// immut
		KEYWORD_IMMUT,
		// insert
		KEYWORD_INSERT,
		// ref
		KEYWORD_REF,
		// const
		KEYWORD_CONST,
		// new
		KEYWORD_NEW,
		// global
		KEYWORD_GLOBAL,
		// constexpr
		KEYWORD_CONSTEXPR,
		// out
		KEYWORD_MUT_OUTPUT,
		// extern
		KEYWORD_EXTERN,
		// || (or)
		LOGICAL_OR_OPERATOR,
		// && (and)
		LOGICAL_AND_OPERATOR,
		// ! (not)
		LOGICAL_NOT_OPERATOR,
		// == (eq)
		LOGICAL_EQUAL_OPERATOR,
		// != (neq)
		LOGICAL_NOT_EQUAL_OPERATOR,
		// < (ls)
		LOGICAL_LESS_OPERATOR,
		// > (gt)
		LOGICAL_GREATER_OPERATOR,
		// <= (lseq)
		LOGICAL_LESS_OR_EQUAL_OPERATOR,
		// >= (gteq)
		LOGICAL_GREATER_OR_EQUAL_OPERATOR,
		// .
		POINT,
		// :
		COLON,
		// #[^#\\]*(\\.[^#\\]*)*#
		SIMPLE_COMMENT,
		// $
		FORMAT_SYMBOL,
		// ,
		COMMA,
		// true
		TRUE,
		// false
		FALSE,
		// #py
		PYTHON_MARKER,
		// #cpp
		CPP_MARKER,
		// #c
		C_MARKER,
		// #lua
		LUA_MARKER,
		// #end
		END_MARKER,
		// #config
		CONFIG_MARKER,
		// %(\w*)([\w\S\s]*?)%(\w*)
		MARKER_EXPRESSION,
		// %(\w*)([\w\S\s]*?)%(end)
		MARKER_EXPRESSION_WITH_END,
		// ->
		POINTER,
		END,
	};


	struct token_type
	{
		std::unique_ptr<std::string> token_pattern{};
		std::unique_ptr<std::regex> regex_token_pattern{};
		const char* enum_type_string{};
		TID enum_type{};
		// What kind of lexeme should it be to become this token type
		bool is_pattern_regex = false;
		token_type();
		token_type(const token_type& rhs);
		token_type(token_type&& rhs) noexcept;
		explicit token_type(TID token_type_, const char* type_str, std::string pattern);
		explicit token_type(TID token_type_, const char* type_str, std::regex pattern);
		token_type& operator=(const token_type& rhs);
		token_type& operator=(token_type&& rhs) noexcept;
		operator token::TID() const { return enum_type; }
		bool operator==(TID rhs) const { return enum_type == rhs; }
		bool operator!=(TID rhs) const { return not (enum_type == rhs); }
	};
}
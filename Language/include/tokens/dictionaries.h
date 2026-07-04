#pragma once
#include <unordered_map>
#include <vector>
#include <llvm_libs.h>
#include "token_info.h"
namespace ul::dictionaries
{
	struct language_dictionary
	{
		static inline const std::vector<token::token_type> raw =
		{
			// NEWLINE: Обычно обрабатывается отдельно в лексере по символу '\n', но если нужен regex:
			token::token_type{ token::TYPE_TOKEN_TYPE::NEWLINE, "NEWLINE", "\n" },

			// --- Логические операторы и сравнения (Regex с границами или просто строки) ---
			// Так как это отдельные токены, можно использовать строки, но для надежности в потоке лучше regex с проверкой контекста.
			// Однако, следуя вашему примеру "просто символ", используем строки, если это одиночные токены.
			// Но ||, &&, !=, <=, >= - это последовательности символов.

			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_OR_OPERATOR, "LOGICAL_OR_OPERATOR", "||" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_OR_OPERATOR, "LOGICAL_OR_OPERATOR", std::regex("\\bor\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_AND_OPERATOR, "LOGICAL_AND_OPERATOR", "&&" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_AND_OPERATOR, "LOGICAL_AND_OPERATOR", std::regex("\\band\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_NOT_OPERATOR, "LOGICAL_NOT_OPERATOR", "!" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_NOT_OPERATOR, "LOGICAL_NOT_OPERATOR", std::regex("\\bnot\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_EQUAL_OPERATOR, "LOGICAL_EQUAL_OPERATOR", "==" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_EQUAL_OPERATOR, "LOGICAL_EQUAL_OPERATOR", std::regex("\\beq\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_NOT_EQUAL_OPERATOR, "LOGICAL_NOT_EQUAL_OPERATOR", "!=" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_NOT_EQUAL_OPERATOR, "LOGICAL_NOT_EQUAL_OPERATOR", std::regex("\\bneq\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_LESS_OPERATOR, "LOGICAL_LESS_OPERATOR", "<" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_LESS_OPERATOR, "LOGICAL_LESS_OPERATOR", std::regex("\\bls\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_GREATER_OPERATOR, "LOGICAL_GREATER_OPERATOR", ">" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_GREATER_OPERATOR, "LOGICAL_GREATER_OPERATOR", std::regex("\\bgt\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_LESS_OR_EQUAL_OPERATOR, "LOGICAL_LESS_OR_EQUAL_OPERATOR", "<=" },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_LESS_OR_EQUAL_OPERATOR, "LOGICAL_LESS_OR_EQUAL_OPERATOR", std::regex("\\blseq\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_GREATER_OR_EQUAL_OPERATOR, "LOGICAL_GREATER_OR_EQUAL_OPERATOR", ">=" },

			// --- Операторы и скобки (Просто символы) ---
			token::token_type{ token::TYPE_TOKEN_TYPE::LBRACKET, "LBRACKET", "(" },
			token::token_type{ token::TYPE_TOKEN_TYPE::RBRACKET, "RBRACKET", ")" },
			token::token_type{ token::TYPE_TOKEN_TYPE::PLUS_OPERATOR, "PLUS_OPERATOR", "+" },
			token::token_type{ token::TYPE_TOKEN_TYPE::MINUS_OPERATOR, "MINUS_OPERATOR", "-" },
			token::token_type{ token::TYPE_TOKEN_TYPE::STAR_OPERATOR, "STAR_OPERATOR", "*" },
			token::token_type{ token::TYPE_TOKEN_TYPE::SLASH_OPERATOR, "SLASH_OPERATOR", "/" },
			token::token_type{ token::TYPE_TOKEN_TYPE::ASSIGNMENT_OPERATOR, "ASSIGNMENT_OPERATOR", "=" },
			token::token_type{ token::TYPE_TOKEN_TYPE::SEMICOLON, "SEMICOLON", ";" },
			token::token_type{ token::TYPE_TOKEN_TYPE::FLBRACKET, "FLBRACKET", "{" },
			token::token_type{ token::TYPE_TOKEN_TYPE::FRBRACKET, "FRBRACKET", "}" },
			token::token_type{ token::TYPE_TOKEN_TYPE::POINT, "POINT", "." },
			token::token_type{ token::TYPE_TOKEN_TYPE::COLON, "COLON", ":" },
			token::token_type{ token::TYPE_TOKEN_TYPE::COMMA, "COMMA", "," },
			token::token_type{ token::TYPE_TOKEN_TYPE::FORMAT_SYMBOL, "FORMAT_SYMBOL", "$" },

			// --- Литералы (Regex) ---
			// 0-9+
			token::token_type{ token::TYPE_TOKEN_TYPE::NUMBER_LITERAL, "NUMBER_LITERAL", std::regex("[0-9]+") },
			// "[^"\\]*(\\.[^"\\]*)*" -> Адаптировано для C++ string literal
			token::token_type{ token::TYPE_TOKEN_TYPE::STRING_LITERAL, "STRING_LITERAL", std::regex("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"") },
			// 0-9+.0-9*
			token::token_type{ token::TYPE_TOKEN_TYPE::FLOAT_LITERAL, "FLOAT_LITERAL", std::regex("[0-9]+\\.[0-9]*") },


			token::token_type{ token::TYPE_TOKEN_TYPE::LOGICAL_GREATER_OR_EQUAL_OPERATOR, "LOGICAL_GREATER_OR_EQUAL_OPERATOR", std::regex("\\bgteq\\b") },

			// --- Ключевые слова (Regex с \b для границ слова) ---
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_IF, "KEYWORD_IF", std::regex("\\bif\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_ELSE, "KEYWORD_ELSE", std::regex("\\belse\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_WHILE, "KEYWORD_WHILE", std::regex("\\bwhile\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_FOR, "KEYWORD_FOR", std::regex("\\bfor\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_RETURN, "KEYWORD_RETURN", std::regex("\\breturn\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_STRUCT, "KEYWORD_STRUCT", std::regex("\\bstruct\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_ENUM, "KEYWORD_ENUM", std::regex("\\benum\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_ELIF, "KEYWORD_ELIF", std::regex("\\belif\\b") }, // Ваш кастомный токен
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_SWITCH, "KEYWORD_SWITCH", std::regex("\\bswitch\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_CASE, "KEYWORD_CASE", std::regex("\\bcase\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_DEFAULT, "KEYWORD_DEFAULT", std::regex("\\bdefault\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_BREAK, "KEYWORD_BREAK", std::regex("\\bbreak\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_CONTINUE, "KEYWORD_CONTINUE", std::regex("\\bcontinue\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_LOOP, "KEYWORD_LOOP", std::regex("\\bloop\\b") }, // Ваш кастомный
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_CLASS, "KEYWORD_CLASS", std::regex("\\bclass\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_IMMUT, "KEYWORD_IMMUT", std::regex("\\bimmut\\b") }, // Ваш кастомный
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_CONST, "KEYWORD_CONST", std::regex("\\bconst\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_GLOBAL, "KEYWORD_GLOBAL", std::regex("\\bglobal\\b") }, // Ваш кастомный
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_CONSTEXPR, "KEYWORD_CONSTEXPR", std::regex("\\bconstexpr\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::KEYWORD_MUT_OUTPUT, "KEYWORD_MUT_OUTPUT", std::regex("\\bout\\b") }, // Соответствует комментарию "// out"

			// --- Булевы значения ---
			token::token_type{ token::TYPE_TOKEN_TYPE::TRUE, "TRUE", std::regex("\\btrue\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::FALSE, "FALSE", std::regex("\\bfalse\\b") },

			// --- Комментарии (Regex) ---
			// #[^#\\]*(\\.[^#\\]*)*# -> Адаптировано. В C++ regex нужно экранировать # если нужно, но # не спецсимвол.
			// Исходный паттерн: #[^#\]*(\\.[^#\]*)#
			token::token_type{ token::TYPE_TOKEN_TYPE::SIMPLE_COMMENT, "SIMPLE_COMMENT", std::regex("#[^#\\\\]*(?:\\\\.[^#\\\\]*)*#") },

			// --- Маркеры (Regex с границами, чтобы не совпало внутри слова) ---
			token::token_type{ token::TYPE_TOKEN_TYPE::MARKER_EXPRESSION, "MARKER_EXPRESSION", std::regex(R"(%((py|config|cpp|c|lua))([\w\s\S]*?)%(end))") },
			token::token_type{ token::TYPE_TOKEN_TYPE::PYTHON_MARKER, "PYTHON_MARKER", std::regex("\\b%py\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::CPP_MARKER, "CPP_MARKER", std::regex("\\b%cpp\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::C_MARKER, "C_MARKER", std::regex("\\b%c\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::LUA_MARKER, "LUA_MARKER", std::regex("\\b%lua\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::END_MARKER, "END_MARKER", std::regex("\\b%end\\b") },
			token::token_type{ token::TYPE_TOKEN_TYPE::CONFIG_MARKER, "CONFIG_MARKER", std::regex("\\b%config\\b") },
			// --- Идентификаторы (Regex, так как нужны диапазоны) ---
			// [a-zA-Z0-9_]+_fn
			token::token_type{ token::TYPE_TOKEN_TYPE::FUNCTION_IDENTIFIER, "FUNCTION_IDENTIFIER", std::regex("[a-zA-Z0-9_]+_fn") },
			// [a-zA-Z0-9_]+_([a-zA-Z0-9]+)
			token::token_type{ token::TYPE_TOKEN_TYPE::VARIABLE_IDENTIFIER, "VARIABLE_IDENTIFIER", std::regex("[a-zA-Z0-9_]+_[a-zA-Z0-9]+") },
			// [a-zA-Z] (Одиночная буква для типа класса? Или начало?)
			// Примечание: regex "[a-zA-Z]" совпадет только с одной буквой. Если нужно слово, используйте "[a-zA-Z][a-zA-Z0-9]*"
			token::token_type{ token::TYPE_TOKEN_TYPE::CLASS_TYPE, "CLASS_TYPE", std::regex("[a-zA-Z][a-zA-Z0-9_]*") },
		};
		// Check any lexeme with all patterns
		template<typename StringType>
		static token::token_type match_pattern(StringType&& lexeme)
		{
			bool state{ false };
			for (auto&& tt : raw)
			{
				state = tt.is_pattern_regex ? std::regex_match(lexeme, *tt.regex_token_pattern) : !tt.token_pattern->compare(lexeme);
				if (state)
				{
					return tt;
				}
			}
			return token::token_type{ token::TYPE_TOKEN_TYPE::NO_TOKEN, "NO_TOKEN", "" };
		}
	};

	using type_factory_ptr = llvm::Type* (*)(llvm::LLVMContext&);
	static inline const std::unordered_map<std::string, type_factory_ptr> ul_llvm_type_table =
	{
		{ "int16", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getInt16Ty(ctx); } },
		{ "int32", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getInt32Ty(ctx); } },
		{ "int64", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getInt64Ty(ctx); } },
		{ "float", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getFloatTy(ctx); } },
		{ "double", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getBFloatTy(ctx); } },
		{ "char", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getInt1Ty(ctx); } },
		{ "bool", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getInt1Ty(ctx); } },
	};

	using constant_int_factory_ptr = llvm::Value* (*)(llvm::IRBuilder<>& builder, uint64_t value);
	static inline const std::unordered_map<std::string, constant_int_factory_ptr> ul_llvm_constant_int_table =
	{
		{ "int16", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt16(value); } },
		{ "int32", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt32(value); } },
		{ "int64", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt64(value); } },
		{ "char", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt1(value); } },
		{ "bool", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt1(value); } }
	};
}
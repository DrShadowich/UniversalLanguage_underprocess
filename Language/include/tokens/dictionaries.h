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
			token::token_type{ token::TID::NEWLINE, "NEWLINE", "\n" },
			// --- Логические операторы и сравнения (Regex с границами или просто строки) ---
			// Так как это отдельные токены, можно использовать строки, но для надежности в потоке лучше regex с проверкой контекста.
			// Однако, следуя вашему примеру "просто символ", используем строки, если это одиночные токены.
			// Но ||, &&, !=, <=, >= - это последовательности символов.
			token::token_type{ token::TID::LOGICAL_OR_OPERATOR, "LOGICAL_OR_OPERATOR", "||" },
			token::token_type{ token::TID::LOGICAL_OR_OPERATOR, "LOGICAL_OR_OPERATOR", std::regex("\\bor\\b") },
			token::token_type{ token::TID::LOGICAL_AND_OPERATOR, "LOGICAL_AND_OPERATOR", "&&" },
			token::token_type{ token::TID::LOGICAL_AND_OPERATOR, "LOGICAL_AND_OPERATOR", std::regex("\\band\\b") },
			token::token_type{ token::TID::LOGICAL_NOT_EQUAL_OPERATOR, "LOGICAL_NOT_EQUAL_OPERATOR", "!=" },
			token::token_type{ token::TID::LOGICAL_NOT_OPERATOR, "LOGICAL_NOT_OPERATOR", "!" },
			token::token_type{ token::TID::LOGICAL_NOT_OPERATOR, "LOGICAL_NOT_OPERATOR", std::regex("\\bnot\\b") },
			token::token_type{ token::TID::LOGICAL_EQUAL_OPERATOR, "LOGICAL_EQUAL_OPERATOR", "==" },
			token::token_type{ token::TID::LOGICAL_EQUAL_OPERATOR, "LOGICAL_EQUAL_OPERATOR", std::regex("\\beq\\b") },
			token::token_type{ token::TID::LOGICAL_NOT_EQUAL_OPERATOR, "LOGICAL_NOT_EQUAL_OPERATOR", std::regex("\\bneq\\b") },
			token::token_type{ token::TID::LOGICAL_LESS_OPERATOR, "LOGICAL_LESS_OPERATOR", "<" },
			token::token_type{ token::TID::LOGICAL_LESS_OPERATOR, "LOGICAL_LESS_OPERATOR", std::regex("\\bls\\b") },
			token::token_type{ token::TID::LOGICAL_GREATER_OPERATOR, "LOGICAL_GREATER_OPERATOR", ">" },
			token::token_type{ token::TID::LOGICAL_GREATER_OPERATOR, "LOGICAL_GREATER_OPERATOR", std::regex("\\bgt\\b") },
			token::token_type{ token::TID::LOGICAL_LESS_OR_EQUAL_OPERATOR, "LOGICAL_LESS_OR_EQUAL_OPERATOR", "<=" },
			token::token_type{ token::TID::LOGICAL_LESS_OR_EQUAL_OPERATOR, "LOGICAL_LESS_OR_EQUAL_OPERATOR", std::regex("\\blseq\\b") },
			token::token_type{ token::TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR, "LOGICAL_GREATER_OR_EQUAL_OPERATOR", ">=" },
			token::token_type{ token::TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR, "LOGICAL_GREATER_OR_EQUAL_OPERATOR", std::regex("\\bgteq\\b") },

			// --- Операторы и скобки (Просто символы) ---
			token::token_type{ token::TID::POINTER, "POINTER", std::regex("\\b->\\b") },
			token::token_type{ token::TID::TRIPLE_POINT, "TRIPLE_POINT", "..." },
			token::token_type{ token::TID::LBRACKET, "LBRACKET", "(" },
			token::token_type{ token::TID::RBRACKET, "RBRACKET", ")" },
			token::token_type{ token::TID::PLUS_OPERATOR, "PLUS_OPERATOR", "+" },
			token::token_type{ token::TID::MINUS_OPERATOR, "MINUS_OPERATOR", "-" },
			token::token_type{ token::TID::STAR_OPERATOR, "STAR_OPERATOR", "*" },
			token::token_type{ token::TID::SLASH_OPERATOR, "SLASH_OPERATOR", "/" },
			token::token_type{ token::TID::ASSIGNMENT_OPERATOR, "ASSIGNMENT_OPERATOR", "=" },
			token::token_type{ token::TID::SEMICOLON, "SEMICOLON", ";" },
			token::token_type{ token::TID::FLBRACKET, "FLBRACKET", "{" },
			token::token_type{ token::TID::FRBRACKET, "FRBRACKET", "}" },
			token::token_type{ token::TID::POINT, "POINT", "." },
			token::token_type{ token::TID::COLON, "COLON", ":" },
			token::token_type{ token::TID::COMMA, "COMMA", "," },
			token::token_type{ token::TID::FORMAT_SYMBOL, "FORMAT_SYMBOL", "$" },
			// --- Литералы (Regex) ---
			// 0-9+
			token::token_type{ token::TID::NUMBER_LITERAL, "NUMBER_LITERAL", std::regex("[0-9]+") },
			// "[^"\\]*(\\.[^"\\]*)*" -> Адаптировано для C++ string literal
			token::token_type{ token::TID::STRING_LITERAL, "STRING_LITERAL", std::regex("\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"") },
			// 0-9+.0-9*
			token::token_type{ token::TID::FLOAT_LITERAL, "FLOAT_LITERAL", std::regex("[0-9]+\\.[0-9]*") },



			// --- Ключевые слова (Regex с \b для границ слова) ---
			token::token_type{ token::TID::KEYWORD_USING, "KEYWORD_USING", std::regex("\\busing\\b") },
			token::token_type{ token::TID::KEYWORD_EXTERN, "KEYWORD_EXTERN", std::regex("\\bextern\\b") },
			token::token_type{ token::TID::KEYWORD_IF, "KEYWORD_IF", std::regex("\\bif\\b") },
			token::token_type{ token::TID::KEYWORD_ELSE, "KEYWORD_ELSE", std::regex("\\belse\\b") },
			token::token_type{ token::TID::KEYWORD_WHILE, "KEYWORD_WHILE", std::regex("\\bwhile\\b") },
			token::token_type{ token::TID::KEYWORD_FOR, "KEYWORD_FOR", std::regex("\\bfor\\b") },
			token::token_type{ token::TID::KEYWORD_RETURN, "KEYWORD_RETURN", std::regex("\\breturn\\b") },
			token::token_type{ token::TID::KEYWORD_STRUCT, "KEYWORD_STRUCT", std::regex("\\bstruct\\b") },
			token::token_type{ token::TID::KEYWORD_ENUM, "KEYWORD_ENUM", std::regex("\\benum\\b") },
			token::token_type{ token::TID::KEYWORD_ELIF, "KEYWORD_ELIF", std::regex("\\belif\\b") }, // Ваш кастомный токен
			token::token_type{ token::TID::KEYWORD_SWITCH, "KEYWORD_SWITCH", std::regex("\\bswitch\\b") },
			token::token_type{ token::TID::KEYWORD_CASE, "KEYWORD_CASE", std::regex("\\bcase\\b") },
			token::token_type{ token::TID::KEYWORD_DEFAULT, "KEYWORD_DEFAULT", std::regex("\\bdefault\\b") },
			token::token_type{ token::TID::KEYWORD_BREAK, "KEYWORD_BREAK", std::regex("\\bbreak\\b") },
			token::token_type{ token::TID::KEYWORD_CONTINUE, "KEYWORD_CONTINUE", std::regex("\\bcontinue\\b") },
			token::token_type{ token::TID::KEYWORD_LOOP, "KEYWORD_LOOP", std::regex("\\bloop\\b") }, // Ваш кастомный
			token::token_type{ token::TID::KEYWORD_CLASS, "KEYWORD_CLASS", std::regex("\\bclass\\b") },
			token::token_type{ token::TID::KEYWORD_IMMUT, "KEYWORD_IMMUT", std::regex("\\bimmut\\b") }, // Ваш кастомный
			token::token_type{ token::TID::KEYWORD_CONST, "KEYWORD_CONST", std::regex("\\bconst\\b") },
			token::token_type{ token::TID::KEYWORD_GLOBAL, "KEYWORD_GLOBAL", std::regex("\\bglobal\\b") }, // Ваш кастомный
			token::token_type{ token::TID::KEYWORD_CONSTEXPR, "KEYWORD_CONSTEXPR", std::regex("\\bconstexpr\\b") },
			token::token_type{ token::TID::KEYWORD_REF, "KEYWORD_REF", std::regex("\\bref\\b") },

			// --- Булевы значения ---
			token::token_type{ token::TID::TRUE, "TRUE", std::regex("\\btrue\\b") },
			token::token_type{ token::TID::FALSE, "FALSE", std::regex("\\bfalse\\b") },

			// --- Комментарии (Regex) ---
			// #[^#\\]*(\\.[^#\\]*)*# -> Адаптировано. В C++ regex нужно экранировать # если нужно, но # не спецсимвол.
			// Исходный паттерн: #[^#\]*(\\.[^#\]*)#
			token::token_type{ token::TID::SIMPLE_COMMENT, "SIMPLE_COMMENT", std::regex("#[^#\\\\]*(?:\\\\.[^#\\\\]*)*#") },

			// --- Маркеры (Regex с границами, чтобы не совпало внутри слова) ---
			token::token_type{ token::TID::MARKER_EXPRESSION, "MARKER_EXPRESSION", std::regex(R"(%(\w*)([\w\S\s]*?)%(\w*))") },
			token::token_type{ token::TID::MARKER_EXPRESSION_WITH_END, "MARKER_EXPRESSION_WITH_END", std::regex(R"(%(\w*)([\w\S\s]*?)%(end))") },
			// --- Идентификаторы (Regex, так как нужны диапазоны) ---
			// _[a - zA - Z]
			token::token_type{ token::TID::UNNAMED_CLASS_TYPE, "UNNAMED_CLASS_TYPE", std::regex("_[a-zA-Z]+") },
			// [a-zA-Z0-9_]+_fn
			token::token_type{ token::TID::FUNCTION_IDENTIFIER, "FUNCTION_IDENTIFIER", std::regex("[a-zA-Z0-9_]+_fn") },
			// [a-zA-Z0-9_]+_([a-zA-Z0-9]+)
			token::token_type{ token::TID::VARIABLE_IDENTIFIER, "VARIABLE_IDENTIFIER", std::regex("[a-zA-Z0-9_]+_[a-zA-Z0-9]+") },
			// [a-zA-Z] (Одиночная буква для типа класса? Или начало?)
			// Примечание: regex "[a-zA-Z]" совпадет только с одной буквой. Если нужно слово, используйте "[a-zA-Z][a-zA-Z0-9]*"
			token::token_type{ token::TID::CLASS_TYPE, "CLASS_TYPE", std::regex("[a-zA-Z][a-zA-Z0-9_]+") },
		};
		// Check any lexeme with all patterns
		static token::token_type match_pattern(const std::string& lexeme);
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
		{ "str", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::PointerType::getInt8Ty(ctx)->getPointerTo(); } },
		{ "ptr", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::PointerType::getInt8Ty(ctx)->getPointerTo(); } },
		{ "", [](llvm::LLVMContext& ctx) -> llvm::Type* { return llvm::Type::getVoidTy(ctx); } }
	};

	using constant_int_factory_ptr = llvm::Value* (*)(llvm::IRBuilder<>& builder, uint64_t value);
	static inline const std::unordered_map<std::string, constant_int_factory_ptr> ul_string_llvm_constant_int_table =
	{
		{ "int16", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt16(value); } },
		{ "int32", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt32(value); } },
		{ "int64", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt64(value); } },
		{ "char", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt8(value); } },
		{ "bool", [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt1(value); } }
	};

	using constant_int_factory_ptr = llvm::Value* (*)(llvm::IRBuilder<>& builder, uint64_t value);
	static inline const std::unordered_map<uint32_t, constant_int_factory_ptr> ul_uint_llvm_constant_int_table =
	{
		{ 16, [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt16(value); } },
		{ 32, [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt32(value); } },
		{ 64, [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt64(value); } },
		{ 8, [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt8(value); } },
		{ 1, [](llvm::IRBuilder<>& builder, uint64_t value) -> llvm::Value* { return builder.getInt1(value); } }
	};

	static inline const std::unordered_map<std::string, uint32_t> ul_llvm_type_to_int_table =
	{
		{ "bool", 1 },
		{ "int1", 1 },
		{ "int8", 8 },
		{ "int16", 16 },
		{ "int32", 32 },
		{ "int64", 64 },
	};

	static inline const std::unordered_map<std::string, uint32_t> ul_llvm_alignment_table =
	{
		{ "int16", 2 },
		{ "int32", 4 },
		{ "int64", 8 },
		{ "char", 1 },
		{ "bool", 1 },
		{ "str", 8 },
		{ "float", 4 },
		{ "double", 8 },
		{ "ptr", 8 },
	};
}
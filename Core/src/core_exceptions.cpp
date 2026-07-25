#include <core_exceptions.h>

namespace ul::ex
{

	parser_exception::parser_exception(std::string msg, std::string file_name, std::pair<uint64_t, uint64_t> col_and_row) :
		file_code_col_{ col_and_row.first }, file_code_line_{ col_and_row.second }
	{
		file_name_ = file_name.c_str();
		message_ = std::move(msg);
		native_string();
	}

	void parser_exception::native_string()
	{
		message_ = std::format("File: {} in row: {}, col: {}\nSyntax error: {}", file_name_, file_code_line_, file_code_col_, message_);
	}

	void code_generator_exception::native_string()
	{
		message_ = std::format("Code generation error in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
	}

	void lexer_exception::native_string()
	{
		message_ = std::format("Error occured while tokenizing in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
	}

	void tokenizer_exception::native_string()
	{
		message_ = std::format("While splitting input text got exception in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
	}

	void visitor_exception::native_string()
	{
		message_ = std::format("While walking the AST, found error in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
	}

	void name_table_exception::native_string()
	{
		message_ = std::format("Table of names got error in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
	}
}
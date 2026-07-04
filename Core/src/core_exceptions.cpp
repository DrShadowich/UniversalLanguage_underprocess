#include <core_exceptions.h>

namespace ul::ex
{
	void parser_exception::native_string()
	{
		message_ = std::format("Parser error in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
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
}
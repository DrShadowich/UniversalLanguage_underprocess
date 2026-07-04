#pragma once
#include <stdexcept>
#include <format>
/*
* Universal Language lexer, parser, code gen exceptions
*/

#define ULEX	ul::ex::


#define EXCEPTION(message)					(throw ULEX exception{ (message), __LINE__, __FILE__, __FUNCTION__ })
#define LEXER_EXCEPTION(message)			(throw ULEX lexer_exception{ (message), __LINE__, __FILE__, __FUNCTION__ })
#define PARSER_EXCEPTION(message)			(throw ULEX parser_exception{ (message), __LINE__, __FILE__, __FUNCTION__ })
#define CODE_GENERATOR_EXCEPTION(message)	(throw ULEX code_generator_exception{ (message), __LINE__, __FILE__, __FUNCTION__ })
#define TOKENIZER_EXCEPTION(message)		(throw ULEX tokenizer_exception{ (message), __LINE__, __FILE__, __FUNCTION__ })
#define VISITOR_EXCEPTION(message)			(throw ULEX visitor_exception{ (message), __LINE__, __FILE__, __FUNCTION__ })


namespace ul::ex
{
	class exception : public std::exception
	{
	protected:
		std::string message_{};
		size_t code_line_{};
		const char* file_name_{};
		const char* function_name_{};
	public:
		exception() {};
		template<typename StringType>
		explicit exception(StringType&& msg, size_t CODE_LINE, const char* FILE_NAME, const char* FUNCTION_NAME) :
			code_line_{ CODE_LINE }, file_name_{ FILE_NAME }, function_name_{ FUNCTION_NAME }, message_{ std::forward<StringType>(msg) }
		{
			native_string();
		}
	protected:
		virtual void native_string()
		{
			message_ = std::format("Error occured in {} -> {} on line {} with message: {}.\n", file_name_, function_name_, code_line_, message_);
		}
	public:
		const char* what() const override
		{
			return message_.c_str();
		}
	};

	class parser_exception final : public ULEX exception
	{
	public:
		using ULEX exception::exception;
	protected:
		virtual void native_string() override;
	};

	class code_generator_exception final : public ULEX exception
	{
	public:
		using ULEX exception::exception;
	protected:
		virtual void native_string() override;
	};

	class lexer_exception final : public ULEX exception
	{
	public:
		using ULEX exception::exception;
	protected:
		virtual void native_string() override;
	};

	class tokenizer_exception final : public ULEX exception
	{
	public:
		using ULEX exception::exception;
	protected:
		virtual void native_string() override;
	};

	class visitor_exception final : public ULEX exception
	{
	public:
		using ULEX exception::exception;
	protected:
		virtual void native_string() override;
	};
}
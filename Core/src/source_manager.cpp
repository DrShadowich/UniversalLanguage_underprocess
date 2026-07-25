#include <source_manager.h>
#include <core_exceptions.h>

SOURCE source_manager::source_manager() :
	information_copy{ "" }
{}

SOURCE source_manager::source_manager(std::string copying_data) :
	information_copy{ std::move(copying_data) }
{}

std::pair<uint64_t, uint64_t> SOURCE source_manager::get_line_and_column(uint64_t offset)
{
	if (information_copy.empty())
		LEXER_EXCEPTION("Expected text :/");
	else if (information_copy.size() < offset)
		LEXER_EXCEPTION("Offset is greater then input text");
	
	uint64_t col{ 0 };
	uint64_t row{ 0 };

	for (uint64_t i{ 0 }; i < offset; ++i)
	{
		if (information_copy[i] == '\n')
		{
			++col;
			row = 0;
		}
		else ++row;
	}
	return { col, row };
}
#include <iostream>
#include <fstream>
#include <lexer.h>
#include <statement_info.h>
#include <parser.h>
#include <dictionaries.h>
#include <visitors.h>
int main()
{
	std::ifstream file{ "main.ul" };
	std::string input{ std::istreambuf_iterator<char>{ file.rdbuf() }, std::istreambuf_iterator<char>{ } };
	file.close();
	ul::lexer::language_lexer l{ input };
	ul::parser::language_parser p{ l };
	auto&& stmts = p.parse_program();
	stmt_output_visitor sv;
	for(auto&& stmt : stmts->statements)
	{
		std::cout << sv.visit(*stmt) << std::endl;
	}
	return 0;
}
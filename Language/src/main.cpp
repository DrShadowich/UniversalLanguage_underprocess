#include <iostream>
#include <fstream>
#include <lexer.h>
#include <statement_info.h>
#include <parser.h>
#include <code_generator.h>
#include <function_names_memory.h>
static void writeModuleToLL(llvm::Module& M, llvm::StringRef Path)
{
	std::error_code EC;
	llvm::raw_fd_ostream OS(Path, EC);
	if (EC) {
		
		return;
	}
	M.print(OS, nullptr);
	OS.flush();
}

int main()
{
	std::ifstream file{ "main.ul" };
	std::string input{ std::istreambuf_iterator<char>{ file.rdbuf() }, std::istreambuf_iterator<char>{ } };
	file.close();
	ul::lexer::language_lexer l{ input };
	ul::parser::language_parser p{ l };
	auto&& stmts = p.parse_program();
	llvm::LLVMContext ctx{};
	llvm::Module module_{ "main", ctx };
	ul::codegen::code_generator c{ ctx, module_, std::move(stmts->statements) };
	try
	{
		c.generate_statements();
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		throw;
	}
	writeModuleToLL(module_, "main.ll");
	return 0;
}
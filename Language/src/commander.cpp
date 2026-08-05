#include <commander.h>
#include <fstream>
#include <commentaries.h>
#include <core_exceptions.h>
#include <iostream>
#include <parser.h>
#include <code_generator.h>
#include <msvc_details.h>

namespace fs = std::filesystem;

cmd::commander::commander(int argc, char** argv) :
	cctx_{ std::make_unique<commander_context>() }
{
	for (int i{ 1 }; i < argc; ++i)
		commands_.emplace_back(argv[i]);
}

std::string cmd::commander_context::cmd_reveal_USL(bool python_used)
{
	std::string cmd_result;
	cmd_result += USL_path + "\\USL.cpp ";
	if(python_used)
		cmd_result += USL_path + "\\py_ul.cpp ";
	return cmd_result;
}

void cmd::commander::execute_commands()
{
	using namespace ul::utils;
	bool next_file_is_output = false;
	bool output_file_chosen = false;
	if (commands_.empty())
	{
		std::cout << "------------------\nUniversal Language\n------------------";
		std::cout << "> Supported commands:\n";
		std::cout << "> -o -> set output file.\n";
		std::cout << "> any .ul file -> add .ul file for compilation.\n";
		std::cout << "> any .cxx .c .c++ .cc .c .h .hh .hpp file -> add c/c++ file for compilation.\n";
		std::cout << "> -ON -> set optimization leves: avalieable -01, -02, -03\n";
		return;
	}
	for (auto&& com : commands_)
	{
		if (com == "-o")
		{
			if (next_file_is_output)
				COMMANDER_EXCEPTION("-o is alredy set");
			next_file_is_output = true;
		}
		else if (next_file_is_output)
		{
			if (output_file_chosen)
				COMMANDER_EXCEPTION("Output file already set");
			output_file_name_ = std::move(com);
		}
		else if (end_with(".ul", com))
		{
			ul_files_.emplace(std::move(com));
		}
		else if(end_with(".cpp", com))
		{
			other_files_.emplace(std::move(com));
		}
		else if(starts_with("-O", com))
		{
			std::string level = com.substr(com.find_first_of("-O"), com.size());
			for (char s : level)
				if (!isdigit(s))
					COMMANDER_EXCEPTION("Optimizations level isn\'t number");
			optimization_level_ = std::stoi(level);
		}
		else
		{
			other_files_.emplace(std::move(com));
		}
	}

	if (ul_files_.empty() && other_files_.empty())
		COMMANDER_EXCEPTION("No input files");

	for(auto&& ul_file : ul_files_)
	{
		compile_ul_files(ul_file);
	}

	if (file_modules_.empty())
		COMMANDER_EXCEPTION("No work to do");


	bool python_used = false;
	bool native_c_used = false;
	std::string ll_files;
	std::string additional_information;
	std::string native_comp = std::format("{}.exe -c", cctx_->clang_path);


	for (auto&& mod : file_modules_)
	{
		if (not mod.python_file().empty() && !python_used)
		{
			python_used = true;
			auto py_dirs = mod.reveal_python();
			additional_information += std::format(" -I{} -L{} -l{}", py_dirs.python_include, py_dirs.python_libs, py_dirs.python_lib);
			auto dll = fs::path(py_dirs.python_dll.substr(1, py_dirs.python_dll.size() - 2));
			auto libs = fs::path(py_dirs.python_libs.substr(1, py_dirs.python_libs.size() - 2));
			if(not fs::exists(libs))
				fs::copy(dll, fs::current_path(), fs::copy_options::overwrite_existing);
			if(not fs::exists(dll))
				fs::copy(libs, fs::current_path() / "Lib", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
		}
		if (not mod.cpp_file().empty())
			other_files_.emplace(std::move(mod.cpp_file()));
		if (not mod.c_file().empty())
		{
			native_c_used = true;
			native_comp += std::format(" \"{}\"", mod.c_file());
		}
		if(not mod.llvm_file().empty())
		{
			ll_files += std::format(" \"{}\"", mod.llvm_file());
		}
	}
	std::string other_files;
	for(auto&& file : other_files_)
	{
		other_files += std::format(" \"{}\"", file);
	}

	std::string native_obj_file_name = "native_c.obj";

	if(native_c_used)
	{
		native_comp += std::format(" -o \"{}\"", native_obj_file_name);
		cmd::start_clang(std::move(native_comp));
	}

	std::string output_comp = 
		std::format
		(//  1					   2  3  4  5  6  7     8
			"{}++.exe -std=c++20 -O{} {} {} {} {} {} -o {} -Woverride-module", 
		 	cctx_->clang_path, // 1
			optimization_level_, // 2
			other_files, // 3
			ll_files, // 4
			cctx_->cmd_reveal_USL(python_used), // 5
			additional_information, // 6
			native_c_used ? native_obj_file_name : "",
			output_file_name_.empty() ? "a.exe" : output_file_name_ // 8
		);
	
	cmd::start_clangpp(std::move(output_comp));
	std::cout << std::format("Result in: {}", output_file_name_.empty() ? "a.exe" : output_file_name_) << std::endl;
	return;
}

std::set<std::string> cmd::commander::get_ul_files()
{
	return ul_files_;
}

cmd::commander_context::commander_context() :
	my_path{ fs::current_path().string() },
	USL_path{ (fs::current_path() / "USL").string() },
	clang_path{ (fs::current_path() / "clang").string() }
{}

void cmd::commander::write_llvm_module(llvm::Module& M, llvm::StringRef Path)
{
	std::error_code EC;
	llvm::raw_fd_ostream OS(Path, EC);
	if (EC) {
		EXCEPTION("Got error while generating .ll code");
		return;
	}
	M.print(OS, nullptr);
	OS.flush();
}

void cmd::commander::compile_ul_files(std::string file_name)
{
	std::string module_name = file_name.substr(0, file_name.find_first_of(".ul"));
	std::ifstream file{ file_name };
	std::string input{ std::istreambuf_iterator<char>{ file.rdbuf() }, std::istreambuf_iterator<char>{ } };
	
	llvm::LLVMContext ctx{};
	llvm::Module module_{ module_name, ctx};
	try
	{
		ul::lexer::language_lexer l{ input };
		ul::parser::language_parser p{ l };
		auto&& stmts = p.parse_program();
		ul::codegen::code_generator c{ ctx, module_, std::move(stmts->statements) };

		c.generate_statements();
		auto&& fm = c.get_file_module();
		for(auto&& rhs_fm : file_modules_)
		{
			if (fm.llvm_file() == rhs_fm.llvm_file())
				return;
		}
		write_llvm_module(module_, fm.llvm_file());
		file_modules_.emplace_back(std::move(fm));
	}

#ifndef DEBUG
	catch(ul::ex::commander_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return;
	}
	catch (ul::ex::code_generator_exception& ex)
	{
		std::cerr << std::format("Critical error in compiler {}\nThis message is code generator\'s exception.", ex.what()) << std::endl;
		return;
	}
	catch (ul::ex::parser_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return;
	}
#endif
#ifdef DEBUG
	catch (ul::ex::commander_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		throw;
	}
	catch (ul::ex::code_generator_exception& ex)
	{
		std::cerr << std::format("Critical error in compiler {}\nThis message is code generator\'s exception.", ex.what()) << std::endl;
		throw;
	}
	catch (ul::ex::parser_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		throw;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		throw;
	}
#endif
}
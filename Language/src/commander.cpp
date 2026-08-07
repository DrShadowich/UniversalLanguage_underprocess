#include <commander.h>
#include <fstream>

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

ul::utils::classes::stringi8 cmd::commander_context::cmd_reveal_USL(bool python_used)
{
	ul::utils::classes::stringi8 cmd_result;
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
		std::cout << "------------------\nUniversal Language\n------------------\n";
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
			output_file_chosen = true;
		}
		else if (com.end_with(".ul"))
		{
			ul_files_.emplace(std::move(com));
		}
		else if(com.end_with(".cpp"))
		{
			other_files_.emplace(std::move(com));
		}
		else if(com.starts_with("-O"))
		{
			ul::utils::classes::stringi8 level = com.substr(2);
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
	ul::utils::classes::stringi8 ll_files;
	ul::utils::classes::stringi8 additional_information;
	ul::utils::classes::stringi8 native_information;
	
	ul::utils::classes::stringi8 native_comp = std::format("{}.exe -c", cctx_->clang_path);

	fs::path output_file{};
	fs::path output_dir{};
	if (output_file_chosen)
	{
		output_file = fs::path(static_cast<const std::string&>(output_file_name_));
		output_dir = output_file.parent_path();
	}
	else
		output_dir = fs::current_path();

	for (auto&& mod : file_modules_)
	{
		if (not mod.python_file().empty() && !python_used)
		{
			python_used = true;
			auto py_dirs = mod.reveal_python();
			auto libs = py_dirs.python_libs.uncover_this_with('\"').make_path(); 
			auto dll = py_dirs.python_dll.uncover_this_with('\"').make_path(); 

			additional_information += std::format(" -I{} -L{} -l{}", py_dirs.python_include, py_dirs.python_libs, py_dirs.python_lib);
			
			if (not fs::exists(output_dir))
				fs::create_directory(output_dir);
			if (not fs::exists(output_dir / "Lib"))
				fs::create_directory(output_dir / "Lib");
			if (not fs::exists(output_dir / "DLLs"))
				fs::create_directory(output_dir / "DLLs");

			std::cout << std::format("> ul: Copying {} in {}\n", dll.filename().string(), output_dir.string());
			fs::copy(dll, output_dir, fs::copy_options::skip_existing);


			std::cout << std::format("> ul: Copying Python Libs in {}\n", (output_dir / "Lib").string());
			fs::copy(libs, output_dir / "Lib", fs::copy_options::skip_existing | fs::copy_options::recursive);

			std::cout << std::format("> ul: Copying Python DLLs in {}\n", (output_dir / "DLLs").string());
			fs::copy(py_dirs.python_dlls.uncover_this_with('\"').make_path(), output_dir / "DLLs", fs::copy_options::skip_existing | fs::copy_options::recursive);

		}
		fs::create_directory(output_dir / "rest");
		if (not mod.cpp_file().empty())
		{
			std::cout << std::format("{} file was copied on {}\n", mod.cpp_file(), (output_dir / "rest").string());
			fs::copy(mod.cpp_file().make_path(), (output_dir / "rest" / mod.cpp_file().make_path().filename()), fs::copy_options::overwrite_existing);
			
			other_files_.emplace(std::move(mod.cpp_file()));
		}
		if (not mod.c_file().empty())
		{
			native_c_used = true;
			
			std::cout << std::format("{} file was copied on {}\n", mod.c_file(), (output_dir / "rest").string());
			fs::copy(mod.c_file().make_path(), (output_dir / "rest" / mod.c_file().make_path().filename()), fs::copy_options::overwrite_existing);
			
			native_comp += std::format(" \"{}\"", mod.c_file());
		}
		if(not mod.llvm_file().empty())
		{
			std::cout << std::format("{} file was copied on {}\n", mod.llvm_file(), (output_dir / "rest").string());
			fs::copy(mod.llvm_file().make_path(), (output_dir / "rest" / mod.llvm_file().make_path().filename()), fs::copy_options::overwrite_existing);

			ll_files += std::format(" \"{}\"", mod.llvm_file());
		}
		if (not mod.additional_cmd_information.empty())
		{
			additional_information += std::format(" {}", mod.additional_cmd_information);
			native_information += std::format(" {}", mod.additional_cmd_information);
		}
	}
	ul::utils::classes::stringi8 other_files;
	for(auto&& file : other_files_)
	{
		other_files += std::format(" \"{}\"", file);
	}

	ul::utils::classes::stringi8 native_obj_file_name = "native_c.obj";

	additional_information += " -w ";
	native_information += " -w ";
	additional_information += std::format(" -static -L\"{}\"", (fs::current_path() / "CppLibs").string());
	additional_information += " -luser32.lib ";
	native_information += " -luser32.lib ";

	if(native_c_used)
	{
		native_comp += std::format(" {} -o \"{}\"", native_information, native_obj_file_name);
		cmd::start_clang(std::move(native_comp));
	}

	// Windows specific

	ul::utils::classes::stringi8 output_comp = 
		std::format
		(//  1						2  3  4  5  6  7     8
			"{}++.exe -std=c++20 -O{} {} {} {} {} {} -o {} ", 
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

std::set<ul::utils::classes::stringi8> cmd::commander::get_ul_files()
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

void cmd::commander::compile_ul_files(ul::utils::classes::stringi8 file_name)
{
	ul::utils::classes::stringi8 module_name = file_name.substr(0, file_name.find_first_of(".ul"));
	std::ifstream file{ file_name };
	ul::utils::classes::stringi8 input{ std::istreambuf_iterator<char>{ file.rdbuf() }, std::istreambuf_iterator<char>{ } };
	
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
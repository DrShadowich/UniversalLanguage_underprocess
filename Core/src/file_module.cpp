#include <file_module.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;


ul::utils::file_module::file_module(ul::utils::file_module&& rhs) noexcept :
	directories{ std::move(rhs.directories) }, additional_cmd_information{ std::move(rhs.additional_cmd_information) }
{}

ul::utils::file_module::file_module()
{}

ul::utils::language_directories::language_directories()
{}
ul::utils::language_directories::language_directories(language_directories&& rhs) noexcept :
	python_dir{ std::move(rhs.python_dir) }, cpp_file_dir{ std::move(rhs.cpp_file_dir) },
	c_file_dir{ std::move(rhs.c_file_dir) }, llvm_file_dir{ std::move(rhs.llvm_file_dir) }
{}


void ul::utils::file_module::append_cmd_information(classes::stringi8 cmd) noexcept
{
	additional_cmd_information += std::format(" {}", std::move(cmd));
}

ul::utils::detail::python_compile_strings ul::utils::file_module::reveal_python() const
{
	detail::python_compile_strings pcs{};
	classes::stringi8 clear_path;
	if (directories->python_dir.covered_with('"'))
		clear_path = directories->python_dir.substr(1, directories->python_dir.size() - 2);
	else clear_path = directories->python_dir;

	pcs.python_dir = std::format("\"{}\"", clear_path);
	std::cout << std::format("> py - ul: Use directory {}\n", pcs.python_dir);

	pcs.python_libs = std::format("\"{}\"", clear_path + "\\Lib");
	std::cout << std::format("> py - ul: Added Libs directory {}\n", pcs.python_libs);
	
	pcs.python_dlls = std::format("\"{}\"", clear_path + "\\DLLs");
	std::cout << std::format("> py - ul: Added DLLs directory {}\n", pcs.python_dlls);

	pcs.python_lib = clear_path + "\\libs";
	for (const auto& entry : fs::directory_iterator(static_cast<std::string>(pcs.python_lib)))
	{
		if (not entry.is_regular_file())
			continue;

		classes::stringi8 fn = entry.path().filename().string();
		if (fn == "python3.lib")
			continue;
		else if (fn.starts_with("python") && fn.end_with(".lib"))
		{
			classes::stringi8 fn_dll = fn.substr(0, fn.find_last_of('.')) + ".dll";
			pcs.python_lib = std::format("\"{}\"", pcs.python_lib + "\\" + fn);
			pcs.python_dll = std::format("\"{}\"", clear_path + "\\" + fn_dll);
			std::cout << std::format("> py - ul: Added static library {}\n", pcs.python_lib);
			std::cout << std::format("> py - ul: Added dynamic library {}\n", pcs.python_dll);
		}
		else
			std::cout << std::format("> py - ul: Skip file {}\n", fn);
	}

	pcs.python_include = std::format("\"{}\"", clear_path + "\\include");
	std::cout << std::format("> py - ul: Added include path {}\n", pcs.python_include);

	return pcs;
}

void ul::utils::file_module::set_llvm_file(classes::stringi8 file) const noexcept
{
	directories->llvm_file_dir = std::move(file);
}

void ul::utils::file_module::set_cpp_file(classes::stringi8 file) const noexcept
{
	directories->cpp_file_dir = std::move(file);
}
void ul::utils::file_module::set_c_file(classes::stringi8 file) const noexcept
{
	directories->c_file_dir = std::move(file);
}
void ul::utils::file_module::set_python_file(classes::stringi8 file) const noexcept
{
	directories->python_dir = std::move(file);
}

ul::utils::classes::stringi8 ul::utils::file_module::cpp_file()
{
	return directories->cpp_file_dir;
}
ul::utils::classes::stringi8 ul::utils::file_module::c_file()
{
	return directories->c_file_dir;
}
ul::utils::classes::stringi8 ul::utils::file_module::python_file()
{
	return directories->python_dir;
}
 
ul::utils::classes::stringi8 ul::utils::file_module::llvm_file()
{
	return directories->llvm_file_dir;
}
#include <file_module.h>
#include <filesystem>
#include <commentaries.h>
namespace fs = std::filesystem;


ul::utils::file_module::file_module(ul::utils::file_module&& rhs) noexcept :
	directories{ std::move(rhs.directories) }
{}

ul::utils::file_module::file_module()
{}

ul::utils::language_directories::language_directories()
{}
ul::utils::language_directories::language_directories(language_directories&& rhs) noexcept :
	python_dir{ std::move(rhs.python_dir) }, cpp_file_dir{ std::move(rhs.cpp_file_dir) },
	c_file_dir{ std::move(rhs.c_file_dir) }, llvm_file_dir{ std::move(rhs.llvm_file_dir) }
{}


ul::utils::detail::python_compile_strings ul::utils::file_module::reveal_python()
{
	detail::python_compile_strings pcs{};
	std::string clear_path;
	if (utils::covered_with('"', directories->python_dir))
		clear_path = directories->python_dir.substr(1, directories->python_dir.size() - 2);
	else clear_path = directories->python_dir;

	pcs.python_dir = std::format("\"{}\"", clear_path);
	pcs.python_libs = std::format("\"{}\"", clear_path + "\\Lib");
	pcs.python_lib = clear_path + "\\libs";

	for (const auto& entry : fs::directory_iterator(pcs.python_lib))
	{
		if (not entry.is_regular_file())
			continue;

		std::string fn = entry.path().filename().string();
		if (fn == "python3.lib")
			continue;
		else if (utils::starts_with("python", fn) && end_with(".lib", fn))
		{
			std::string fn_dll = fn.substr(0, fn.find_last_of('.')) + ".dll";
			pcs.python_lib = std::format("\"{}\"", pcs.python_lib + "\\" + fn);
			pcs.python_dll = std::format("\"{}\"", clear_path + "\\" + fn_dll);
		}
	}
	pcs.python_include = std::format("\"{}\"", clear_path + "\\include");
	return pcs;
}

void ul::utils::file_module::set_llvm_file(std::string file) const noexcept
{
	directories->llvm_file_dir = std::move(file);
}

void ul::utils::file_module::set_cpp_file(std::string file) const noexcept
{
	directories->cpp_file_dir = std::move(file);
}
void ul::utils::file_module::set_c_file(std::string file) const noexcept
{
	directories->c_file_dir = std::move(file);
}
void ul::utils::file_module::set_python_file(std::string file) const noexcept
{
	directories->python_dir = std::move(file);
}

std::string ul::utils::file_module::cpp_file()
{
	return directories->cpp_file_dir;
}
std::string ul::utils::file_module::c_file()
{
	return directories->c_file_dir;
}
std::string ul::utils::file_module::python_file()
{
	return directories->python_dir;
}

std::string ul::utils::file_module::llvm_file()
{
	return directories->llvm_file_dir;
}
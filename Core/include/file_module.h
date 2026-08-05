#pragma once
#include <string>
#include <string_view>
#include <memory>

namespace ul::utils
{
	namespace detail
	{
		struct python_compile_strings
		{
			std::string python_dir;
			std::string python_libs;
			std::string python_lib;
			std::string python_include;
			std::string python_dll;
		};

		struct cpp_compile_strings
		{
			std::string file_dir;
		};

		struct c_compile_strings
		{
			std::string file_dir;
		};
	}

	struct language_directories
	{
		std::string llvm_file_dir{ "" };
		std::string ul_file_dir{ "" };
		std::string cpp_file_dir{ "" };
		std::string c_file_dir{ "" };
		std::string python_dir{ "" };
		language_directories();
		language_directories(language_directories&&) noexcept;
	};

	struct file_module
	{
	public:
		std::unique_ptr<language_directories> directories = std::make_unique<language_directories>();

		file_module(const file_module&) = delete;
		file_module& operator=(const file_module&) = delete;
		file_module(file_module&& rhs) noexcept;
		file_module();

		detail::python_compile_strings reveal_python();
		void set_cpp_file(std::string file) const noexcept;
		void set_c_file(std::string file) const noexcept ;
		void set_python_file(std::string file) const noexcept;
		void set_llvm_file(std::string file) const noexcept;
		std::string cpp_file();
		std::string c_file();
		std::string python_file();
		std::string llvm_file();
	};
}
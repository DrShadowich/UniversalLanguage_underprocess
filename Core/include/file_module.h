#pragma once
#include <ulstring.h>
#include <string_view>
#include <memory>

namespace ul::utils
{
	namespace detail
	{
		struct python_compile_strings
		{
			classes::stringi8 python_dir;
			classes::stringi8 python_libs;
			classes::stringi8 python_lib;
			classes::stringi8 python_include;
			classes::stringi8 python_dll;
			classes::stringi8 python_dlls;
		};

		struct cpp_compile_strings
		{
			classes::stringi8 file_dir;
		};

		struct c_compile_strings
		{
			classes::stringi8 file_dir;
		};
	}

	struct language_directories
	{
		classes::stringi8 llvm_file_dir{ "" };
		classes::stringi8 ul_file_dir{ "" };
		classes::stringi8 cpp_file_dir{ "" };
		classes::stringi8 c_file_dir{ "" };
		classes::stringi8 python_dir{ "" };
		language_directories();
		language_directories(language_directories&&) noexcept;
	};

	struct file_module
	{
	public:
		std::unique_ptr<language_directories> directories = std::make_unique<language_directories>();
		classes::stringi8  additional_cmd_information{ "" };

		file_module(const file_module&) = delete;
		file_module& operator=(const file_module&) = delete;
		file_module(file_module&& rhs) noexcept;
		file_module();

		detail::python_compile_strings reveal_python() const;
		void set_cpp_file( classes::stringi8  file) const noexcept;
		void set_c_file( classes::stringi8  file) const noexcept ;
		void set_python_file( classes::stringi8  file) const noexcept;
		void set_llvm_file( classes::stringi8  file) const noexcept;
		void append_cmd_information( classes::stringi8  cmd) noexcept;
		classes::stringi8  cpp_file();
		classes::stringi8  c_file();
		classes::stringi8  python_file();
		classes::stringi8  llvm_file();
	};
}
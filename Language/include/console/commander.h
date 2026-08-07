#pragma once
#include <ulstring.h>
#include <vector>
#include <set>
#include <memory>
#include <filesystem>
#include <llvm+.h>
#include <file_module.h>

namespace cmd
{

	struct commander_context
	{
		ul::utils::classes::stringi8 my_path;
		ul::utils::classes::stringi8 USL_path;
		ul::utils::classes::stringi8 clang_path;
		commander_context();
		ul::utils::classes::stringi8 cmd_reveal_USL(bool python_used);
	};
	// Provide cmd commands with language
	class commander final
	{
	private:
		std::unique_ptr<commander_context> cctx_;
		ul::utils::classes::stringi8 output_file_name_{};
		uint32_t optimization_level_{ 0 };
		std::set<ul::utils::classes::stringi8> other_files_{};
		std::set<ul::utils::classes::stringi8> ul_files_{};
		std::vector<ul::utils::classes::stringi8> commands_{};
		std::vector<ul::utils::file_module> file_modules_{};
	public:
		commander(int argc, char** argv);
		std::set<ul::utils::classes::stringi8> get_ul_files();
		void execute_commands();
		void compile_ul_files(ul::utils::classes::stringi8 file_name);
	private:
		static void write_llvm_module(llvm::Module& M, llvm::StringRef Path);
	};
}
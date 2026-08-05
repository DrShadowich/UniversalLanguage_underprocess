#pragma once
#include <string>
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
		std::string my_path;
		std::string USL_path;
		std::string clang_path;
		commander_context();
		std::string cmd_reveal_USL(bool python_used);
	};
	// Provide cmd commands with language
	class commander final
	{
	private:
		std::unique_ptr<commander_context> cctx_;
		std::string output_file_name_{};
		uint32_t optimization_level_{ 0 };
		std::set<std::string> other_files_{};
		std::set<std::string> ul_files_{};
		std::vector<std::string> commands_{};
		std::vector<ul::utils::file_module> file_modules_{};
	public:
		commander(int argc, char** argv);
		std::set<std::string> get_ul_files();
		void execute_commands();
		void compile_ul_files(std::string file_name);
	private:
		static void write_llvm_module(llvm::Module& M, llvm::StringRef Path);
	};
}
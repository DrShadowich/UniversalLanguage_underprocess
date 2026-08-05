#pragma once
#include <llvm+.h>
#include <name_table.h>
#include <file_module.h>

namespace ul::codegen
{
	class marker_generator
	{
	private:
		llvm::LLVMContext& ctx_;
		llvm::IRBuilder<>& builder_;
		llvm::Module& mod_;
		utils::name_table& names_table_;
		utils::file_module& file_module_;
		uint32_t function_counter_{ 0 };
	public:
		marker_generator(llvm::LLVMContext&, llvm::IRBuilder<>&, llvm::Module&, utils::name_table&, utils::file_module&);
		void generate_config(std::string code);
		void generate_python_code(std::string code);
		void generate_cpp_code(std::string code);
		void generate_c_code(std::string code);
	};
}
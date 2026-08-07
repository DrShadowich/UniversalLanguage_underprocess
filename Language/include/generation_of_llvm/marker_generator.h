#pragma once
#include <llvm+.h>
#include <name_table.h>
#include <file_module.h>
#include <ulstring.h>
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
		
		static inline std::string cpp_code_template;
		static inline std::string c_code_template;
		static inline std::string python_code_template;
	public:
		marker_generator(llvm::LLVMContext&, llvm::IRBuilder<>&, llvm::Module&, utils::name_table&, utils::file_module&);
		void generate_config(utils::classes::stringi8 code);
		void generate_python_code(utils::classes::stringi8 code, bool in_function);
		void generate_cpp_code(utils::classes::stringi8 code, bool in_function);
		void generate_c_code(utils::classes::stringi8 code, bool in_function);
	};
}
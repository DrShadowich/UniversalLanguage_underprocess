#pragma once
#include <unordered_map>
#include <string>
#include <llvm_value.h>
#include <core_exceptions.h>

namespace ul::utils
{


	class name_table final
	{
	private:
		using map_value = std::pair<llvm::Value*, uint32_t>;
		using map_type = std::pair<llvm::Type*, uint32_t>;
		using map_function = std::pair<llvm::Function*, uint32_t>;
		using map_functiontype = std::pair<llvm::FunctionType*, uint32_t>;
		std::unordered_map<classes::stringi8, map_value> map_value_{};
		std::unordered_map<classes::stringi8, map_type> map_type_{};
		std::unordered_map<classes::stringi8, map_function> map_function_{};
		std::unordered_map<classes::stringi8, map_functiontype> map_function_type_{};

		uint32_t cur_depth_{};
	

	public:
		~name_table();
		name_table();
		name_table(const name_table&) = delete;
		name_table(name_table&&) = delete;
		name_table& operator=(const name_table&) = delete;
		name_table& operator=(name_table&&) = delete;
		
		void insert(const classes::stringi8& var_name, llvm::Value* value);
		void insert(const classes::stringi8& type_name, llvm::Type* type);
		void insert(const classes::stringi8& func_name, llvm::Function* function);

		bool contains_variable(const classes::stringi8& name);
		bool contains_type(const classes::stringi8& name);
		bool contains_function(const classes::stringi8& name);
		
		std::vector<llvm::Value*> get_all_variables();

		llvm::Value* get_variable(const classes::stringi8& name);
		llvm::Type* get_type(const classes::stringi8& name);
		llvm::Function* get_function(const classes::stringi8& name);


		void update_namespace(uint32_t new_depth);
			
		void operator++();
		void operator--();
	};
}
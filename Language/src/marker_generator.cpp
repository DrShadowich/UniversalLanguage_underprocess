#include <marker_generator.h>
#include <sstream>
#include <dictionaries.h>
#include <fstream>
#include <format>

#define CODEGEN ul::codegen::

CODEGEN marker_generator::marker_generator(llvm::LLVMContext& ctx, llvm::IRBuilder<>& builder, llvm::Module& mod, utils::name_table& nt, utils::file_module& fm) :
	ctx_{ ctx }, builder_{ builder }, names_table_{ nt }, mod_{ mod }, file_module_{ fm }
{}

void CODEGEN marker_generator::generate_config(utils::classes::stringi8 body)
{
	body.trim_assginment();
	std::istringstream is{ body };
	utils::classes::stringi8 line;
	do
	{
		std::getline(is, line);
		if (line.empty())
			continue;
		utils::classes::stringi8 variable = line.substr(0, line.find_first_of('='));
		utils::classes::stringi8 value = line.substr(line.find_first_of('=') + 1);
		if (variable == "python_dir")
			file_module_.set_python_file(std::move(value));
		else if (variable == "c_include")
		{
			if (not value.covered_with('\"'))
				value.cover_this_with("\"");
			file_module_.append_cmd_information(std::format("-I{}", std::move(value)));
		}
		else
			CODE_GENERATOR_EXCEPTION("Unhandled config variable");
	} while (is.good());
}

void CODEGEN marker_generator::generate_python_code(utils::classes::stringi8 body, bool in_function)
{
	if (file_module_.python_file().empty())
		CODE_GENERATOR_EXCEPTION("UL haven\'t got Python");
	std::vector<llvm::Type*> run_type
	{
		llvm::Type::getInt8Ty(ctx_)->getPointerTo()
	};
	std::vector<llvm::Type*> ins_type
	{
		// char* var_name, void* data, type data_type
		llvm::Type::getInt8Ty(ctx_)->getPointerTo(),
		llvm::Type::getInt8Ty(ctx_)->getPointerTo(),
		llvm::Type::getInt32Ty(ctx_)
	};
	std::vector<llvm::Type*> get_type
	{
		// char* var_name, void* data, type data_type, int flag
		llvm::Type::getInt8Ty(ctx_)->getPointerTo(),
		llvm::Type::getInt8Ty(ctx_)->getPointerTo(),
		llvm::Type::getInt32Ty(ctx_),
		llvm::Type::getInt32Ty(ctx_),
	};
	auto* py_get_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), get_type, false);
	auto py_get = mod_.getOrInsertFunction("__py_get", py_get_type);
	auto* py_insert_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), ins_type, false);
	auto py_insert = mod_.getOrInsertFunction("__py_insert", py_insert_type);
	auto* py_exec_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx_), run_type, false);
	auto py_exec = mod_.getOrInsertFunction("__py_run", py_exec_type);
	
	for (auto&& i : names_table_.get_all_variables())
	{
		utils::classes::stringi8 str_var_name = i->getName().str();
		std::vector<llvm::Value*> args =
		{
			create_string(mod_, builder_, ctx_, str_var_name),
			i,
			llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(ctx_),
				static_cast<int32_t>(dictionaries::ul_native_type_number.at(str_var_name.get_type_from_name()))),
		};
		builder_.CreateCall(py_insert, std::move(args));
	}
	if (in_function)
	{
		std::string py_string = std::format("{}\n{}", python_code_template, body);
		builder_.CreateCall(py_exec, { create_string(mod_, builder_, ctx_, py_string) });
		auto vec = names_table_.get_all_variables();
		auto it_last = std::prev(vec.end());
		for (auto&& i : vec)
		{
			utils::classes::stringi8 str_var_name = i->getName().str();
			std::vector<llvm::Value*> args =
			{
				create_string(mod_, builder_, ctx_, str_var_name),
				i,
				llvm::ConstantInt::get(
				llvm::Type::getInt32Ty(ctx_),
					static_cast<int32_t>(dictionaries::ul_native_type_number.at(str_var_name.get_type_from_name()))),
			};
			if (*it_last == i)
				args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0));
			else
				args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 1));
			builder_.CreateCall(py_get, std::move(args));
		}
	}
	else
		python_code_template += std::format("{}\n", body);
}
void CODEGEN marker_generator::generate_cpp_code(utils::classes::stringi8 body, bool in_function)
{
	utils::classes::stringi8 cpp_function_name = std::format("__cpp{}", std::to_string(function_counter_++));
	if (file_module_.cpp_file().empty())
	{
		file_module_.set_cpp_file(std::format("{}.cpp", mod_.getName().str()));
		std::ofstream cpp_file{ file_module_.cpp_file(), std::ios::binary };
		cpp_file.close();
	}
	std::istringstream is{ body };
	utils::classes::stringi8 remains;
	utils::classes::stringi8 main;
	utils::classes::stringi8 line;
	do
	{
		std::getline(is, line);
		line.remove_all('\t');
		if (line.empty())
			continue;
		if (line.starts_with("#include"))
			remains += std::format("{}\n", line);
		else
			main += std::format("{}\n", line);
	} while (is.good());

	remains += cpp_code_template;

	std::ifstream cpp_file{ file_module_.cpp_file() };
	utils::classes::stringi8 old_body{ std::istreambuf_iterator<char>(cpp_file.rdbuf()), std::istreambuf_iterator<char>() };
	cpp_file.close();

	std::ofstream ocpp_file{ file_module_.cpp_file() };
	if (in_function)
	{
		//                       1  2                    3          4         1        2         3                  4
		ocpp_file << std::format("{} {} extern \"C\" void {}()\n{{\n{} }}\n", remains, old_body, cpp_function_name, main);
		ocpp_file.close();
		auto* cpp_exec_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), false);
		auto cpp_exec = mod_.getOrInsertFunction(cpp_function_name, cpp_exec_type);
		builder_.CreateCall(cpp_exec);
	}
	else
		cpp_code_template += std::format("{}\n", body);
}
void CODEGEN marker_generator::generate_c_code(utils::classes::stringi8 body, bool in_function)
{
	utils::classes::stringi8 c_function_name = std::format("__c{}", std::to_string(function_counter_++));
	if (file_module_.c_file().empty())
	{
		file_module_.set_c_file(std::format("{}.c", mod_.getName().str()));
		std::ofstream c_file{ file_module_.c_file(), std::ios::binary };
		c_file.close();
	}
	std::ifstream c_file{ file_module_.c_file() };
	std::istringstream is{ body };
	utils::classes::stringi8 remains;
	utils::classes::stringi8 main;
	utils::classes::stringi8 line;
	do
	{
		std::getline(is, line);
		line.remove_all('\t');
		if (line.empty())
			continue;
		if (line.starts_with("#include"))
			remains += std::format("{}\n", line);
		else
			main += std::format("{}\n", line);
	} while (is.good());

	remains += c_code_template;

	utils::classes::stringi8 old_body;
	c_file >> old_body;
	c_file.close();

	std::ofstream oc_file{ file_module_.c_file(), std::ios::app };
	if (in_function)
	{
		//                    
		oc_file << std::format("#ifndef __cplusplus\n{} {} void {}(void)\n{{\n{} }}\n#endif\n", remains, old_body, c_function_name, main);
		oc_file.close();
		auto* c_exec_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), false);
		auto c_exec = mod_.getOrInsertFunction(c_function_name, c_exec_type);
		builder_.CreateCall(c_exec);
	}
	else
		c_code_template += std::format("{}\n", body);
}
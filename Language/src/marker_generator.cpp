#include <marker_generator.h>
#include <commentaries.h>
#include <simple_variable.h>
#include <sstream>
#include <dictionaries.h>
#include <fstream>
#include <format>

#define CODEGEN ul::codegen::

CODEGEN marker_generator::marker_generator(llvm::LLVMContext& ctx, llvm::IRBuilder<>& builder, llvm::Module& mod, utils::name_table& nt, utils::file_module& fm) :
	ctx_{ ctx }, builder_{ builder }, names_table_{ nt }, mod_{ mod }, file_module_{ fm }
{}

void CODEGEN marker_generator::generate_config(std::string body)
{
	body = utils::trim_assginment(body);
	std::istringstream is{ body };
	std::string line;
	do
	{
		std::getline(is, line);
		if (line.empty())
			continue;
		std::string variable = line.substr(0, line.find('='));
		std::string value = line.substr(line.find('=') + 1);
		if (variable == "python_dir")
			file_module_.set_python_file(std::move(value));
		else
			CODE_GENERATOR_EXCEPTION("Unhandled config variable");
	} while (is.good());
}

void CODEGEN marker_generator::generate_python_code(std::string body)
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
		std::string str_var_name = i->getName().str();
		std::vector<llvm::Value*> args =
		{
			create_string(mod_, builder_, ctx_, str_var_name),
			i,
			llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(ctx_),
				static_cast<int32_t>(dictionaries::ul_native_type_number.at(utils::get_type_from_name(str_var_name)))),
		};
		builder_.CreateCall(py_insert, std::move(args));
	}
	builder_.CreateCall(py_exec, { create_string(mod_, builder_, ctx_, body) });
	auto vec = names_table_.get_all_variables();
	auto it_last = std::prev(vec.end());
	for (auto&& i : vec)
	{
		std::string str_var_name = i->getName().str();
		std::vector<llvm::Value*> args =
		{
			create_string(mod_, builder_, ctx_, str_var_name),
			i,
			llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(ctx_),
				static_cast<int32_t>(dictionaries::ul_native_type_number.at(utils::get_type_from_name(str_var_name)))),
		};
		if (*it_last == i)
			args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0));
		else
			args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 1));
		builder_.CreateCall(py_get, std::move(args));
	}
}
void CODEGEN marker_generator::generate_cpp_code(std::string body)
{
	std::string cpp_function_name = std::format("__cpp{}", std::to_string(function_counter_++));
	if (file_module_.cpp_file().empty())
	{
		file_module_.set_cpp_file(std::format("{}.cpp", mod_.getName().str()));
		std::ofstream cpp_file{ file_module_.cpp_file(), std::ios::binary };
		cpp_file.close();
	}
	std::istringstream is{ body };
	std::string remains;
	std::string main;
	std::string line;
	do
	{
		std::getline(is, line);
		line = utils::remove_all('\t', line);
		if (line.empty())
			continue;
		if (utils::starts_with("#include", line))
			remains += std::format("{}\n", line);
		else
			main += std::format("{}\n", line);
	} while (is.good());

	std::ifstream cpp_file{ file_module_.cpp_file() };

	std::string old_body{ std::istreambuf_iterator<char>(cpp_file.rdbuf()), std::istreambuf_iterator<char>() };

	cpp_file.close();
	std::ofstream ocpp_file{ file_module_.cpp_file() };
	//                       1  2                    3          4         1        2         3                  4
	ocpp_file << std::format("{} {} extern \"C\" void {}()\n{{\n{} }}\n", remains, old_body, cpp_function_name, main);
	ocpp_file.close();
	auto* cpp_exec_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), false);
	auto cpp_exec = mod_.getOrInsertFunction(cpp_function_name, cpp_exec_type);
	builder_.CreateCall(cpp_exec);
}
void CODEGEN marker_generator::generate_c_code(std::string body)
{
	std::string c_function_name = std::format("__c{}", std::to_string(function_counter_++));
	if (file_module_.c_file().empty())
	{
		file_module_.set_c_file(std::format("{}.c", mod_.getName().str()));
		std::ofstream c_file{ file_module_.c_file(), std::ios::binary };
		c_file.close();
	}
	std::ifstream c_file{ file_module_.c_file() };
	std::istringstream is{ body };
	std::string remains;
	std::string main;
	std::string line;
	do
	{
		std::getline(is, line);
		line = utils::remove_all('\t', line);
		if (line.empty())
			continue;
		if (utils::starts_with("#include", line))
			remains += std::format("{}\n", line);
		else
			main += std::format("{}\n", line);
	} while (is.good());

	std::string old_body;
	c_file >> old_body;
	c_file.close();

	std::ofstream oc_file{ file_module_.c_file(), std::ios::app };
	//                    
	oc_file << std::format("#ifndef __cplusplus\n{} {} void {}(void)\n{{\n{} }}\n#endif\n", remains, old_body, c_function_name, main);
	oc_file.close();
	auto* c_exec_type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), false);
	auto c_exec = mod_.getOrInsertFunction(c_function_name, c_exec_type);
	builder_.CreateCall(c_exec);
}
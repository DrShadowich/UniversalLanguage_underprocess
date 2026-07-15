#include <code_generator.h>
#include <dictionaries.h>
#include <function_name_info.h>
uint32_t inner_block = 0;
#define get_function_name(fn_str)	utils::get_name_without_type(parser::function_name_info::get_name(fn_str))
namespace ul::codegen
{
	code_generator::code_generator(llvm::LLVMContext& ctx, llvm::Module& module, std::vector<std::unique_ptr<stmt::statement>> AST) :
		ctx_{ ctx }, module_{ module }, builder_{ ctx }, AST_{ std::move(AST) }
	{
		names_table_.insert("int16", dictionaries::ul_llvm_type_table.at("int16")(ctx_));
		names_table_.insert("int32", dictionaries::ul_llvm_type_table.at("int32")(ctx_));
		names_table_.insert("int64", dictionaries::ul_llvm_type_table.at("int64")(ctx_));
		names_table_.insert("float", dictionaries::ul_llvm_type_table.at("float")(ctx_));
		names_table_.insert("double", dictionaries::ul_llvm_type_table.at("double")(ctx_));
		names_table_.insert("str", dictionaries::ul_llvm_type_table.at("str")(ctx_));
		names_table_.insert("char", dictionaries::ul_llvm_type_table.at("char")(ctx_));
		names_table_.insert("bool", dictionaries::ul_llvm_type_table.at("bool")(ctx_));
		names_table_.insert("ptr", dictionaries::ul_llvm_type_table.at("ptr")(ctx_));
		names_table_.insert("", dictionaries::ul_llvm_type_table.at("")(ctx_));
	}
	void code_generator::add_depth()
	{
		++names_table_;
	}
	void code_generator::sub_depth()
	{
		--names_table_;
	}
	llvm_aligned_type code_generator::get_aligned_type(const std::string& name)
	{
		llvm_aligned_type atype{};
		if (name.empty())
			CODE_GENERATOR_EXCEPTION("Type was empty string");
		std::string type_str = utils::get_type_from_name(name);
		if (!names_table_.contains_type(name))
			atype.type = dictionaries::ul_llvm_type_table.at(type_str)(ctx_);
		else CODE_GENERATOR_EXCEPTION(std::format("Type {} doesn\'t exsist", type_str));
		atype.align = dictionaries::ul_llvm_alignment_table.contains(type_str) ?
			llvm::Align(dictionaries::ul_llvm_alignment_table.at(type_str)) :
			llvm::Align(1);
		return atype;
	}
	void code_generator::generate_statements()
	{
		for (auto&& stmt : AST_)
		{
			generate_statement(std::move(stmt));
		}
	}
	llvm_union code_generator::generate_statement(stmt::statement_ptr stmt)
	{
		if (stmt == nullptr)
			CODE_GENERATOR_EXCEPTION("Statement was nullptr");
		auto* stmtp = stmt.get();
		if (auto* n = dynamic_cast<stmt::assignment_statement*>(stmtp))
		{
			llvm_union ret_variable;
			llvm::Value* rhs = generate_expression(std::move(n->rhs_value)).value.value;
			auto&& llvm_type = get_aligned_type(n->variable_name);
			if (!names_table_.contains_variable(n->variable_name))
			{
				llvm::AllocaInst* variable = builder_.CreateAlloca(llvm_type.type, nullptr, n->variable_name);
				builder_.CreateAlignedStore(rhs, variable, llvm_type.align);
				names_table_.insert(n->variable_name, variable);
				ret_variable.value = { llvm::cast<llvm::Value>(variable), false };
			}
			else
			{
				builder_.CreateAlignedStore(rhs, names_table_.get_variable(n->variable_name), llvm_type.align);
				ret_variable.value = { names_table_.get_variable(n->variable_name), false };
			}
			return ret_variable;
		}
		else if (auto* n = dynamic_cast<stmt::extern_function_declaration*>(stmtp))
		{
			llvm_union extern_func_variable;
			extern_func_variable = generate_expression(std::move(n->function_def));
			return extern_func_variable;
		}
		else if (auto* n = dynamic_cast<stmt::block_statement*>(stmtp))
		{
			++inner_block;
			add_depth();
			llvm::BasicBlock* old_block = builder_.GetInsertBlock();
			llvm::BasicBlock* just_block = llvm::BasicBlock::Create(ctx_, std::to_string(inner_block), old_block->getParent());
			llvm::BasicBlock* after_block = llvm::BasicBlock::Create(ctx_, std::to_string(inner_block) + " exit", old_block->getParent());
			builder_.CreateBr(just_block);
			builder_.SetInsertPoint(just_block);

			for (auto&& stmt : n->statements)
			{
				llvm_union last_operation = generate_statement(std::move(stmt));
			}
			builder_.CreateBr(after_block);
			builder_.SetInsertPoint(after_block);
			sub_depth();
		}
		else if (auto* n = dynamic_cast<stmt::expression_statement*>(stmtp))
		{
			return generate_expression(std::move(n->expression));
		}
		else if (auto* n = dynamic_cast<stmt::function_definition*>(stmtp))
		{
			llvm::Function* function = (generate_expression(std::move(n->function_def)).function_info);
			if (function->empty())
			{
				add_depth();
				for (auto&& it = function->arg_begin(); it != function->arg_end(); ++it)
				{
					names_table_.insert(it->getName().str(), it);
				}
				llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(ctx_, "entry", function);
				builder_.SetInsertPoint(entry_block);
				for (auto&& stmt : n->inner_stmt->statements)
				{
					llvm_union last_operation = generate_statement(std::move(stmt));
					if (last_operation.value.is_return_value)
					{
						builder_.CreateRet(last_operation.value.value);
					}
				}
				sub_depth();
			}
			else
				CODE_GENERATOR_EXCEPTION(std::format("{} is already defined and declared", n->function_def->function->name->short_name));
		}
		else
			CODE_GENERATOR_EXCEPTION("Unhandled statement type in code generator");
	}
	llvm_union code_generator::generate_expression(expr::expr_node_ptr expr)
	{
		if (expr == nullptr)
			CODE_GENERATOR_EXCEPTION("Expression was nullptr");
		auto* exprp = expr.get();
		if (auto* n = dynamic_cast<expr::binary_operator_node*>(exprp))
		{
			llvm_union ret_operator;
			llvm::Value* left = generate_expression(std::move(n->left)).value.value;
			llvm::Value* right = generate_expression(std::move(n->right)).value.value;
			llvm::Value* left_operand{ nullptr };
			llvm::Value* right_operand{ nullptr };
			if (left->getType()->isPointerTy() && !right->getType()->isPointerTy())
			{
				left_operand = builder_.CreateLoad(right->getType(), left, left->getName());
				right_operand = right;
			}
			else if (!left->getType()->isPointerTy() && right->getType()->isPointerTy())
			{
				right_operand = builder_.CreateLoad(left->getType(), right, right->getName());
				left_operand = left;

			}
			else
			{
				left_operand = left;
				right_operand = right;
			}
			switch (n->op.type)
			{
			case token::TYPE_TOKEN_TYPE::PLUS_OPERATOR:
				ret_operator.value = { builder_.CreateAdd(left_operand, right_operand, ""), false };
				return ret_operator;
			case token::TYPE_TOKEN_TYPE::MINUS_OPERATOR:
				ret_operator.value = { builder_.CreateSub(left_operand, right_operand, ""), false };
				return ret_operator;
			case token::TYPE_TOKEN_TYPE::SLASH_OPERATOR:
				ret_operator.value = { builder_.CreateSDiv(left_operand, right_operand, ""), false };
				return ret_operator;
			case token::TYPE_TOKEN_TYPE::STAR_OPERATOR:
				ret_operator.value = { builder_.CreateMul(left_operand, right_operand, ""), false };
				return ret_operator;
			default:
				CODE_GENERATOR_EXCEPTION("Unhandled binary operator type in code generator");
			}
		}
		else if (auto* n = dynamic_cast<expr::field_call_node*>(exprp))
		{

		}
		else if (auto* n = dynamic_cast<expr::type_variable_node*>(exprp))
		{
			llvm_union ret_type;
			ret_type.type = get_aligned_type(n->name).type;
			return ret_type;
		}
		// You need to delete vector*
		else if (auto* n = dynamic_cast<expr::function_parameters_node*>(exprp))
		{
			llvm_union ret_types;
			if (n->types.empty())
				return ret_types.param_list = new std::vector<llvm::Argument*>, ret_types;
			auto* vec_types = new std::vector<llvm::Argument*>;
			for (uint32_t i{ 0 }; i < n->types.size(); ++i)
			{
				vec_types->emplace_back(new llvm::Argument(generate_expression(std::move(n->types[i])).type, std::move(n->names[i])));
			}
			ret_types.param_list = std::move(vec_types);
			return ret_types;
		}
		// You need to delete vector*
		else if (auto* n = dynamic_cast<expr::function_arguments_node*>(exprp))
		{
			llvm_union ret_args;
			std::vector<llvm::Value*>* vec_args = new std::vector<llvm::Value*>;
			for (auto&& arg : n->args)
			{
				vec_args->push_back(generate_expression(std::move(arg)).value.value);
			}
			ret_args.args_list = std::move(vec_args);
			return ret_args;
		}
		else if (auto* n = dynamic_cast<expr::function_call_node*>(exprp))
		{
			llvm_union ret_call_instance;
			// Переделать
			if (n->function->is_extern)
			{
				std::string new_short_name = utils::get_name_without_type(n->function->name->short_name);
				n->function->name->full_name = new_short_name;
				n->function->name->short_name = std::move(new_short_name);
			}
			llvm::Function* calling_func = names_table_.get_function(n->function->name->full_name);
			auto calling_function =
				module_.getOrInsertFunction(n->function->name->full_name, calling_func->getFunctionType());
			//
			auto* args = generate_expression(std::move(n->arguments)).args_list;

			if (calling_function.getFunctionType()->getReturnType()->isVoidTy())
			{
				builder_.CreateCall(calling_function, *args);
				ret_call_instance.value = { nullptr, false };
			}
			else
			{
				llvm::Value* call_inst = builder_.CreateCall(calling_function, *args, std::format("ret_{}", n->function->name->short_name));
				ret_call_instance.value = { std::move(call_inst), false };
			}
			delete args;
			return ret_call_instance;
		}
		else if (auto* n = dynamic_cast<expr::function_node*>(exprp))
		{
			llvm_union ret_function;
			llvm::Function* function{};
			if (names_table_.contains_function(n->name->full_name))
			{
				function = names_table_.get_function(n->name->full_name);
			}
			else function = module_.getFunction(n->name->full_name);
			if (function == nullptr)
				CODE_GENERATOR_EXCEPTION(std::format("Undeclared function {}", n->name->short_name));
			llvm::Type* ftype = function->getFunctionType()->getPointerTo();
			llvm::Constant* func_ptr = llvm::ConstantExpr::getBitCast(function, ftype);
			ret_function.value = { std::move(func_ptr), false };
			return ret_function;
		}
		else if (auto* n = dynamic_cast<expr::number_literal_node*>(exprp))
		{
			llvm_union ret_value;
			int64_t literal = std::stoll(n->lexeme);
			llvm::Type* type = llvm::Type::getIntNTy(ctx_, n->bit_count);
			ret_value.value = { llvm::ConstantInt::get(type, literal), false };
			return ret_value;
		}
		else if (auto* n = dynamic_cast<expr::string_literal_node*>(exprp))
		{
			llvm_union ret_value;
			std::string_view ul_string_literal = n->lexeme;
			ul_string_literal.remove_prefix(1);
			ul_string_literal.remove_suffix(1);

			auto str_type = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx_), ul_string_literal.size() + 1);
			auto str_init = llvm::ConstantDataArray::getString(ctx_, ul_string_literal);
			auto literal_global = new llvm::GlobalVariable
			{
				module_, str_type, true, llvm::GlobalValue::LinkageTypes::PrivateLinkage, str_init
			};
			literal_global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

			llvm::Value* idx0 = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0);
			llvm::Value* indices[] = { idx0, idx0 };
			llvm::Value* string_ptr = builder_.CreateInBoundsGEP(str_type, literal_global, llvm::ArrayRef<llvm::Value*>(indices, 2));
			ret_value.value = { string_ptr, false };
			return ret_value;
		}
		else if (auto* n = dynamic_cast<expr::unary_operator_node*>(exprp))
		{

		}
		else if (auto* n = dynamic_cast<expr::return_value_node*>(exprp))
		{
			llvm_union ret_ret_value;
			llvm::Value* ret_value = generate_expression(std::move(n->value)).value.value;
			ret_ret_value.value = { std::move(ret_value), true };
			return ret_ret_value;
		}
		else if (auto* n = dynamic_cast<expr::variable_node*>(exprp))
		{
			llvm_union ret_var;
			auto&& llvm_type = get_aligned_type(n->name);
			if (!names_table_.contains_variable(n->name))
			{
				llvm::AllocaInst* variable = builder_.CreateAlloca(llvm_type.type, nullptr, n->name);
				names_table_.insert(n->name, variable);
				ret_var.value = { llvm::cast<llvm::Value>(variable), false };
			}
			else
			{
				llvm::Value* return_value{};

				if (auto* argument = llvm::dyn_cast<llvm::Argument>(names_table_.get_variable(n->name)))
				{
					return_value = builder_.CreateAlloca(llvm_type.type, nullptr);
					builder_.CreateStore(argument, return_value);
					names_table_.insert(n->name, return_value);
				}
				else
					return_value = names_table_.get_variable(n->name);

				ret_var.value = {
					llvm::cast<llvm::Value>(builder_.CreateAlignedLoad(llvm_type.type, names_table_.get_variable(n->name), llvm_type.align, n->name)),
					false
				};
			}
			return ret_var;
		}
		else if (auto* n = dynamic_cast<expr::function_definition_node*>(exprp))
		{
			llvm_union ret_function_definition;
			auto* vec_params = generate_expression(std::move(n->parameters)).param_list;
			std::vector<llvm::Type*> vec_types;
			for (auto&& param : *vec_params)
			{
				vec_types.push_back(param->getType());
			}
			llvm::FunctionType* function_type =
				llvm::FunctionType::get
				(
					names_table_.get_type(n->function_type->type_str),
					vec_types,
					n->va_args
				);
			if (n->function->name->short_name == "main_fn" || n->function->is_extern)
			{
				std::string new_short_name = utils::get_name_without_type(n->function->name->short_name);
				n->function->name->full_name = new_short_name;
				n->function->name->short_name = std::move(new_short_name);
			}
			ret_function_definition.function_info = llvm::cast<llvm::Function>(module_.getOrInsertFunction(n->function->name->full_name, function_type).getCallee());
			auto* function = llvm::cast<llvm::Function>(ret_function_definition.function_info);
			for (uint32_t i{ 0 }; i < function->arg_size(); ++i)
			{
				(function->arg_begin() + i)->setName(vec_params->at(i)->getName());
			}
			delete vec_params;
			names_table_.insert(n->function->name->full_name, function);
			return ret_function_definition;
		}
		else if (auto* n = dynamic_cast<expr::argument_node*>(exprp))
		{
			return generate_expression(std::move(n->value));
		}
		else
			CODE_GENERATOR_EXCEPTION("Unhandled expression type in code generator");
	}

	code_generator::~code_generator()
	{
	}
}
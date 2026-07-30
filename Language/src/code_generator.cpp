#include <code_generator.h>
#include <dictionaries.h>
#include <function_name_info.h>
#include <stmt_expr_cast.h>
#include <llvm+.h>

#define get_function_name(fn_str)	utils::get_name_without_type(parser::function_name_info::get_name(fn_str))

#define CODEGEN	ul::codegen::


CODEGEN	code_generator::code_generator(llvm::LLVMContext& ctx, llvm::Module& module, std::vector<std::unique_ptr<stmt::statement>> AST) :
	ctx_{ ctx }, module_{ module }, builder_{ ctx }, AST_{ std::move(AST) }, gctx_{ std::make_unique<generator_context>() }
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
void CODEGEN code_generator::add_depth()
{
	++names_table_;
}
void CODEGEN code_generator::sub_depth()
{
	--names_table_;
}
CODEGEN llvm_aligned_type CODEGEN code_generator::get_aligned_type(const std::string& name)
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

void CODEGEN code_generator::on_return_value(llvm_union ua)
{
	if (gctx_->is_return_value)
	{
		builder_.CreateRet(ua.value);
		gctx_->is_return_value = false;
	}
}

void CODEGEN code_generator::generate_statements()
{
	for (auto&& stmt : AST_)
	{
		generate_statement(std::move(stmt));
	}
}
CODEGEN llvm_union CODEGEN code_generator::generate_statement(stmt::statement_ptr stmt)
{
	if (stmt == nullptr)
		CODE_GENERATOR_EXCEPTION("Statement was nullptr");
	llvm_union universal_answer;
	auto* stmtp = stmt.get();

	if(auto* n = ul::dyn_cast<stmt::elif_statement>(stmtp))
	{
		auto [then_bb, else_bb, _] = make_then_blocks(ctx_, gctx_->if_statement_space->getParent(), "elif");
		auto* boolean_condition = generate_expression(std::move(n->condition)).value;
		builder_.CreateCondBr(boolean_condition, then_bb, else_bb);

		builder_.SetInsertPoint(then_bb);
		generate_statement(std::move(n->inner_stmt));
		builder_.CreateBr(gctx_->if_statement_space);

		builder_.SetInsertPoint(else_bb);
		if (n->next_cond_stmt)
			generate_statement(std::move(n->next_cond_stmt));
		else
			builder_.CreateBr(gctx_->if_statement_space);
		return universal_answer;
	}
	else if(auto* n = ul::dyn_cast<stmt::marker_statement>(stmtp))
	{
		stmt::marker_statement_ptr st{ ul::dyn_cast<stmt::marker_statement>(stmt.release()) };

	}
	else if(auto* n = ul::dyn_cast<stmt::for_loop_statement>(stmtp))
	{
		auto parent = builder_.GetInsertBlock()->getParent();
		auto* loop_init = llvm::BasicBlock::Create(ctx_, "for_init", parent);
		auto* loop_pred = llvm::BasicBlock::Create(ctx_, "for_pred", parent);
		auto* loop_dif = llvm::BasicBlock::Create(ctx_, "for_dif", parent);
		auto* loop_body = llvm::BasicBlock::Create(ctx_, "for_body", parent);
		auto* loop_exit = llvm::BasicBlock::Create(ctx_, "for_exit", parent);
		gctx_->loop_spaces.push(loop_exit);

		builder_.CreateBr(loop_init);
		builder_.SetInsertPoint(loop_init);
		for (auto&& def : n->value_definitions)
			generate_expression(std::move(def));

		builder_.CreateBr(loop_pred);
		builder_.SetInsertPoint(loop_pred);
		llvm::Value* condition =
			generate_expression(std::move(n->condition_expr)).value;
		builder_.CreateCondBr(condition, loop_body, loop_exit);

		builder_.SetInsertPoint(loop_dif);
		for (auto&& dif : n->value_interactions)
			generate_expression(std::move(dif));
		builder_.CreateBr(loop_pred);

		builder_.SetInsertPoint(loop_body);

		gctx_->is_named_statement = true;
		generate_statement(std::move(n->inner_stmt));
		gctx_->is_named_statement = false;

		builder_.CreateBr(loop_dif);
		builder_.SetInsertPoint(loop_exit);

	}
	else if(auto* n = ul::dyn_cast<stmt::while_loop_statement>(stmtp))
	{
		auto parent = builder_.GetInsertBlock()->getParent();
		auto* loop_pred = llvm::BasicBlock::Create(ctx_, "while_pred", parent);
		auto* loop_body = llvm::BasicBlock::Create(ctx_, "while_body", parent);
		auto* loop_exit = llvm::BasicBlock::Create(ctx_, "while_exit", parent);
		
		gctx_->loop_spaces.push(loop_exit);
		
		builder_.CreateBr(loop_pred);
		builder_.SetInsertPoint(loop_pred);
		llvm::Value* condition = 
			generate_expression(std::move(n->condition_expr)).value;
		builder_.CreateCondBr(condition, loop_body, loop_exit);

		builder_.SetInsertPoint(loop_body);

		gctx_->is_named_statement = true;
		generate_statement(std::move(n->inner_stmt));
		gctx_->is_named_statement = false;

		builder_.CreateBr(loop_pred);
		builder_.SetInsertPoint(loop_exit);
	}
	else if(auto* n = ul::dyn_cast<stmt::loop_statement>(stmtp))
	{
		auto parent = builder_.GetInsertBlock()->getParent();
		auto* loop_body = llvm::BasicBlock::Create(ctx_, "loop_body", parent);
		auto* loop_exit = llvm::BasicBlock::Create(ctx_, "loop_exit", parent);
		gctx_->loop_spaces.push(loop_exit);
		builder_.CreateBr(loop_body);
		builder_.SetInsertPoint(loop_body);
		
		gctx_->is_named_statement = true;
		generate_statement(std::move(n->inner_stmt));
		gctx_->is_named_statement = false;
		builder_.CreateBr(loop_body);
		builder_.SetInsertPoint(loop_exit);
	}
	else if(auto* n = ul::dyn_cast<stmt::else_statement>(stmtp))
	{
		generate_statement(std::move(n->inner_stmt));
		builder_.CreateBr(gctx_->if_statement_space);
		return universal_answer;
	}
	else if(auto* n = ul::dyn_cast<stmt::if_statement>(stmtp))
	{	
		gctx_->is_named_statement = true;
		auto parent = builder_.GetInsertBlock()->getParent();
		auto [then_bb, else_bb, merge_bb] = make_then_blocks(ctx_, parent, "if");
		gctx_->if_statement_space = std::move(merge_bb);

		auto* boolean_condition = generate_expression(std::move(n->condition)).value;
		builder_.CreateCondBr(boolean_condition, then_bb, else_bb);
		// true
		builder_.SetInsertPoint(then_bb);
		generate_statement(std::move(n->inner_stmt));
		builder_.CreateBr(gctx_->if_statement_space);
		
		// false
		builder_.SetInsertPoint(else_bb);
		if(n->next_cond_stmt)
		{
			generate_statement(std::move(n->next_cond_stmt));
		}
		else
		{
			builder_.CreateBr(gctx_->if_statement_space);
		}
		// end
		builder_.SetInsertPoint(gctx_->if_statement_space);
		gctx_->is_named_statement = false;
		gctx_->if_statement_space = nullptr;
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<stmt::extern_function_declaration>(stmtp))
	{
		universal_answer = generate_expression(std::move(n->function_def));
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<stmt::block_statement>(stmtp))
	{
		std::string in_bb_name = std::to_string(++gctx_->inner_block);
		std::string exit_bb_name = std::to_string(gctx_->inner_block) + " exit";
		add_depth();
		if (!gctx_->is_named_statement)
		{
			llvm::BasicBlock* old_block = builder_.GetInsertBlock();
			llvm::BasicBlock* just_block = llvm::BasicBlock::Create(ctx_, std::move(in_bb_name), old_block->getParent());
			llvm::BasicBlock* after_block = llvm::BasicBlock::Create(ctx_, std::move(exit_bb_name), old_block->getParent());
			builder_.CreateBr(just_block);
			builder_.SetInsertPoint(just_block);

			for (auto&& stmt : n->statements)
			{
				llvm_union last_operation = generate_statement(std::move(stmt));
			}
			builder_.CreateBr(after_block);
			builder_.SetInsertPoint(after_block);
		}
		else
		{
			for (auto&& stmt : n->statements)
			{
				llvm_union last_operation = generate_statement(std::move(stmt));
				on_return_value(std::move(last_operation));
			}
		}
		sub_depth();
		return universal_answer;
	}
	else if(auto* n = ul::dyn_cast<stmt::return_statement>(stmtp))
	{
		universal_answer = generate_expression(std::move(n->return_expression));
		gctx_->is_return_value = true;
		return universal_answer;
	}

	else if (auto* n = ul::dyn_cast<stmt::expression_statement>(stmtp))
	{
		return generate_expression(std::move(n->expression));
	}
	else if (auto* n = ul::dyn_cast<stmt::function_definition>(stmtp))
	{
		gctx_->is_named_statement = true;
		llvm::Function* function = (generate_expression(std::move(n->function_def)).function_info);
		if (not function->empty())
			CODE_GENERATOR_EXCEPTION(std::format("{} is already defined and declared", n->function_def->function->name->short_name));
		
		for (auto&& it = function->arg_begin(); it != function->arg_end(); ++it)
		{
			names_table_.insert(it->getName().str(), it);
		}

		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(ctx_, "entry", function);
		gctx_->current_function_space = entry_block;

		builder_.SetInsertPoint(entry_block);
		generate_statement(std::move(n->inner_stmt));
		gctx_->is_named_statement = false;
		return universal_answer;
	}
	else
		CODE_GENERATOR_EXCEPTION("Unhandled statement type in code generator");
}

std::pair<llvm::Value*, llvm::Value*> CODEGEN code_generator::get_binary_ops_operands(expr::expr_node_ptr first, expr::expr_node_ptr second)
{
	llvm::Value* left = generate_expression(std::move(first)).value;
	gctx_->left_buffer = left;
	llvm::Value* right = generate_expression(std::move(second)).value;
	llvm::Value* left_operand{ nullptr };
	llvm::Value* right_operand{ nullptr };
	if (any_is_pointer(left, right) == left)
	{
		left_operand = builder_.CreateLoad(typeof(right), left, nameof(left));
		right_operand = right;
	}
	else if (any_is_pointer(left, right) == right)
	{
		right_operand = builder_.CreateLoad(typeof(left), right, nameof(right));
		left_operand = left;
	}
	else
	{
		left_operand = left;
		right_operand = right;
	}
	gctx_->left_buffer = nullptr;
	return { left_operand, right_operand };
}

CODEGEN llvm_union CODEGEN code_generator::generate_expression(expr::expr_node_ptr expr)
{
	if (expr == nullptr)
		CODE_GENERATOR_EXCEPTION("Expression was nullptr");

	llvm_union universal_answer;
	auto* exprp = expr.get();

	if(auto* n = ul::dyn_cast<expr::logical_binary_operator_node>(exprp))
	{
		auto&& [left_operand, right_operand] = get_binary_ops_operands(std::move(n->left), std::move(n->right));

		switch (n->op)
		{

#define CreateCompare(IntegerCompare, FloatCompare, Lexeme)\
if (typeof(left_operand) != typeof(right_operand))\
			CODE_GENERATOR_EXCEPTION("Types are not equal");\
			if (both_is_integers(left_operand, right_operand))\
				universal_answer.value = (IntegerCompare)(left_operand, right_operand, (Lexeme));\
			else\
				universal_answer.value = (FloatCompare)(left_operand, right_operand, (Lexeme))


		case token::TID::LOGICAL_EQUAL_OPERATOR:
			CreateCompare(builder_.CreateICmpEQ, builder_.CreateFCmpOEQ, "==");
			return universal_answer;
		case token::TID::LOGICAL_GREATER_OPERATOR:
			CreateCompare(builder_.CreateICmpSGT, builder_.CreateFCmpOGT, ">");
			return universal_answer;
		case token::TID::LOGICAL_GREATER_OR_EQUAL_OPERATOR:
			CreateCompare(builder_.CreateICmpSGE, builder_.CreateFCmpOGE, ">=");
			return universal_answer;
		case token::TID::LOGICAL_LESS_OPERATOR:
			CreateCompare(builder_.CreateICmpSLT, builder_.CreateFCmpOLT, "<");
			return universal_answer;
		case token::TID::LOGICAL_LESS_OR_EQUAL_OPERATOR:
			CreateCompare(builder_.CreateICmpSLE, builder_.CreateFCmpOLE, "<=");
			return universal_answer;
		case token::TID::LOGICAL_NOT_EQUAL_OPERATOR:
			CreateCompare(builder_.CreateICmpNE, builder_.CreateFCmpONE, "!=");
			return universal_answer;
#undef CreateCompare
		case token::TID::LOGICAL_AND_OPERATOR:
		{
			auto* parent = builder_.GetInsertBlock()->getParent();
			auto* cond = builder_.CreateICmpEQ(left_operand, llvm::ConstantInt::get(left_operand->getType(), 1));
			auto [and_then_block, and_else_block, and_merge_block] = make_then_blocks(ctx_, parent, "and");

			builder_.CreateCondBr(cond, and_then_block, and_else_block);

			builder_.SetInsertPoint(and_then_block);
			llvm::Value* second_cond = builder_.CreateICmpEQ(right_operand, llvm::ConstantInt::get(right_operand->getType(), 1));
			builder_.CreateBr(and_merge_block);

			builder_.SetInsertPoint(and_else_block);
			builder_.CreateBr(and_merge_block);

			builder_.SetInsertPoint(and_merge_block);
			auto* phi = builder_.CreatePHI(llvm::Type::getInt1Ty(ctx_), 2);
			phi->addIncoming(second_cond, and_then_block);
			phi->addIncoming(llvm::ConstantInt::getFalse(ctx_), and_else_block);
			universal_answer.value = phi;
			return universal_answer;
		}
		case token::TID::LOGICAL_OR_OPERATOR:
		{
			auto* parent = builder_.GetInsertBlock()->getParent();
			auto* cond = builder_.CreateICmpEQ(left_operand, llvm::ConstantInt::get(left_operand->getType(), 1));
			
			auto [or_then_block, or_else_block, or_merge_block] = make_then_blocks(ctx_, parent, "or");

			builder_.CreateCondBr(cond, or_then_block, or_else_block);

			builder_.SetInsertPoint(or_then_block);
			builder_.CreateBr(or_merge_block);

			builder_.SetInsertPoint(or_else_block);
			llvm::Value* second_cond = builder_.CreateICmpEQ(right_operand, llvm::ConstantInt::get(right_operand->getType(), 1));
			builder_.CreateBr(or_merge_block);

			builder_.SetInsertPoint(or_merge_block);
			auto* phi = builder_.CreatePHI(llvm::Type::getInt1Ty(ctx_), 2);
			phi->addIncoming(llvm::ConstantInt::getTrue(ctx_), or_then_block);
			phi->addIncoming(second_cond, or_else_block);
			universal_answer.value = phi;
			return universal_answer;
		}
		default:
			CODE_GENERATOR_EXCEPTION("Unhandled logical operator type for logical binary expression");
		}
	}
	else if(auto* n = ul::dyn_cast<expr::unary_operator_node>(exprp))
	{
		llvm::Value* operand = generate_expression(std::move(n->child)).value;
		switch (n->op.type)
		{
		case token::TID::MINUS_OPERATOR:
			universal_answer.value = builder_.CreateNeg(operand);
			return universal_answer;
		case token::TID::PLUS_OPERATOR:
			universal_answer.value = operand;
			return universal_answer;
		case token::TID::LOGICAL_NOT_OPERATOR:
			universal_answer.value = builder_.CreateNot(operand);
			return universal_answer;
		default:
			CODE_GENERATOR_EXCEPTION("Unhandled unary operator type for basic unary expression");
		}
	}
	else if (auto* n = ul::dyn_cast<expr::break_node>(exprp))
	{
		if (gctx_->loop_spaces.empty())
			CODE_GENERATOR_EXCEPTION("Loop\'s previous space was empty");
		builder_.CreateBr(gctx_->loop_spaces.top());
		gctx_->loop_spaces.pop();
	}
	else if (auto* n = ul::dyn_cast<expr::break_node>(exprp))
	{
		builder_.CreateBr(builder_.GetInsertBlock());
	}
	else if(auto* n = ul::dyn_cast<expr::variable_additional_assignment_expr>(exprp))
	{

		std::string var_name = ul::dyn_cast<expr::variable_node>(n->left.get())->name;
		auto* var = generate_expression(std::move(n->left)).value;
		auto* var_ptr = names_table_.get_variable(var_name);
		gctx_->left_buffer = var;
		auto* value = generate_expression(std::move(n->right)).value;
		gctx_->left_buffer = nullptr;
		auto&& llvm_type = get_aligned_type(var_name);
		llvm::Value* operation;
		

		switch(n->op)
		{
		case token::TID::PLUS_ASSIGNMENT_OPERATOR:
			operation = builder_.CreateAdd(var, value, "+=");
			builder_.CreateAlignedStore(operation, var_ptr, llvm_type.align);
			break;
		case token::TID::MINUS_ASSIGNMENT_OPERATOR:
			operation = builder_.CreateSub(var, value, "-=");
			builder_.CreateAlignedStore(operation, var_ptr, llvm_type.align);
			break;
		case token::TID::SLASH_ASSIGNMENT_OPERATOR:
			operation = builder_.CreateSDiv(var, value, "/=");
			builder_.CreateAlignedStore(operation, var_ptr, llvm_type.align);
			break;
		case token::TID::STAR_ASSIGNMENT_OPERATOR:
			operation = builder_.CreateMul(var, value, "*=");
			builder_.CreateAlignedStore(operation, var_ptr, llvm_type.align);
			break;
		default:
			CODE_GENERATOR_EXCEPTION("Unhandled additional assignment binary operator type");
		}
		universal_answer.value = var;
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::variable_assignment_expr>(exprp))
	{
		auto* lhs = ul::dyn_cast<expr::variable_node>(n->left.get());
		llvm::Value* rhs = generate_expression(std::move(n->right)).value;
		auto&& llvm_type = get_aligned_type(lhs->name);
		if (!names_table_.contains_variable(lhs->name))
		{
			llvm::AllocaInst* variable = builder_.CreateAlloca(llvm_type.type, nullptr, lhs->name);
			builder_.CreateAlignedStore(rhs, variable, llvm_type.align);
			names_table_.insert(lhs->name, variable);
			universal_answer.value = llvm::cast<llvm::Value>(variable);
		}
		else
		{
			builder_.CreateAlignedStore(rhs, names_table_.get_variable(lhs->name), llvm_type.align);
			universal_answer.value = names_table_.get_variable(lhs->name);
		}
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::binary_operator_node>(exprp))
	{
		auto&& [left_operand, right_operand] = get_binary_ops_operands(std::move(n->left), std::move(n->right));
		switch (n->op)
		{
		case token::TID::PLUS_OPERATOR:
			universal_answer.value = builder_.CreateAdd(left_operand, right_operand, "+");
			return universal_answer;
		case token::TID::MINUS_OPERATOR:
			universal_answer.value = builder_.CreateSub(left_operand, right_operand, "-");
			return universal_answer;
		case token::TID::SLASH_OPERATOR:
			universal_answer.value = builder_.CreateSDiv(left_operand, right_operand, "/");
			return universal_answer;
		case token::TID::STAR_OPERATOR:
			universal_answer.value = builder_.CreateMul(left_operand, right_operand, "*");
			return universal_answer;
		default:
			CODE_GENERATOR_EXCEPTION("Unhandled binary operator type for basic binary expression");
		}
	}
	else if (auto* n = ul::dyn_cast<expr::field_call_node>(exprp))
	{

	}
	else if (auto* n = ul::dyn_cast<expr::type_variable_node>(exprp))
	{
		universal_answer.type = get_aligned_type(n->name).type;
		return universal_answer;
	}
	// You need to delete vector*
	else if (auto* n = ul::dyn_cast<expr::function_parameters_node>(exprp))
	{
		gctx_->parameters_buffer.clear();
		if (n->types.empty())
			return universal_answer;
		std::vector<llvm::Argument*> vec_types;
		for (size_t i{ 0 }; i < n->types.size(); ++i)
		{
			vec_types.emplace_back(new llvm::Argument(generate_expression(std::move(n->types[i])).type, std::move(n->names[i])));
		}
		gctx_->parameters_buffer = std::move(vec_types);
		return universal_answer;
	}
	// You need to delete vector*
	else if (auto* n = ul::dyn_cast<expr::function_arguments_node>(exprp))
	{
		gctx_->arguments_buffer.clear();
		std::vector<llvm::Value*> vec_args;
		for (auto&& arg : n->args)
		{
			vec_args.emplace_back(generate_expression(std::move(arg)).value);
		}
		gctx_->arguments_buffer = std::move(vec_args);
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::function_call_node>(exprp))
	{
		// Переделать
		if (n->function->is_extern)
		{
			std::string new_short_name = utils::get_name_without_type(n->function->name->short_name);
			n->function->name->full_name = new_short_name;
			n->function->name->short_name = std::move(new_short_name);
		}
		llvm::Function* calling_func = names_table_.get_function(n->function->name->full_name);
		auto calling_function =
			module_.getOrInsertFunction(n->function->name->full_name, typeof(calling_func));
		
		// get arguments to gctx_
		generate_expression(std::move(n->arguments));

		if (typeof(calling_function)->getReturnType()->isVoidTy())
		{
			builder_.CreateCall(calling_function, gctx_->arguments_buffer);
		}
		else
		{
			llvm::Value* call_inst = builder_.CreateCall(calling_function, gctx_->arguments_buffer, std::format("ret_{}", n->function->name->short_name));
			universal_answer.value = std::move(call_inst);
		}
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::function_node>(exprp))
	{
		llvm::Function* function{};
		if (names_table_.contains_function(n->name->full_name))
		{
			function = names_table_.get_function(n->name->full_name);
		}
		else function = module_.getFunction(n->name->full_name);
		if (function == nullptr)
			CODE_GENERATOR_EXCEPTION(std::format("Undeclared function {}", n->name->short_name));
		llvm::Type* ftype = typeof(function)->getPointerTo();
		llvm::Constant* func_ptr = llvm::ConstantExpr::getBitCast(function, ftype);
		universal_answer.value = std::move(func_ptr);
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::number_literal_node>(exprp))
	{
		int64_t literal = std::stoll(n->lexeme);
		if (gctx_->if_statement_space)
			universal_answer.value = literal ? llvm::ConstantInt::getTrue(ctx_) : llvm::ConstantInt::getFalse(ctx_);
		else
		{
			llvm::Type* type = gctx_->left_buffer ? 
				typeof(gctx_->left_buffer) :
				llvm::Type::getIntNTy(ctx_, n->bit_count);
			universal_answer.value = llvm::ConstantInt::get(type, literal);
		}
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::string_literal_node>(exprp))
	{
		auto str_type = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx_), n->literal.size() + 1);
		auto str_init = llvm::ConstantDataArray::getString(ctx_, n->literal);
		auto literal_global = new llvm::GlobalVariable
		{
			module_, str_type, true, llvm::GlobalValue::LinkageTypes::PrivateLinkage, str_init
		};
		literal_global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

		llvm::Value* idx0 = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx_), 0);
		llvm::Value* indices[] = { idx0, idx0 };
		llvm::Value* string_ptr = builder_.CreateInBoundsGEP(str_type, literal_global, llvm::ArrayRef<llvm::Value*>(indices, 2));
		universal_answer.value = string_ptr;
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::variable_reference_node>(exprp))
	{
		if (!names_table_.contains_variable(n->variable->name))
			CODE_GENERATOR_EXCEPTION(std::format("Name {} doesn\'t exsist", n->variable->name));
		// Just get pointer
		universal_answer.value = names_table_.get_variable(n->variable->name);
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::variable_node>(exprp))
	{
		auto&& llvm_type = get_aligned_type(n->name);
		if (!names_table_.contains_variable(n->name))
		{
			llvm::AllocaInst* variable = builder_.CreateAlloca(llvm_type.type, nullptr, n->name);
			names_table_.insert(n->name, variable);
			universal_answer.value = llvm::cast<llvm::Value>(variable);
		}
		else
		{
			if (auto* argument = llvm::dyn_cast<llvm::Argument>(names_table_.get_variable(n->name)))
			{
				llvm::Value* return_value{};
				return_value = builder_.CreateAlloca(llvm_type.type, nullptr);
				builder_.CreateStore(argument, return_value);
				names_table_.insert(n->name, return_value);
			}
			universal_answer.value =
				llvm::cast<llvm::Value>
				(builder_.CreateAlignedLoad(llvm_type.type, names_table_.get_variable(n->name), llvm_type.align, n->name));
		}
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::function_definition_node>(exprp))
	{
		bool have_va_args = n->parameters->va_args;

		// get parameters to gctx_
		generate_expression(std::move(n->parameters));
		
		std::vector<llvm::Type*> vec_types;
		for (auto&& param : gctx_->parameters_buffer)
		{
			vec_types.push_back(typeof(param));
		}
		llvm::FunctionType* function_type =
			llvm::FunctionType::get
			(
				names_table_.get_type(n->function_type->type_str),
				vec_types,
				have_va_args
			);
		if (n->function->name->short_name == "main_fn" || n->function->is_extern)
		{
			std::string new_short_name = utils::get_name_without_type(n->function->name->short_name);
			n->function->name->full_name = new_short_name;
			n->function->name->short_name = std::move(new_short_name);
		}
		universal_answer.function_info = llvm::cast<llvm::Function>(module_.getOrInsertFunction(n->function->name->full_name, function_type).getCallee());
		auto* function = llvm::cast<llvm::Function>(universal_answer.function_info);
		for (uint32_t i{ 0 }; i < function->arg_size(); ++i)
		{
			(function->arg_begin() + i)->setName(nameof(gctx_->parameters_buffer.at(i)));
		}
		names_table_.insert(n->function->name->full_name, function);
		return universal_answer;
	}
	else if (auto* n = ul::dyn_cast<expr::argument_node>(exprp))
	{
		universal_answer = generate_expression(std::move(n->value));
		if(typeof(universal_answer.value)->isIntegerTy(1))
		{
			universal_answer.value = builder_.CreateZExt(universal_answer.value, llvm::Type::getInt32Ty(ctx_), "bool_to_int");
		}
		return universal_answer;
	}
	else
		CODE_GENERATOR_EXCEPTION("Unhandled expression type in code generator");
}

CODEGEN code_generator::~code_generator()
{
}
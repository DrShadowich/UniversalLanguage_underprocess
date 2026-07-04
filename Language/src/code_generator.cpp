#include <code_generator.h>
#include <dictionaries.h>
#include <simple_variable.h>
namespace ul::codegen
{
	code_generator::code_generator(llvm::LLVMContext& ctx, llvm::Module& module, std::vector<std::unique_ptr<stmt::statement>> AST) :
		ctx_{ ctx }, module_{ module }, builder_{ ctx }, AST_{ std::move(AST) }
	{}
	void code_generator::generate_statements()
	{
		for(auto&& stmt : AST_)
		{
			auto* stmtp = stmt.get();
			if(auto* n = dynamic_cast<stmt::assignment_statement*>(stmtp))
			{

			}
			else if(auto* n = dynamic_cast<stmt::block_statement*>(stmtp))
			{

			}
			else if(auto* n = dynamic_cast<stmt::expression_statement*>(stmtp))
			{

			}
			else if(auto* n = dynamic_cast<stmt::function_definition*>(stmtp))
			{

			}
			else if (auto* n = dynamic_cast<stmt::statement*>(stmtp))
			{
			}
			else
			{
				CODE_GENERATOR_EXCEPTION("Unhandled statement type in code generator");
			}
		}
	}
	void* code_generator::generate_expression(expr::expr_node_ptr expr)
	{
		auto* exprp = expr.get();
		if(auto* n = dynamic_cast<expr::binary_operator_node*>(exprp))
		{
			switch(n->op.type)
			{
				case token::TYPE_TOKEN_TYPE::PLUS_OPERATOR:
					return builder_.CreateAdd((llvm::Value*)generate_expression(std::move(n->left)), (llvm::Value*)generate_expression(std::move(n->right)));
				case token::TYPE_TOKEN_TYPE::MINUS_OPERATOR:
					return builder_.CreateSub((llvm::Value*)generate_expression(std::move(n->left)), (llvm::Value*)generate_expression(std::move(n->right)));
				case token::TYPE_TOKEN_TYPE::SLASH_OPERATOR:
					return builder_.CreateSDiv((llvm::Value*)generate_expression(std::move(n->left)), (llvm::Value*)generate_expression(std::move(n->right)));
				case token::TYPE_TOKEN_TYPE::STAR_OPERATOR:
					return builder_.CreateMul((llvm::Value*)generate_expression(std::move(n->left)), (llvm::Value*)generate_expression(std::move(n->right)));
				default:
					CODE_GENERATOR_EXCEPTION("Unhandled binary operator type in code generator");
			}
		}
		else if(auto* n = dynamic_cast<expr::field_call_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::function_arguments_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::function_call_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::function_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::newline_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::number_literal_node*>(exprp))
		{
			int64_t literal = std::stoll(n->lexeme);
			std::string type = std::move(utils::get_type_from_name(n->lexeme));
			return dictionaries::ul_llvm_constant_int_table.at(type)(builder_, literal);
		}
		else if(auto* n = dynamic_cast<expr::string_literal_node*>(exprp))
		{
			llvm::Value* str = builder_.CreateGlobalStringPtr(n->lexeme);
			return str;
		}
		else if(auto* n = dynamic_cast<expr::unary_operator_node*>(exprp))
		{

		}
		else if(auto* n = dynamic_cast<expr::variable_node*>(exprp))
		{

		}
		else
		{
			CODE_GENERATOR_EXCEPTION("Unhandled expression type in code generator");
		}
	}

}
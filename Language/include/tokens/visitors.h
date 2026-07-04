#pragma once
#include <string>
#include <functional>
#include <expression_info.h>
#include <statement_info.h>
#include <format>
#include <core_exceptions.h>
struct visitor
{
	virtual ~visitor() = default;
};

template<typename NodeType>
struct make_ptr_visitor : public visitor
{
	virtual std::unique_ptr<NodeType> operator()(NodeType&) = 0;
};

template<typename NodeType>
struct output_visitor : public visitor
{
	virtual std::string visit(NodeType&) = 0;
};



struct expr_output_visitor : public output_visitor<ul::expr::expr_node>
{
	virtual std::string visit(ul::expr::expr_node& ep) override
	{
		using namespace ul::expr;
		if (auto* n = dynamic_cast<binary_operator_node*>(&ep))
		{
			return std::format("{} {} {}", visit(*n->left), n->op.lexeme, visit(*n->right));
		}
		else if (auto* n = dynamic_cast<unary_operator_node*>(&ep))
		{
			return std::format("{} {}", n->op.lexeme, visit(*n->child));
		}
		else if (auto* n = dynamic_cast<number_literal_node*>(&ep))
		{
			return std::format("({})", n->lexeme);
		}
		else if (auto* n = dynamic_cast<string_litral_node*>(&ep))
		{
			return std::format("{}", n->lexeme);
		}
		else if (auto* n = dynamic_cast<newline_node*>(&ep))
		{
			return "\\n";
		}
		else if (auto* n = dynamic_cast<named_node*>(&ep))
		{
			return std::format("{}", n->name);
		}
		else if (auto* n = dynamic_cast<function_call_node*>(&ep))
		{
			return std::format("{}({})", visit(*n->function), visit(*n->arguments));
		}
		else if (auto* n = dynamic_cast<function_arguments_node*>(&ep))
		{
			std::string args;
			for (size_t size{ 0 }; size < n->args.size(); ++size)
			{
				args += n->args.size() - size == 1 ? std::format("{}", visit(*n->args[size])) : std::format("{}, ", visit(*n->args[size]));
			}
			return args;
		}
		else if (auto* n = dynamic_cast<field_call_node*>(&ep))
		{
			return std::format("{}.{}", visit(*n->parent), visit(*n->child), visit(*n->next_field_call));
		}
		else if (auto* n = dynamic_cast<marker_node*>(&ep))
		{
			return std::format("\n{}\n", n->marker_expr.lexeme);
		}
		VISITOR_EXCEPTION("Unhandled expression node type in expression output visitor");
	}
};


struct stmt_output_visitor : public output_visitor<ul::stmt::statement>
{
	virtual std::string visit(ul::stmt::statement& st) override 
	{
		using namespace ul::expr;
		using namespace ul::stmt;
		expr_output_visitor ev;
		if (auto* n = dynamic_cast<expression_statement*>(&st))
		{
			return std::format("{}", ev.visit(*n->expression));
		}
		else if (auto* n = dynamic_cast<assignment_statement*>(&st))
		{
			return std::format("{} = {}", n->variable_name, ev.visit(*n->rhs_value));
		}
		else if (auto* n = dynamic_cast<block_statement*>(&st))
		{
			std::string res = "{ ";
			for (auto&& stmt : n->statements)
			{
				res += visit(*stmt) + ';';
			}
			res += " }";
			return res;
		}
		else if(auto* n = dynamic_cast<function_definition*>(&st))
		{
			return std::format("{}\n{}", ev.visit(*n->function_def), visit(*n->inner_stmt));
		}
		VISITOR_EXCEPTION("Unhandled statement node type in statement output visitor");
	}
};

#include <stmt_expr_cast.h>
namespace ul
{
	stmt::expression_statement_ptr dyn_cast(expr::expr_node_ptr rhs)
	{
		return std::make_unique<stmt::expression_statement>(std::move(rhs));
	}
}
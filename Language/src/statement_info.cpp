#include <statement_info.h>
namespace ul::stmt
{
	else_statement_ptr reverse_if_statements_list(else_statement_ptr root)
	{
        else_statement_ptr prev = nullptr;
        else_statement_ptr curr = std::move(root);

        while (curr) {
            else_statement* raw_curr{ curr.release() };
            else_statement* next_node{ raw_curr->next_cond_stmt.release() };

            raw_curr->next_cond_stmt.reset(prev.release()); 

            prev.reset(raw_curr);
            curr.reset(next_node);
        }
        return prev;
	}
}
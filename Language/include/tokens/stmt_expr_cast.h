#pragma once
#include <expression_info.h>
#include <statement_info.h>
namespace ul
{
	template<typename T, typename V>
	T* dyn_cast(V ref_value_uptr)
	{
		if constexpr (std::is_pointer_v<V>)
		{
			if (T* n = dynamic_cast<T*>(ref_value_uptr))
				return n;
		}
		else if(not std::is_pointer_v<V>)
		{
			if (T* n = dynamic_cast<T*>(&ref_value_uptr))
				return n;
		}
		else
			return nullptr;
	}

	stmt::expression_statement_ptr dyn_cast(expr::expr_node_ptr rhs);
}
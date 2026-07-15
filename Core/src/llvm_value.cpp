#include <llvm_value.h>
namespace ul::types
{
#if 0
	llvm_value::llvm_value(const llvm_value& rhs) :
		ptr{ std::make_unique<llvm::Value>(*rhs.ptr) },
		var_depth{ rhs.var_depth }
	{}
	llvm_value::llvm_value(llvm_value&& rhs) noexcept :
		ptr{ std::move(rhs.ptr) },
		var_depth{ std::move(rhs.var_depth) }
	{}
	llvm_value& llvm_value::operator=(const llvm_value& rhs)
	{
		this->ptr = std::make_unique<llvm::Value>(*rhs.ptr);
		this->var_depth = rhs.var_depth;
		return *this;
	}
	llvm_value& llvm_value::operator=(llvm_value&& rhs) noexcept
	{
		this->ptr = std::move(rhs.ptr);
		this->var_depth = std::move(rhs.var_depth);
		return *this;
	}
#endif
}
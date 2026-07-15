#include <llvm_libs.h>
#include <memory>
namespace ul::types
{
#if 0
	struct l1lvm_value
	{
		// Ptr to IRBuilder's value
		std::unique_ptr<llvm::Value> ptr;
		// Depth preserved for name_table.h
		uint32_t var_depth;
		llvm_value(const llvm_value& rhs);
		llvm_value(llvm_value&& rhs) noexcept;
		llvm_value& operator=(const llvm_value& rhs);
		llvm_value& operator=(llvm_value&& rhs) noexcept;
	};
#endif
}
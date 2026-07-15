#include <name_table.h>
namespace ul::utils
{
	name_table::name_table() :
		map_function_{},
		map_type_{},
		map_value_{},
		cur_depth_{ 0 }
	{}

	name_table::~name_table()
	{
		map_function_.clear();
		map_type_.clear();
		map_value_.clear();
	}

	void name_table::insert(const std::string& var_name, llvm::Value* value)
	{
		map_value_[var_name] = map_value{ value, cur_depth_ };
	}
	void name_table::insert(const std::string& type_name, llvm::Type* type)
	{
		map_type_[type_name] = map_type{ type, cur_depth_ };
	}
	void name_table::insert(const std::string& func_name, llvm::Function* function)
	{
		map_function_[func_name] = map_function{ function, cur_depth_ };
	}

	bool name_table::contains_variable(const std::string& name)
	{
		return map_value_.contains(name);
	}
	bool name_table::contains_type(const std::string& name)
	{
		return map_type_.contains(name);
	}
	bool name_table::contains_function(const std::string& name)
	{
		return map_function_.contains(name);
	}
	void name_table::update_namespace(uint32_t new_depth)
	{
		for (auto it = map_type_.begin(); it != map_type_.end();)
		{
			if (it->second.second > new_depth) {
				it = map_type_.erase(it);
			}
			else ++it;
		}
		for (auto it = map_function_.begin(); it != map_function_.end();)
		{
			if (it->second.second > new_depth) {
				it = map_function_.erase(it);
			}
			else ++it;
		}
		for (auto it = map_value_.begin(); it != map_value_.end();)
		{
			if (it->second.second > new_depth) {
				it = map_value_.erase(it);
			}
			else ++it;
		}
		cur_depth_ = new_depth;
	}

	llvm::Value* name_table::get_variable(const std::string& name)
	{
		return map_value_[name].first;
	}
	llvm::Type* name_table::get_type(const std::string& name)
	{
		return map_type_[name].first;
	}
	llvm::Function* name_table::get_function(const std::string& name)
	{
		return map_function_[name].first;
	}

	void name_table::operator++()
	{
		update_namespace(cur_depth_ + 1);
	}
	void name_table::operator--()
	{
		if (static_cast<int64_t>(cur_depth_ - 1) < 0)
			NAMES_EXCEPTION("Current depth lesser then 0");
		else update_namespace(cur_depth_ - 1);
	}
}
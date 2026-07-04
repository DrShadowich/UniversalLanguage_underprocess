#pragma once
#include <unordered_map>
#include <string>
#include <set>
#include <ranges>
#include <algorithm>

namespace utils
{
	struct name_table
	{
	private:
		uint32_t depth_;
		std::unordered_map<std::string, uint32_t> name_map_;
	public:
		void update_namespace(uint32_t depth)
		{
			depth_ = depth;
			for(auto&& it = name_map_.begin(); it != name_map_.end();)
			{
				if (it->second > depth_)
					it = name_map_.erase(it);
				else
					++it;
			}
		}
	};
}
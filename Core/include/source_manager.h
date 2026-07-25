#pragma once
#include <source_location_ul.h>
#include <string>

namespace ul::source
{
	class source_manager
	{
	private:
		std::string information_copy;
	public:
		source_manager();
		explicit source_manager(std::string information);
		std::pair<uint64_t, uint64_t> get_line_and_column(uint64_t offset);
	};
}
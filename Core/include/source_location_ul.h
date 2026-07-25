#pragma once
#include <cstdint>

#define SOURCE ul::source::

namespace ul::source
{
	struct source_location
	{
		uint64_t offset;
	};
}
#pragma once

#include <vector>

namespace wpl
{
	namespace tests
	{
		template <typename T, size_t n>
		inline std::vector<T> mkvector(T (&data)[n])
		{	return std::vector<T>(data, data + n);	}
	}
}

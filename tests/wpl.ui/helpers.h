#pragma once

#include <vector>

namespace wpl
{
	namespace tests
	{
		template <typename T, size_t n>
		inline T *begin(T (&container_)[n])
		{	return container_;	}

		template <typename Container>
		inline typename Container::iterator begin(Container &container_)
		{	return container_.begin();	}

		template <typename T, size_t n>
		inline T *end(T (&container_)[n])
		{	return container_ + n;	}

		template <typename Container>
		inline typename Container::iterator end(Container &container_)
		{	return container_.end();	}

		template <typename T, size_t n>
		inline std::vector<T> mkvector(T (&data)[n])
		{	return std::vector<T>(data, data + n);	}
	}
}

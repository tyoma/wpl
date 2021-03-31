//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "models.h"

#include <limits>

namespace wpl
{
	template <typename BaseT>
	class group_headers_model : public BaseT, noncopyable
	{
	public:
		typedef typename BaseT::index_type index_type;
		typedef std::pair<index_type /*start*/, index_type /*end*/> range_type;

	public:
		group_headers_model(columns_model &underlying);

		virtual range_type get_group(index_type index) const throw() = 0;

		virtual index_type get_count() const throw() override;
		virtual void get_value(index_type index, short &value) const override;

	protected:
		void update_mapping(index_type count);

	private:
		columns_model &_underlying;
		slot_connection _connection;
		index_type _count;
		std::vector<range_type> _ranges;
	};



	template <typename BaseT>
	inline group_headers_model<BaseT>::group_headers_model(columns_model &underlying)
		: _underlying(underlying), _count(0)
	{
		_connection = _underlying.invalidate += [this] (index_type index) {
			using namespace std;

			if (this->npos() != index)
			{
				auto i = upper_bound(_ranges.begin(), _ranges.end(),
					make_pair(index, numeric_limits<columns_model::index_type>::max()));

				if (_ranges.begin() != i)
					i--;
				if (_ranges.end() != i && !(i->first <= index && index < i->second))
					return;
				index = distance(_ranges.begin(), i);
			}
			this->invalidate(index);
		};
	}

	template <typename BaseT>
	inline typename group_headers_model<BaseT>::index_type group_headers_model<BaseT>::get_count() const throw()
	{	return _count;	}

	template <typename BaseT>
	inline void group_headers_model<BaseT>::get_value(index_type index, short &value) const
	{
		value = 0;
		for (auto r = get_group(index); r.first != r.second; r.first++)
		{
			short v;

			_underlying.get_value(r.first, v);
			value += v;
		}
	}

	template <typename BaseT>
	inline void group_headers_model<BaseT>::update_mapping(index_type count)
	{
		_ranges.resize(count);
		for (auto i = index_type(); i != count; ++i)
			_ranges[i] = get_group(i);
		_count = count;
		this->invalidate(this->npos());
	}
}

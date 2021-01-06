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

namespace wpl
{
	template <typename BaseT>
	class on_focus_invalidate : public BaseT
	{
	public:
		on_focus_invalidate();

	public:
		bool has_focus;

	protected:
		virtual void got_focus() override;
		virtual void lost_focus() override;
	};



	template <typename BaseT>
	inline on_focus_invalidate<BaseT>::on_focus_invalidate()\
		: has_focus(false)
	{	}

	template <typename BaseT>
	inline void on_focus_invalidate<BaseT>::got_focus()
	{
		has_focus = true;
		this->invalidate(nullptr);
	}

	template <typename BaseT>
	inline void on_focus_invalidate<BaseT>::lost_focus()
	{
		has_focus = false;
		this->invalidate(nullptr);
	}
}

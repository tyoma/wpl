//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../container.h"
#include "../layout.h"

typedef struct HWND__ *HWND;

namespace wpl
{
	namespace ui
	{
		typedef std::pair<widget_ptr, HWND> widget_native;

		class native_container : public container, noncopyable
		{
		public:
			native_container(HWND parent);

			void reposition(void *&hdwp, const layout_manager::position &new_position);

		private:
			struct child;
			typedef std::vector<child> children_container;

		private:
			virtual std::shared_ptr<widget> create_widget(const std::wstring &type, const std::wstring &id);
			virtual std::shared_ptr<container> create_container(const std::wstring &id);

		private:
			const HWND _parent;
			children_container _children;
		};

		struct native_container::child
		{
			widget_ptr widget;
			HWND hwnd;
			std::shared_ptr<native_container> container;
		};



		inline native_container::native_container(HWND parent)
			: _parent(parent)
		{	}
	}
}

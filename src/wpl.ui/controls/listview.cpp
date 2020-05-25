//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/ui/controls/listview.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace controls
		{
			void listview_core::draw(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/) const
			{	}

			void listview_core::resize(unsigned /*cx*/, unsigned /*cy*/, positioned_native_views &/*native_views*/)
			{	}

			void listview_core::set_columns_model(shared_ptr<columns_model> /*cmodel*/)
			{	}

			void listview_core::set_model(shared_ptr<table_model> /*model*/)
			{	}

			void listview_core::adjust_column_widths()
			{	}

			void listview_core::select(index_type /*item*/, bool /*reset_previous*/)
			{	}

			void listview_core::clear_selection()
			{	}

			void listview_core::ensure_visible(index_type /*item*/)
			{	}

			void listview_core::draw_subitem_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
				const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/, index_type /*subitem*/)
			{	}

			void listview_core::draw_item(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
				const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/)
			{	}
		}
	}
}

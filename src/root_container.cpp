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

#include <wpl/root_container.h>

#include <agge/figures.h>
#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <wpl/cursor.h>
#include <wpl/stylesheet.h>

using namespace agge;
using namespace std;

namespace wpl
{
	root_container::root_container(shared_ptr<cursor_manager> cursor_manager_)
		: _cursor_manager(cursor_manager_)
	{	}

	void root_container::apply_styles(const stylesheet &stylesheet_)
	{
		_background = stylesheet_.get_color("background");
		invalidate(nullptr);
	}

	void root_container::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
	{
		typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

		agge::add_path(*rasterizer_, agge::rectangle(0.0f, 0.0f, _size.w, _size.h));
		ctx(rasterizer_, blender_t(_background), winding<>());
		container::draw(ctx, rasterizer_);
	}

	void root_container::resize(unsigned cx, unsigned cy, positioned_native_views &native_views)
	{
		_size.w = static_cast<real_t>(cx), _size.h = static_cast<real_t>(cy);
		container::resize(cx, cy, native_views);
	}

	void root_container::mouse_enter()
	{	_cursor_manager->push(_cursor_manager->get(cursor_manager::arrow));	}

	void root_container::mouse_leave()
	{
		container::mouse_leave();
		_cursor_manager->pop();
	}
}

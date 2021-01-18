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

#include <wpl/win32/visual_router.h>

#include <agge/math.h>
#include <wpl/helpers.h>
#include <wpl/win32/helpers.h>

#pragma warning(disable: 4355)

using namespace agge;
using namespace std;

namespace wpl
{
	namespace win32
	{
		visual_router::visual_router(HWND hwnd, const vector<placed_view> &views, const form_context &context)
			: _hwnd(hwnd), _underlying(views, *this), _rasterizer(new gcontext::rasterizer_type), _context(context),
				_offset(zero())
		{	}

		void visual_router::reload_views()
		{	_underlying.reload_views();	}

		void visual_router::set_offset(const agge_vector<int> &offset)
		{	_offset = offset;	}

		bool visual_router::handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM /*wparam*/, LPARAM /*lparam*/)
		{
			switch (message)
			{
			default:
				return false;

			case WM_ERASEBKGND:
				result = TRUE;
				break;

			case WM_PAINT:
				helpers::paint_sequence ps(hwnd);
				auto &backbuffer = *_context.backbuffer;
				auto offset = create_vector<int>(ps.rcPaint.left, ps.rcPaint.top);
				gcontext ctx(backbuffer, *_context.renderer, *_context.text_engine, offset += _offset,
					create_rect<int>(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom));

				backbuffer.resize(ps.width(), ps.height());
				_rasterizer->reset();
				_underlying.draw(ctx, _rasterizer);
				backbuffer.blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.width(), ps.height());
				result = 0;
				break;
			}
			return true;
		}

		void visual_router::invalidate(const rect_i &area)
		{
			auto area2 = area;

			wpl::offset(area2, -_offset.dx, -_offset.dy);

			RECT rc = {	area2.x1, area2.y1, area2.x2, area2.y2	};

			::InvalidateRect(_hwnd, &rc, FALSE);
		}
	}
}

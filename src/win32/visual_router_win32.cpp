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

#include <wpl/win32/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace win32
	{
		visual_router::visual_router(const vector<placed_view> &views, visual_router_host &host,
				const form_context &context)
			: wpl::visual_router(views, host), _rasterizer(new gcontext::rasterizer_type), _context(context)
		{	}

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
				const vector_i offset = { ps.rcPaint.left, ps.rcPaint.top };
				gcontext ctx(backbuffer, *_context.renderer, *_context.text_engine, offset,
					create_rect<int>(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom));

				backbuffer.resize(ps.width(), ps.height());
				_rasterizer->reset();
				draw(ctx, _rasterizer);
				backbuffer.blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.width(), ps.height());
				result = 0;
				break;
			}
			return true;
		}
	}
}

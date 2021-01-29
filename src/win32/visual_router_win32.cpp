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
#include <wpl/misc/statistics_view.h>
#include <wpl/win32/helpers.h>

#pragma warning(disable: 4355)

using namespace agge;
using namespace std;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			LARGE_INTEGER c_tmp;
			float c_counter_period = 1000.0f / static_cast<float>(::QueryPerformanceFrequency(&c_tmp), c_tmp.QuadPart);

			float stopwatch(LARGE_INTEGER &previous)
			{
				LARGE_INTEGER current;

				::QueryPerformanceCounter(&current);
				swap(previous, current);
				return c_counter_period * static_cast<float>(previous.QuadPart - current.QuadPart);
			}

			void invalidate_window(HWND hwnd, const rect_i &rc_)
			{
				RECT rc = {	rc_.x1, rc_.y1, rc_.x2, rc_.y2	};

				::InvalidateRect(hwnd, &rc, FALSE);
			}
		}

		visual_router::visual_router(HWND hwnd, const vector<placed_view> &views, const form_context &context)
			: _hwnd(hwnd), _underlying(views, *this), _rasterizer(new gcontext::rasterizer_type), _context(context),
				_offset(zero()), _measure_draw(false)
		{
#ifdef WPL_SHOW_STATISTICS
			_statistics_view.reset(new misc::statistics_view(context.text_engine));
#endif
		}

		visual_router::~visual_router()
		{	}

		void visual_router::reload_views()
		{	_underlying.reload_views();	}

		void visual_router::set_offset(const agge_vector<int> &offset)
		{	_offset = offset;	}

		bool visual_router::handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM /*wparam*/, LPARAM /*lparam*/)
		{
			switch (message)
			{
			case WM_SIZE:
				_measure_draw = true;

			default:
				return false;

			case WM_ERASEBKGND:
				result = TRUE;
				break;

			case WM_PAINT:
				LARGE_INTEGER time = {};
				helpers::paint_sequence ps(hwnd);
				auto &backbuffer = *_context.backbuffer;
				auto offset = create_vector<int>(ps.rcPaint.left, ps.rcPaint.top);

				backbuffer.resize(ps.width(), ps.height());

				gcontext ctx(backbuffer, *_context.renderer, *_context.text_engine, offset += _offset);

				_rasterizer->reset();
				stopwatch(time);
				_underlying.draw(ctx, _rasterizer);
				if (_statistics_view)
					_statistics_view->draw(ctx, _rasterizer);
				const auto render_time = stopwatch(time);
				backbuffer.blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.width(), ps.height());
				const auto blit_time = stopwatch(time);

				if (_statistics_view && _measure_draw)
				{
					_statistics_view->set_value("Render", render_time, "ms");
					_statistics_view->set_value("Blit", blit_time, "ms");
					_statistics_view->set_value("Invalid rect", create_box<int>(ps.width(), ps.height()), "");
					invalidate_window(_hwnd, _statistics_view->update());
					_measure_draw = false;
				}
				result = 0;
				break;
			}
			return true;
		}

		void visual_router::invalidate(const rect_i &area)
		{
			auto area2 = area;

			wpl::offset(area2, -_offset.dx, -_offset.dy);
			invalidate_window(_hwnd, area2);
			_measure_draw = true;
		}
	}
}

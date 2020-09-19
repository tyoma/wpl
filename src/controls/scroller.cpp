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

#include <wpl/controls/scroller.h>

#include <algorithm>
#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <agge/stroke_features.h>
#include <math.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		namespace
		{
			typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

			class line
			{
			public:
				line(real_t x1, real_t y1, real_t x2, real_t y2)
					: m_index(0)
				{
					m_points[0] = create_point(x1, y1);
					m_points[1] = create_point(x2, y2);
				}

				void rewind(unsigned)
				{	m_index = 0;	}

				unsigned vertex(real_t* x, real_t* y)
				{
					switch (m_index)
					{
					case 0:	return *x = m_points[m_index].x, *y = m_points[m_index++].y, path_command_move_to;
					case 1:	return *x = m_points[m_index].x, *y = m_points[m_index++].y, path_command_line_to;
					default:	return path_command_stop;
					}
				}

			private:
				point_r m_points[2];
				int m_index;
			};
		}

		scroller::scroller(orientation orientation_)
			: _orientation(orientation_)
		{	_thumb_style.set_join(joins::bevel());	}

		void scroller::set_model(shared_ptr<scroll_model> model)
		{
			_underlying_invalidate = model ? model->invalidated += [this] { invalidate(0); } : slot_connection();
			_model = model;
			invalidate(0);
		}

		scroller::thumb scroller::get_thumb() const
		{
			if (_model)
			{
				const pair<double, double> r(_model->get_range()), w(_model->get_window());

				if (w.second < r.second)
				{
					thumb t = {
						true,
						0.7f * _width,
						to_screen(r, w.first),
						to_screen(r, w.first + w.second)
					};

					return t;
				}
			}

			thumb t = { false, };
	
			return t;
		}

		void scroller::mouse_down(mouse_buttons /*button*/, int /*depressed*/, int x, int y)
		{
			const thumb t =  get_thumb();

			if (t.active)
			{
				const int c = _orientation == horizontal ? x : y;

				if (c < static_cast<int>(t.lbound))
				{
					page_less();
				}
				else if (c >= static_cast<int>(ceilf(t.ubound)))
				{
					page_more();
				}
				else
				{
					_model->scrolling(true);
					_captured_point = c;
					_captured_window = _model->get_window();
					capture(_capture);
				}
			}
		}

		void scroller::mouse_move(int /*depressed*/, int x, int y)
		{
			if (_capture)
			{
				const int c = _orientation == horizontal ? x : y;
				const pair<double, double> r(_model->get_range()), w = _captured_window;
				const double delta = (c - _captured_point) * _rextent * r.second;

				_model->scroll_window(w.first + delta, w.second);
			}
		}

		void scroller::mouse_up(mouse_buttons /*button*/, int /*depressed*/, int /*x*/, int /*y*/)
		{
			if (_capture)
			{
				_capture.reset();
				_model->scrolling(false);
			}
		}

		void scroller::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			const thumb t = get_thumb();

			if (t.active)
			{
				const bool horz = _orientation == horizontal;
				const real_t hw = 0.5f * _width;
				const pair<real_t, real_t> c(hw, _extent - hw);
				line l_channel(horz ? c.first : hw, horz ? hw : c.first, horz ? c.second : hw, horz ? hw : c.second);
				line l_thumb(horz ? t.lbound : hw, horz ? hw : t.lbound, horz ? t.ubound : hw, horz ? hw : t.ubound);
				const color clr_channel = { 255, 255, 255, 192 };
				const color clr_thumb = { 64, 64, 64, 128 };

				add_path(*rasterizer_, assist(l_channel, _thumb_style));
				ctx(rasterizer_, blender_t(clr_channel), winding<>());
				add_path(*rasterizer_, assist(l_thumb, _thumb_style));
				ctx(rasterizer_, blender_t(clr_thumb), winding<>());
			}
		}

		void scroller::resize(unsigned cx, unsigned cy, positioned_native_views &/*native_views*/)
		{
			const int extent = _orientation == horizontal ? cx : cy;
			const int width = _orientation == horizontal ? cy : cx;

			_extent = static_cast<real_t>(extent);
			_rextent = 1.0 / (extent - width); // TODO: fix division by zero below
			_width = static_cast<real_t>(width);
			if (real_t w = 0.7f * _width)
			{
				_thumb_style.set_cap(caps::round());
				_thumb_style.width(w);
			}
			if (extent && _width)
				invalidate(0);
		}

		void scroller::page_less()
		{
			pair<double, double> r(_model->get_range()), w(_model->get_window());

			w.first = (max)(r.first, w.first - w.second);
			_model->scroll_window(w.first, w.second);
		}

		void scroller::page_more()
		{
			pair<double, double> r(_model->get_range()), w(_model->get_window());

			w.first = (min)(r.first + r.second - w.second, w.first + w.second);
			_model->scroll_window(w.first, w.second);
		}

		real_t scroller::to_screen(const pair<double, double> &range, double c) const
		{
			// TODO: fix division by zero below
			return static_cast<real_t>((c - range.first) / (range.second * _rextent)) + 0.5f * _width;
		}
	}
}

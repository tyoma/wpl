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
#include <agge/figures.h>
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
		scroller::scroller(orientation orientation_)
			: _orientation(orientation_)
		{	_thumb_style.set_join(joins::bevel());	}

		void scroller::set_model(shared_ptr<scroll_model> model)
		{
			_underlying_invalidate = model ? model->invalidate += [this] { invalidate(nullptr); } : slot_connection();
			_model = model;
			invalidate(nullptr);
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

		void scroller::mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y)
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
					const auto initial_window = _model->get_window();

					_scroll.start([this, initial_window] (int dx, int dy) {
						const pair<double, double> r(_model->get_range());
						const double delta = (_orientation == horizontal ? dx : dy) * _rextent * r.second;

						_model->scroll_window(initial_window.first + delta, initial_window.second);
					}, capture, button_, x, y);
					_model->scrolling(true);
				}
			}
		}

		void scroller::mouse_move(int /*depressed*/, int x, int y)
		{	_scroll.mouse_move(x, y);	}

		void scroller::mouse_up(mouse_buttons button_, int /*depressed*/, int /*x*/, int /*y*/)
		{
			if (_scroll.mouse_up(button_))
				_model->scrolling(false);
		}

		void scroller::mouse_scroll(int /*depressed*/, int /*x*/, int /*y*/, int delta_x, int delta_y)
		{
			if (const int delta = _orientation == horizontal ? delta_x : delta_y)
			{
				pair<double, double> w(_model->get_window());
				const double increment = _model->get_increment();

				w.first -= increment * delta;
				_model->scrolling(true);
				_model->scroll_window(w.first, w.second);
				_model->scrolling(false);
			}
		}

		void scroller::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender_t;

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

		void scroller::layout(const placed_view_appender &append_view, const agge::box<int> &box_)
		{
			const int extent = _orientation == horizontal ? box_.w : box_.h;
			const int width = _orientation == horizontal ? box_.h : box_.w;

			_extent = static_cast<real_t>(extent);
			_rextent = 1.0 / (extent - width); // TODO: fix division by zero below
			_width = static_cast<real_t>(width);
			if (real_t w = 0.7f * _width)
			{
				_thumb_style.set_cap(caps::round());
				_thumb_style.width(w);
			}
			integrated_control<wpl::scroller>::layout(append_view, box_);
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

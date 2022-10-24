//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/controls/range_slider.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/curves.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/math.h>
#include <agge/path.h>
#include <agge/stroke_features.h>
#include <agge/tools.h>
#include <agge/vertex_sequence.h>
#include <wpl/stylesheet.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;

		auto thumb_inner = [] (real_t x0, real_t x1, real_t x2, real_t x3, real_t y0, real_t y1, real_t y2, real_t y3, real_t y4, real_t d) {
			return agge::joining(line(x0, y1, x1, y0))
				& line(x1, y2, x2, y2)
				& line(x2, y0, x3, y1)
				& cbezier(x3, y3, x3, y3 + d, x2 + d, y4, x2, y4, 0.05f)
				& cbezier(x1, y4, x1 - d, y4, x0, y3 + d, x0, y3, 0.05f)
				& agge::path_close();
		};

		auto thumb = [] (real_t l, real_t r, real_t y, real_t w) {
			return wpl::controls::thumb_inner(l - 0.5f * w, l, r, r + 0.5f * w,
				y - 2.0f * w, y - w, y - 0.5f * w, y, y + 0.5f * w,
				0.5f * c_qarc_bezier_k * w);
		};



		void range_slider_core::layout(const placed_view_appender &append_view, const box<int> &box_)
		{
			integrated_control<wpl::range_slider>::layout(append_view, box_);
			_state = initialize(create_box(static_cast<real_t>(box_.w), static_cast<real_t>(box_.h)));
			if (_model)
				recalculate(_state, *_model);
		}

		void range_slider_core::set_model(shared_ptr<sliding_window_model> model)
		{
			_invalidation = (_model = model, model) ? model->invalidate += [this] (bool) {
				recalculate(_state, *_model);
				invalidate(nullptr);
			} : nullptr;
			if (_model)
				recalculate(_state, *_model);
			invalidate(nullptr);
		}

		void range_slider_core::mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y)
		{
			if (auto m = _model)
			{
				const auto range = m->get_range();
				const auto w = m->get_window();
				const auto k = range.second / (_state.channel.far_x - _state.channel.near_x);

				switch (hit_test(_state, create_point(static_cast<real_t>(x), static_cast<real_t>(y))))
				{
				case part_near:
					start_drag(button_, x, y, [w, k] (int d) {	return make_pair(w.first + k * d, w.second - k * d);	});
					break;

				case part_far:
					start_drag(button_, x, y, [w, k] (int d) {	return make_pair(w.first, w.second + k * d);	});
					break;

				case part_shaft:
					start_drag(button_, x, y, [w, k] (int d) {	return make_pair(w.first + k * d, w.second);	});
					break;
				}
			}
		}

		void range_slider_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{	draw(_state, ctx, rasterizer_);	}

		template <typename F>
		void range_slider_core::start_drag(mouse_buttons button_, int x, int y, const F &calculate_window)
		{
			const auto m = _model;

			m->scrolling(true);
			_drag.start([calculate_window, m] (int dx, int) {
				const auto new_window = calculate_window(dx);
				m->set_window(new_window.first, new_window.second);
			}, [m] {	m->scrolling(false);	}, capture, button_, x, y);
		}

		void range_slider_core::recalculate(descriptor &state, const sliding_window_model &model)
		{
			const auto r = model.get_range();
			const auto w = model.get_window();
			const auto k = (state.channel.far_x - state.channel.near_x) / r.second;

			state.thumb.near_x = static_cast<real_t>((w.first - r.first) * k + state.channel.near_x);
			state.thumb.far_x = static_cast<real_t>((w.first + w.second- r.first) * k + state.channel.near_x);
		}


		void range_slider::apply_styles(const stylesheet &stylesheet_)
		{
			_thumb_width = stylesheet_.get_value("thumb.width.slider");
			_stroke[0].set_cap(caps::round());
			_stroke[1].set_join(joins::bevel());
			_stroke[1].width(1.0f);
		}

		range_slider::descriptor range_slider::initialize(box_r box_) const
		{
			const auto channel_overhang = 0.5f * _thumb_width + 3.0f;
			range_slider::descriptor d = {
				2 * _thumb_width,
				{	channel_overhang, static_cast<real_t>(box_.w) - channel_overhang	},
			};

			return d;
		}

		void range_slider::draw(const descriptor &state, gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			line channel(state.channel.near_x, state.y, state.channel.far_x, state.y);

			rasterizer_->reset();
			_stroke[0].width(_thumb_width + 2.0f);
			add_path(*rasterizer_, assist(channel, _stroke[0]));
			ctx(rasterizer_, blender(color::make(64, 64, 64)), winding<>());

			rasterizer_->reset();
			_stroke[0].width(_thumb_width + 3.0f);
			add_path(*rasterizer_, assist(assist(channel, _stroke[0]), _stroke[1]));
			ctx(rasterizer_, blender(color::make(255, 255, 255)), winding<>());

			rasterizer_->reset();
			add_path(*rasterizer_, thumb(state.thumb.near_x, state.thumb.far_x, state.y, _thumb_width));
			ctx(rasterizer_, blender(color::make(128, 128, 128)), winding<>());
		}

		range_slider::thumb_part range_slider::hit_test(const descriptor &state, point_r point_)
		{
			auto w = 10.0f;
			const auto hw = 0.5f * w;
			const auto &r = state.thumb;
			const auto within = [] (real_t v, real_t n, real_t f) {	return n <= v && v < f;	};

			if (within(point_.x, r.near_x - hw, r.near_x + hw) && within(point_.y, state.y - 2.0f * w, state.y + hw))
				return part_near;
			else if (within(point_.x, r.far_x - hw, r.far_x + hw) && within(point_.y, state.y - 2.0f * w, state.y + hw))
				return part_far;
			else if (within(point_.x, r.near_x, r.far_x) && within(point_.y, state.y - hw, state.y + hw))
				return part_shaft;
			else
				return part_none;
		}
	}
}

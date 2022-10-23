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

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
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

		namespace
		{
			typedef blender_solid_color<simd::blender_solid_color, platform_pixel_order> blender;
		}

		void range_slider_core::layout(const placed_view_appender &append_view, const box<int> &box_)
		{
			integrated_control<wpl::range_slider>::layout(append_view, box_);
			_state = initialize(create_box(static_cast<real_t>(box_.w), static_cast<real_t>(box_.h)));
			if (_model)
				recalculate(_state, *_model);
		}

		void range_slider_core::set_model(shared_ptr<sliding_window_model> model)
		{
			_invalidation = model->invalidate += [this] (bool) {
				recalculate(_state, *_model);
			};
			_model = model;
			recalculate(_state, *_model);
		}

		void range_slider_core::mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y)
		{
			switch (auto part = hit_test(_state, create_point(static_cast<real_t>(x), static_cast<real_t>(y))))
			{
			case part_none:
				break;

			default:
				_drag.start([] (int, int) {
				}, [] {	}, capture, button_, x, y);
			}
		}

		void range_slider_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{	draw(_state, ctx, rasterizer_);	}

		void range_slider_core::recalculate(descriptor &state, const sliding_window_model &model)
		{
			const auto r = model.get_range();
			const auto w = model.get_window();
			const auto k = (state.channel.far_x - state.channel.near_x) / r.second;

			state.thumb.near_x = static_cast<real_t>((w.first - r.first) * k + state.channel.near_x);
			state.thumb.far_x = static_cast<real_t>((w.first + w.second- r.first) * k + state.channel.near_x);
		}


		range_slider::descriptor range_slider::initialize(box_r box_) const
		{
			auto w = 10.0f;
			range_slider::descriptor d = {
				2 * w,
				{	0.5f * w + 2.0f, static_cast<real_t>(box_.w) - 0.5f * w - 2.0f	},
			};

			return d;
		}

		void range_slider::draw(const descriptor &state, gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			line channel(state.channel.near_x, state.y, state.channel.far_x, state.y);
			auto thumb_ = thumb(state.thumb.near_x, state.thumb.far_x, state.y, 10.0f);

			_stroke[0].set_cap(caps::round());
			_stroke[0].width(12.0f);
			_stroke[1].set_join(joins::bevel());
			_stroke[1].width(1.0f);

			rasterizer_->reset();
			add_path(*rasterizer_, assist(channel, _stroke[0]));
			ctx(rasterizer_, blender(color::make(64, 64, 64)), winding<>());

			rasterizer_->reset();
			_stroke[0].width(13.0f);
			add_path(*rasterizer_, assist(assist(channel, _stroke[0]), _stroke[1]));
			ctx(rasterizer_, blender(color::make(255, 255, 255)), winding<>());

			rasterizer_->reset();
			add_path(*rasterizer_, thumb_);
			ctx(rasterizer_, blender(color::make(128, 128, 128)), winding<>());
		}

		range_slider::thumb_part range_slider::hit_test(const descriptor &thumb, point_r point_)
		{
			auto w = 10.0f;
			const auto hw = 0.5f * w;
			const auto &r = thumb.thumb;
			const auto within = [] (real_t v, real_t n, real_t f) {	return n <= v && v < f;	};

			if (within(point_.x, r.near_x - hw, r.near_x + hw) && within(point_.y, thumb.y - 2.0f * w, thumb.y + hw))
				return part_near;
			else if (within(point_.x, r.far_x - hw, r.far_x + hw) && within(point_.y, thumb.y - 2.0f * w, thumb.y + hw))
				return part_far;
			else if (within(point_.x, r.near_x, r.far_x) && within(point_.y, thumb.y - hw, thumb.y + hw))
				return part_shaft;
			else
				return part_none;
		}
	}
}

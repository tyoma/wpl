#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/tools.h>
#include <agge.text/limit.h>
#include <agge.text/richtext.h>
#include <agge.text/text_engine.h>
#include <samples/common/application.h>
#include <wpl/controls.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/helpers.h>
#include <wpl/view.h>

using namespace std;
using namespace wpl;

namespace
{
	const agge::font_style_annotation c_default_annotation = {	agge::font_descriptor::create("Arial", 13),	};

	class hint : public view
	{
	public:
		hint()
			: _text(c_default_annotation)
		{	}

		void set_text(gcontext::text_engine_type &text_services, const agge::richtext_modifier_t &text)
		{
			_text.clear();
			_text << text;
			_box = text_services.measure(_text, agge::limit::none());
			_box.w += 10.0f, _box.h += 10.0f;
			invalidate(nullptr);
		}

		agge::box<int> get_box() const
		{	return agge::create_box(static_cast<int>(_box.w) + 1, static_cast<int>(_box.h) + 1);	}

	private:
		typedef agge::blender_solid_color<agge::simd::blender_solid_color, platform_pixel_order> blender_t;

	private:
		virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
		{
			auto r = agge::create_rect(0.0f, 0.0f, _box.w, _box.h);

			agge::add_path(*rasterizer, agge::rectangle(r.x1, r.y1, r.x2, r.y2));
			ctx(rasterizer, blender_t(agge::color::make(240, 240, 240)), agge::winding<>());
			inflate(r, -4.999f, -5.0f);
			ctx.text_engine.render(*rasterizer, _text, agge::align_center, agge::align_center, r, agge::limit::none());
			rasterizer->sort(true);
			ctx(rasterizer, blender_t(agge::color::make(32, 32, 32)), agge::winding<>());
		}

	private:
		shared_ptr<gcontext::text_engine_type> _text_services;
		agge::richtext_t _text;
		agge::box_r _box;
	};

	class cursor_location : public control, public view, public enable_shared_from_this<view>
	{
	public:
		cursor_location(shared_ptr<gcontext::text_engine_type> text_services)
			: _text_services(text_services)
		{	}

	private:
		typedef agge::blender_solid_color<agge::simd::blender_solid_color, platform_pixel_order> blender_t;

	private:
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &b) override
		{
			placed_view pv = {
				shared_from_this(),
				shared_ptr<native_view>(),
				agge::create_rect(0, 0, b.w, b.h),
				0,
				false,
			};

			append_view(pv);
			if (_hint)
			{
				const auto b2 = _hint->get_box();

				pv.regular = _hint;
				pv.location = agge::create_rect(_location.x, _location.y, _location.x + b2.w, _location.y + b2.h);
				pv.overlay = true;
				offset(pv.location, 16, 16);
				append_view(pv);
			}
		}

		virtual void mouse_enter() override
		{	_hint = make_shared<hint>(), layout_changed(true);	}

		virtual void mouse_leave() override
		{	_hint.reset(), layout_changed(true);	}

		virtual void mouse_move(int /*depressed*/, int x, int y) override
		{
			using namespace agge;

			_location = create_point(x, y);
			_hint->set_text(*_text_services, "X: " + style::weight(bold) + to_string((long long)x).c_str()
				+ style::weight(regular) + ", Y: " + style::weight(bold) + to_string((long long)y).c_str());
			layout_changed(false);
		}

	private:
		shared_ptr<gcontext::text_engine_type> _text_services;
		shared_ptr<hint> _hint;
		point_i _location;
	};
}

int main()
{
	application app;
	const auto fct = app.create_default_factory();
	const rect_i l = { 100, 100, 400, 300 };
	shared_ptr<form> f = fct->create_form();
	slot_connection c = f->close += [&app] {	app.exit();	};

	fct->register_control("cursor_location", [] (const factory &, const control_context &context) {
		return make_shared<cursor_location>(context.text_services);
	});

	f->set_root(fct->create_control("cursor_location"));
	f->set_location(l);
	f->set_visible(true);
	
	app.run();
}

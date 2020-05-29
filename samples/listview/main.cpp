#include <wpl/ui/container.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>
#include <wpl/ui/controls/listview.h>

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/form.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/filling_rules.h>
#include <samples/common/font_loader.h>
#include <samples/common/platform.h>
#include <samples/common/timer.h>
#include <string>

using namespace agge;
using namespace std;
using namespace wpl;
using namespace wpl::ui;

namespace
{
	typedef agge::text_engine<gcontext::rasterizer_type> text_engine_t;
	typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

	class my_listview : public controls::listview_core
	{
	public:
		my_listview()
			: _font_loader(new native_font_loader), _text_engine(new text_engine_t(*_font_loader, 4)),
				_font(_text_engine->create_font(L"Segoe UI", 13, false, false, agge::font::key::gf_vertical)),
				_border_width(1.0f)
		{
			agge::font::metrics m = _font->get_metrics();

			_item_height = real_t(int(1.0f * (m.leading + m.ascent + m.descent) + _border_width));
			_baseline_offset = real_t(int(0.5f * (_item_height + m.ascent - m.descent + _border_width)));
		}

	private:
		virtual real_t get_item_height() const
		{	return _item_height;	}

		virtual void draw_item_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*ras*/, const rect_r &/*b*/,
			index_type /*item*/, unsigned /*state*/) const
		{	}

		virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b, index_type /*item*/,
			unsigned /*state*/, index_type /*subitem*/, const wstring &text) const
		{
			_text_engine->render_string(*ras, *_font, text.c_str(), layout::near, b.x1, b.y1 + _baseline_offset, b.x2 - b.x1);
//			ras->sort(true);
			ctx(ras, blender_t(color::make(255, 255, 255)), winding<>());
		}

	private:
		agge::real_t _item_height, _baseline_offset, _border_width;
		shared_ptr<text_engine_base::loader> _font_loader;
		shared_ptr<text_engine_t> _text_engine;
		shared_ptr<agge::font> _font;
	};


	class my_columns : public columns_model
	{
		virtual index_type get_count() const throw() {	return 50u;	}
		virtual void get_column(index_type /*index*/, column &column) const { column.width = 45; }
		virtual void update_column(index_type /*index*/, short int /*width*/) {	}
		virtual std::pair<index_type, bool> get_sort_order() const throw() { return make_pair(npos(), false); }
		virtual void activate_column(index_type /*column*/) {	}
	};

	class my_model : public table_model
	{
	public:
		my_model()
			: _n(0)
		{
			_timer = create_timer(20, [this] (unsigned) {
				_n++;
				invalidated(100);
			});
		}

		virtual index_type get_count() const throw()
		{	return 100u;	}

		virtual void get_text(index_type row, index_type column, wstring &text) const
		{
			row++, column++;
			text = to_wstring(long double(row * _n + 13 * column * _n));
		}

		virtual void set_order(index_type /*column*/, bool /*ascending*/)
		{	}

	private:
		double _n;
		shared_ptr<void> _timer;
	};
}

int main()
{
	ui::font fnt = { L"", 8 };
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = create_form();
	slot_connection c = f->close += &exit_message_loop;
	shared_ptr<listview> lv(new my_listview);
//	shared_ptr<listview> lv = create_listview();
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model);

	lv->set_columns_model(cm);
	lv->set_model(m);

	f->set_font(fnt);
	f->set_view(lv);
	f->set_location(l);
	f->set_visible(true);
	f->set_background_color(color::make(8, 32, 64));
	run_message_loop();
}

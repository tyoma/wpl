#include <wpl/container.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/controls/header.h>
#include <wpl/controls/listview.h>

#include <wpl/win32/controls.h>
#include <wpl/win32/form.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/dash.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>
#include <samples/common/font_loader.h>
#include <samples/common/platform.h>
#include <samples/common/timer.h>
#include <string>

using namespace agge;
using namespace std;
using namespace wpl;

namespace
{
	typedef agge::text_engine<gcontext::rasterizer_type> text_engine_t;
	typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

	class my_listview : public controls::listview_core
	{
	public:
		my_listview()
			: _font_loader(new native_font_loader), _text_engine(new text_engine_t(*_font_loader, 4)),
				_font(_text_engine->create_font(L"segoe ui", 14, false, false, agge::font::key::gf_vertical)),
				_border_width(1.0f)
		{
			agge::font::metrics m = _font->get_metrics();

			_item_height = real_t(int(1.0f * (m.leading + m.ascent + m.descent) + _border_width));
			_baseline_offset = real_t(int(0.5f * (_item_height + m.ascent - m.descent)));

			_stroke.set_cap(agge::caps::butt());
			_stroke.set_join(agge::joins::bevel());
			_stroke.width(1.0f);
			_dash.add_dash(1.0f, 1.0f);
		}

	private:
		virtual real_t get_item_height() const
		{	return _item_height;	}

		virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b,
			index_type /*item*/, unsigned state) const
		{
			if (state & selected)
			{
				add_path(*ras, rectangle(b.x1, b.y1, b.x2, b.y2 - _border_width));
				ctx(ras, blender_t(color::make(32, 208, 255)), winding<>());
			}
			if (state & focused)
			{
				add_path(*ras, assist(assist(rectangle(b.x1 + 0.25f, b.y1 + 0.5f, b.x2 - 0.25f, b.y2 - _border_width - 0.5f), _dash), _stroke));
				ctx(ras, blender_t(color::make(255, 255, 255)), winding<>());
			}
		}

		virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &ras, const rect_r &b, index_type /*item*/,
			unsigned state, index_type /*subitem*/, const wstring &text) const
		{
			const color text_color = state & selected ? color::make(0, 0, 0) : color::make(255, 255, 255);

			_text_engine->render_string(*ras, *_font, text.c_str(), layout::near, b.x1, b.y1 + _baseline_offset, b.x2 - b.x1);
//			ras->sort(true);
			ctx(ras, blender_t(text_color), winding<>());
		}

	private:
		agge::real_t _item_height, _baseline_offset, _border_width;
		shared_ptr<text_engine_base::loader> _font_loader;
		shared_ptr<text_engine_t> _text_engine;
		shared_ptr<agge::font> _font;
		mutable agge::stroke _stroke;
		mutable agge::dash _dash;
	};


	class my_header : public controls::header
	{
	public:
		my_header()
			: _font_loader(new native_font_loader), _text_engine(new text_engine_t(*_font_loader, 4)),
				_font(_text_engine->create_font(L"segoe ui", 14, false, false, agge::font::key::gf_vertical))
		{	}

		virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &ras, const agge::rect_r &b,
			index_type /*item*/, unsigned /*item_state_flags*/ /*state*/, const wstring &text) const
		{
			auto m = _font->get_metrics();
			_text_engine->render_string(*ras, *_font, text.c_str(), layout::near, b.x1, b.y2 - m.descent, b.x2 - b.x1);
			ctx(ras, blender_t(color::make(255, 255, 255)), winding<>());
		}

	private:
		shared_ptr<text_engine_base::loader> _font_loader;
		shared_ptr<text_engine_t> _text_engine;
		shared_ptr<agge::font> _font;
		mutable agge::stroke _stroke;
	};


	class my_columns : public columns_model
	{
	public:
		my_columns()
			: _columns(20, column(L"test", 60))
		{	}

	private:
		virtual index_type get_count() const throw() { return static_cast<index_type>(_columns.size()); }
		virtual void get_value(index_type index, short int &w) const { w = _columns[index].width; }
		virtual void get_column(index_type index, column &column_) const { column_ = _columns[index]; }
		virtual void update_column(index_type index, short int width) { _columns[index].width = width, invalidated(); }
		virtual pair<index_type, bool> get_sort_order() const throw() { return make_pair(npos(), false); }
		virtual void activate_column(index_type /*column*/) {	}

	private:
		vector<column> _columns;
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
	wpl::font fnt = { L"", 8 };
	view_location l = { 100, 100, 300, 200 };
	shared_ptr<form> f = create_form();
	slot_connection c = f->close += &exit_message_loop;
	shared_ptr<listview> lv = controls::create_listview<my_listview, my_header>();
//	shared_ptr<listview> lv = create_listview();
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model);

	lv->set_columns_model(cm);
	lv->set_model(m);

	f->set_font(fnt);
	f->set_view(lv->get_view());
	f->set_location(l);
	f->set_visible(true);
	f->set_background_color(color::make(8, 32, 64));
	lv->select(1, true);
	run_message_loop();
}

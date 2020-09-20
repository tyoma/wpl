#include <crtdbg.h>

#include <wpl/container.h>
#include <wpl/factory.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/controls/listview_core.h>

#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/dash.h>
#include <agge/figures.h>
#include <agge/filling_rules.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>
#include <agge.text/text_engine.h>
#include <samples/common/platform.h>
#include <samples/common/timer.h>
#include <string>

using namespace agge;
using namespace std;
using namespace wpl;

namespace
{
	typedef blender_solid_color<simd::blender_solid_color, order_bgra> blender_t;

	class my_listview : public controls::listview_core
	{
	public:
		my_listview(const shared_ptr<stylesheet> &ss)
			: _font(ss->get_font("text")), _border_width(ss->get_value("border"))
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
			unsigned state, columns_model::index_type /*subitem*/, const wstring &text) const
		{
			const color text_color = state & selected ? color::make(0, 0, 0) : color::make(255, 255, 255);

			ctx.text_engine.render_string(*ras, *_font, text.c_str(), layout::near, b.x1, b.y1 + _baseline_offset, b.x2 - b.x1);
			ctx(ras, blender_t(text_color), winding<>());
		}

	private:
		agge::real_t _item_height, _baseline_offset, _border_width;
		agge::font::ptr _font;
		mutable agge::stroke _stroke;
		mutable agge::dash _dash;
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

		virtual shared_ptr<const trackable> track(index_type row) const
		{
			shared_ptr<my_trackable> t(new my_trackable);

			t->_index = row;
			return t;
		}

	private:
		struct my_trackable : trackable
		{
			virtual index_type index() const
			{	return _index;	}

			index_type _index;
		};

	private:
		double _n;
		shared_ptr<void> _timer;
	};
}

int main()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	auto te = create_text_engine();
	auto ss = make_shared<stylesheet_db>();
	auto fct = factory::create_default(ss);

	ss->set_font("text", te->create_font(L"Segoe UI", 20, false, false, agge::font::key::gf_vertical));
	ss->set_color("background", agge::color::make(16, 16, 16));
	ss->set_color("text", agge::color::make(192, 192, 192));
	ss->set_value("padding", 3);
	ss->set_value("border", 1);

	fct->register_control("listview", [] (const factory &, shared_ptr<stylesheet> ss) {
		return shared_ptr<control>(new my_listview(ss));
	});

	view_location l = { 100, 100, 300, 200 };
	auto f = fct->create_form();
	shared_ptr<container> c(new container);
	shared_ptr<stack> stk(new stack(5, false));
	auto conn = f->close += &exit_message_loop;
	auto lv = static_pointer_cast<listview>(fct->create_control("listview"));
	shared_ptr<my_columns> cm(new my_columns);
	shared_ptr<my_model> m(new my_model);

	c->set_layout(stk);

	lv->set_columns_model(cm);
	lv->set_model(m);
	c->add_view(lv->get_view(), 1);
	stk->add(100);

	auto btn = static_pointer_cast<button>(fct->create_control("button"));
	btn->set_text(L"first");
	c->add_view(btn->get_view(), 2);
	stk->add(20);

	btn = static_pointer_cast<button>(fct->create_control("button"));
	btn->set_text(L"second");
	c->add_view(btn->get_view(), 3);
	stk->add(20);

	f->set_view(c);
	f->set_location(l);
	f->set_visible(true);
	f->set_background_color(color::make(8, 32, 64));
	run_message_loop();
}

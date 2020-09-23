#include <wpl/controls/listview_core.h>

#include "helpers-visual.h"
#include "Mockups.h"
#include "MockupsListView.h"
#include "predicates.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef columns_model::column column_t;
			enum drawing_event_type { item_background = 1, subitem_background = 2, item_self = 4, subitem_self = 8, };

			mocks::autotrackable_table_model_ptr create_model(table_model::index_type count, columns_model::index_type columns_count = 1)
			{
				return mocks::autotrackable_table_model_ptr(new mocks::autotrackable_table_model(count, columns_count));
			}

			class tracking_listview : public controls::listview_core
			{
			public:
				struct drawing_event
				{
					template <typename T>
					drawing_event(drawing_event_type type_, gcontext &context_,
							const gcontext::rasterizer_ptr &rasterizer_, const agge::rect<T> &box_, index_type item_,
							unsigned state_, columns_model::index_type subitem_ = 0u, wstring text_ = L"")
						: type(type_), context(&context_), rasterizer(rasterizer_.get()), item(item_),
							subitem(subitem_), state(state_), text(text_)
					{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2; }

					drawing_event_type type;
					gcontext *context;
					gcontext::rasterizer_type *rasterizer;
					agge::rect<double> box;
					table_model::index_type item;
					columns_model::index_type subitem;
					unsigned state;
					wstring text;
				};

				using controls::listview_core::item_state_flags;

			public:
				tracking_listview()
					: item_height(0), reported_events(item_background | subitem_background | item_self | subitem_self)
				{	}

			public:
				mutable vector<drawing_event> events;
				double item_height;
				unsigned reported_events;

			private:
				virtual agge::real_t get_minimal_item_height() const
				{	return (agge::real_t)item_height;	}

				virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
					const agge::rect_r &box, index_type item, unsigned state) const
				{
					if (item_background & reported_events)
						events.push_back(drawing_event(item_background, ctx, rasterizer, box, item, state));
				}

				virtual void draw_subitem_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
					const agge::rect_r &box, index_type item, unsigned state, columns_model::index_type subitem) const
				{
					if (subitem_background & reported_events)
						events.push_back(drawing_event(subitem_background, ctx, rasterizer, box, item, state, subitem));
				}

				virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
					const agge::rect_r &box, index_type item, unsigned state) const
				{
					if (item_self & reported_events)
						events.push_back(drawing_event(item_self, ctx, rasterizer, box, item, state));
				}

				virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
					index_type item, unsigned state, columns_model::index_type subitem, const std::wstring &text) const
				{
					if (subitem_self & reported_events)
						events.push_back(drawing_event(subitem_self, ctx, rasterizer, box, item, state, subitem, text));
				}
			};

			typedef tracking_listview::item_state_flags item_state;

			struct rect_eq : eq
			{
				using eq::operator ();

				bool operator ()(const tracking_listview::drawing_event &lhs,
					const tracking_listview::drawing_event &rhs) const
				{
					return lhs.type == rhs.type && lhs.context == rhs.context && lhs.rasterizer == rhs.rasterizer
						&& (*this)(lhs.box, rhs.box) && lhs.item == rhs.item && lhs.state == rhs.state
						&& lhs.subitem == rhs.subitem && lhs.text == rhs.text;
				}

				template <typename T1, typename T2>
				bool operator ()(const agge::rect<T1> &lhs, const agge::rect<T2> &rhs) const
				{
					return (*this)(lhs.x1, rhs.x1) && (*this)(lhs.y1, rhs.y1)
						&& (*this)(lhs.x2, rhs.x2) && (*this)(lhs.y2, rhs.y2);
				}
			};
		}

		begin_test_suite( ListViewCoreTests )

			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> ren;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext> ctx;
			gcontext::rasterizer_ptr ras;
			view::positioned_native_views nviews;

			init( Init )
			{
				surface.reset(new gcontext::surface_type(1000, 1000, 0));
				ren.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ctx.reset(new gcontext(*surface, *ren, *text_engine, agge::zero()));
				ras.reset(new gcontext::rasterizer_type);
			}


			test( ConstructionOfListViewDoesNotDrawAnything )
			{
				// INIT / ACT
				tracking_listview lv;

				// ASSERT
				assert_is_empty(lv.events);
			}


			test( NothingIsDrawnIfModelsAreMissing )
			{
				// INIT
				tracking_listview lv1, lv2;

				lv1.resize(1000, 1000, nviews);
				lv2.resize(1000, 1000, nviews);

				// ACT
				lv1.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(lv1.events);

				// INIT
				lv1.set_columns_model(mocks::columns_model::create(L"Name", 100));
				lv2.set_model(create_model(10));

				// ACT
				lv1.draw(*ctx, ras);
				lv2.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(lv1.events);
				assert_is_empty(lv2.events);
			}


			test( ListViewIsInvalidatedWhenModelIsSet )
			{
				// INIT
				auto invalidations = 0;
				tracking_listview lv;

				lv.resize(1000, 1000, nviews);

				auto c = lv.invalidate += [&] (const void* p) { assert_null(p); invalidations++; };

				// ACT
				lv.set_model(create_model(10));

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				lv.set_model(create_model(10));
				lv.set_model(create_model(100));

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( DrawingSequenceIsItemBgSubitemBgItemFgSubitemFg )
			{
				// INIT
				tracking_listview lv;

				lv.resize(1000, 1000, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"Name"));
				lv.set_model(create_model(1, 1));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 0, 0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 0, 0, 0), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 0, 0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 0, 0, 0), 0, 0),
				};

				assert_equal_pred(reference, lv.events, rect_eq());
			}


			test( DrawingSequenceIsRowsCols )
			{
				// INIT
				agge::rect_r z = {};
				tracking_listview lv;
				column_t c[] = { column_t(L"", 0), column_t(L"", 0), column_t(L"", 0), };

				lv.resize(1000, 1000, nviews);
				lv.set_columns_model(mocks::columns_model::create(c, columns_model::npos(), true));
				lv.set_model(create_model(4, 3));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, z, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 0, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 0, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, z, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 0, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 0, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 0, 0, 2),

					tracking_listview::drawing_event(item_background, *ctx, ras, z, 1, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 1, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 1, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 1, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, z, 1, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 1, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 1, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 1, 0, 2),

					tracking_listview::drawing_event(item_background, *ctx, ras, z, 2, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 2, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 2, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 2, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, z, 2, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 2, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 2, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 2, 0, 2),

					tracking_listview::drawing_event(item_background, *ctx, ras, z, 3, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 3, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 3, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, z, 3, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, z, 3, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 3, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 3, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, z, 3, 0, 2),
				};

				assert_equal_pred(reference, lv.events, rect_eq());
			}


			test( CoordinatesArePassedToAspectDrawers )
			{
				// INIT
				tracking_listview lv;
				column_t c1[] = { column_t(L"", 10), column_t(L"", 30), };
				column_t c2[] = { column_t(L"", 10), column_t(L"", 13), column_t(L"", 29), };

				lv.resize(1000, 1000, nviews);
				lv.item_height = 3;
				lv.set_columns_model(mocks::columns_model::create(c1, columns_model::npos(), true));
				lv.set_model(create_model(3, 2));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 40, 3), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 0, 10, 3), 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(10, 0, 40, 3), 0, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 40, 3), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 0, 10, 3), 0, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(10, 0, 40, 3), 0, 0, 1),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 3, 40, 6), 1, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 3, 10, 6), 1, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(10, 3, 40, 6), 1, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 3, 40, 6), 1, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 3, 10, 6), 1, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(10, 3, 40, 6), 1, 0, 1),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 6, 40, 9), 2, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 6, 10, 9), 2, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(10, 6, 40, 9), 2, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 6, 40, 9), 2, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 6, 10, 9), 2, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(10, 6, 40, 9), 2, 0, 1),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());

				// INIT
				lv.item_height = 5.1;
				lv.set_columns_model(mocks::columns_model::create(c2, columns_model::npos(), true));
				lv.set_model(create_model(2, 3));
				lv.events.clear();

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0.0, 0.0, 52.0, 5.1), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0.0, 0.0, 10.0, 5.1), 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(10.0, 0.0, 23.0, 5.1), 0, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(23.0, 0.0, 52.0, 5.1), 0, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 0.0, 52.0, 5.1), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0.0, 0.0, 10.0, 5.1), 0, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(10.0, 0.0, 23.0, 5.1), 0, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(23.0, 0.0, 52.0, 5.1), 0, 0, 2),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0.0, 5.1, 52.0, 10.2), 1, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0.0, 5.1, 10.0, 10.2), 1, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(10.0, 5.1, 23.0, 10.2), 1, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(23.0, 5.1, 52.0, 10.2), 1, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 5.1, 52.0, 10.2), 1, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0.0, 5.1, 10.0, 10.2), 1, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(10.0, 5.1, 23.0, 10.2), 1, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(23.0, 5.1, 52.0, 10.2), 1, 0, 2),
				};

				assert_equal_pred(reference2, lv.events, rect_eq());
			}


			test( ModelTextIsPassedToSubItem )
			{
				// INIT
				tracking_listview lv;
				column_t c[] = { column_t(L"", 1), column_t(L"", 1), };
				const auto cm = mocks::columns_model::create(c, columns_model::npos(), true);
				const auto m = create_model(2, 2);

				lv.resize(1000, 1000, nviews);
				lv.item_height = 1;
				lv.set_columns_model(cm);
				lv.set_model(m);

				m->items[0][0] = L"one";
				m->items[0][1] = L"one two";
				m->items[1][0] = L"lorem ipsum";
				m->items[1][1] = L"dolor sit amet";

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 2, 1), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(1, 0, 2, 1), 0, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 2, 1), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0, 0, L"one"),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(1, 0, 2, 1), 0, 0, 1, L"one two"),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 1, 2, 2), 1, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(1, 1, 2, 2), 1, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 1, 2, 2), 1, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0, 0, L"lorem ipsum"),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(1, 1, 2, 2), 1, 0, 1, L"dolor sit amet"),
				};

				assert_equal_pred(reference, lv.events, rect_eq());
			}


			test( OnlyVerticallyVisibleItemsGetsToRendering )
			{
				// INIT
				tracking_listview lv;

				lv.resize(1000, 31, nviews);
				lv.item_height = 15.4;
				lv.set_columns_model(mocks::columns_model::create(L"", 10));
				lv.set_model(create_model(4, 1));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				assert_equal(12u, lv.events.size());

				// INIT
				lv.item_height = 31;
				lv.events.clear();

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				assert_equal(4u, lv.events.size());
			}


			test( NonNullStableVerticalScrollModelIsProvided )
			{
				// INIT
				tracking_listview lv;

				// INIT / ACT
				const auto sm = lv.get_vscroll_model();

				// ASSERT
				assert_not_null(sm);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 0), sm->get_range(), eq());
				assert_equal_pred(make_pair(0, 0), sm->get_window(), eq());

				// ACT / ASSERT
				assert_equal(sm, lv.get_vscroll_model());
			}


			test( NonNullStableHorizontalScrollModelIsProvided )
			{
				// INIT
				tracking_listview lv;

				// INIT / ACT
				const auto sm = lv.get_hscroll_model();

				// ASSERT
				assert_not_null(sm);
				assert_not_equal(lv.get_vscroll_model(), sm);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 0), sm->get_range(), eq());
				assert_equal_pred(make_pair(0, 0), sm->get_window(), eq());

				// ACT / ASSERT
				assert_equal(sm, lv.get_hscroll_model());
			}


			test( DestructionOfTheCoreLeavesVerticalScrollFunctional )
			{
				// INIT
				shared_ptr<tracking_listview> lv(new tracking_listview);
				const auto sm = lv->get_vscroll_model();
				const auto m = create_model(1, 1);

				lv->resize(1000, 31, nviews);
				lv->item_height = 15.4;
				lv->set_columns_model(mocks::columns_model::create(L"", 10));
				lv->set_model(m);

				// ACT
				lv.reset();

				// ACT / ASSERT
				sm->scrolling(true); // nothing happens
				sm->scroll_window(2, 10); // nothing happends
				assert_equal_pred(make_pair(0, 0), sm->get_range(), eq());
				assert_equal_pred(make_pair(0, 0), sm->get_window(), eq());
			}


			test( DestructionOfTheCoreLeavesHorizontalScrollFunctional )
			{
				// INIT
				shared_ptr<tracking_listview> lv(new tracking_listview);
				const auto sm = lv->get_hscroll_model();
				const auto m = create_model(1, 1);

				lv->resize(1000, 31, nviews);
				lv->item_height = 15.4;
				lv->set_columns_model(mocks::columns_model::create(L"", 10));
				lv->set_model(m);

				// ACT
				lv.reset();

				// ACT / ASSERT
				sm->scrolling(true); // nothing happens
				sm->scroll_window(2, 10); // nothing happends
				assert_equal_pred(make_pair(0, 0), sm->get_range(), eq());
				assert_equal_pred(make_pair(0, 0), sm->get_window(), eq());
			}


			test( VerticalScrollModelRangeIsZeroToItemCount )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m(create_model(1, 1));

				lv.resize(1000, 31, nviews);
				lv.item_height = 15.4;
				lv.set_columns_model(mocks::columns_model::create(L"", 10));
				lv.set_model(m);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 1), sm->get_range(), eq());

				// INIT
				m->items.resize(17, vector<wstring>(1));

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 17), sm->get_range(), eq());
			}


			test( HorizontalScrollModelRangeEqualsTotalColumnsWidth )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto m(create_model(1, 1));
				column_t columns1[] = { column_t(L"", 19), column_t(L"", 13), column_t(L"", 37), };

				lv.resize(1000, 31, nviews);
				lv.set_columns_model(mocks::columns_model::create(columns1, columns_model::npos(), true));

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 69), sm->get_range(), eq());

				// INIT
				column_t columns2[] = { column_t(L"", 19), column_t(L"", 37), };

				lv.set_columns_model(mocks::columns_model::create(columns2, columns_model::npos(), true));

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 56), sm->get_range(), eq());
			}


			test( HorizontalScrollModelWindowEqualsToWidth )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto m(create_model(1, 1));

				// ACT
				lv.resize(1000, 31, nviews);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 1000), sm->get_window(), eq());

				// ACT
				lv.resize(171, 31, nviews);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 171), sm->get_window(), eq());
			}


			test( HorizontalScrollModelIsInvalidatedOnSettingColumnsModel )
			{
				// INIT
				auto invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto m(create_model(1, 1));

				lv.resize(1000, 31, nviews);

				const auto c = sm->invalidated += [&] { invalidations++; };

				// ACT
				lv.set_columns_model(mocks::columns_model::create(L"", 123));

				// ACT / ASSERT
				assert_equal(1, invalidations);

				// ACT
				lv.set_columns_model(mocks::columns_model::create(L"", 123));
				lv.set_columns_model(mocks::columns_model::create(L"", 127));

				// ACT / ASSERT
				assert_equal(3, invalidations);
			}


			test( HorizontalScrollModelIsInvalidatedOnResize )
			{
				// INIT
				auto invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto m(create_model(1, 1));

				lv.set_columns_model(mocks::columns_model::create(L"", 123));

				const auto c = sm->invalidated += [&] { invalidations++; };

				// ACT
				lv.resize(1000, 31, nviews);

				// ACT / ASSERT
				assert_equal(1, invalidations);

				// ACT
				lv.resize(1000, 31, nviews);
				lv.resize(100, 100, nviews);

				// ACT / ASSERT
				assert_equal(3, invalidations);
			}


			test( VerticalScrollModelWindowSizeIsProportionalToWindowHeight )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m = create_model(1000, 1);

				lv.item_height = 5;
				lv.set_columns_model(mocks::columns_model::create(L"", 10));
				lv.set_model(m);

				// ACT
				lv.resize(1000, 31, nviews);
				const auto v1 = sm->get_window();

				// ASSERT
				assert_approx_equal(0.0, v1.first, 0.0001);

				lv.resize(1000, 39, nviews);
				const auto v2 = sm->get_window();

				// ASSERT
				assert_approx_equal(0.0, v2.first, 0.0001);
				assert_approx_equal(31.0 / 39.0, v1.second / v2.second, 0.0001);

				// ACT
				lv.resize(1000, 397, nviews);
				const auto v3 = sm->get_window().second;

				// ASSERT
				assert_approx_equal(39.0 / 397.0, v2.second / v3, 0.0001);
			}


			test( VerticalScrollModelWindowSizeIsInverselyProportionalToItemHeight )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m = create_model(1, 1);

				m->items.resize(111, vector<wstring>(1));
				lv.set_columns_model(mocks::columns_model::create(L"", 10));
				lv.set_model(m);
				lv.resize(1000, 1000, nviews);

				// ACT
				lv.item_height = 5;
				const auto v1 = sm->get_window().second;
				lv.item_height = 17.3;
				const auto v2 = sm->get_window().second;

				// ASSERT
				assert_approx_equal((1000.0 / 5.0), v1, 0.0001);
				assert_approx_equal((1000.0 / 17.3), v2, 0.0001);

				// INIT
				lv.item_height = 0;

				// ACT / ASSERT
				assert_equal(0.0, sm->get_window().second);
			}


			test( UpdatingVerticalScrollModelOffsetsDisplayVertically )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.resize(100, 1000, nviews);
				lv.item_height = 10;
				lv.set_columns_model(mocks::columns_model::create(L"", 10));
				lv.set_model(create_model(1, 1));

				// ACT
				sm->scroll_window(-0.8, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(0.31, 0);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
				};

				assert_equal_pred(reference2, lv.events, rect_eq());
			}


			test( UpdatingHorizontalScrollModelOffsetsDisplayHorizontally )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				columns_model::column columns[] = {
					columns_model::column(L"", 17), columns_model::column(L"", 31),columns_model::column(L"", 23),
				};
				auto cm = mocks::columns_model::create(columns, columns_model::npos(), false);

				lv.reported_events = item_background | subitem_background | item_self | subitem_self;
				lv.item_height = 1;
				lv.resize(30, 1000, nviews);
				lv.set_columns_model(cm);
				lv.set_model(create_model(1, 3));

				// ACT
				sm->scroll_window(-13, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(13.0, 0.0, 84.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(13.0, 0.0, 30.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(13.0, 0.0, 84.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(13.0, 0.0, 30.0, 1.0), 0, 0, 0),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(0, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0.0, 0.0, 71.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0.0, 0.0, 17.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(17.0, 0.0, 48.0, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 0.0, 71.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0.0, 0.0, 17.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(17.0, 0.0, 48.0, 1.0), 0, 0, 1),
				};

				assert_equal_pred(reference2, lv.events, rect_eq());

				// INIT
				cm->columns[1].width = 10;
				lv.events.clear();

				// ACT
				sm->scroll_window(7.4, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference3[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(-7.4, 0.0, 42.6, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(-7.4, 0.0, 9.6, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(9.6, 0.0, 19.6, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(19.6, 0.0, 42.6, 1.0), 0, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(-7.4, 0.0, 42.6, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(-7.4, 0.0, 9.6, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(9.6, 0.0, 19.6, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(19.6, 0.0, 42.6, 1.0), 0, 0, 2),
				};

				assert_equal_pred(reference3, lv.events, rect_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(30, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference4[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(-30.0, 0.0, 20.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(-3.0, 0.0, 20.0, 1.0), 0, 0, 2),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(-30.0, 0.0, 20.0, 1.0), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(-3.0, 0.0, 20.0, 1.0), 0, 0, 2),
				};

				assert_equal_pred(reference4, lv.events, rect_eq());
			}


			test( OnlyVisibleAndPartiallyVisibleItemsAreDrawnIfWindowIsApplied2 )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(100, 1));

				// ACT
				sm->scroll_window(98.3 /*first visible*/, 0 /*we don't care yet*/);
				lv.events.clear();
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, -3, 100, 7), 98, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 7, 100, 17), 99, 0),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(101, 0);
				lv.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(lv.events);
			}


			test( ListViewIsInvalidatedOnModelInvalidation )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);

				lv.item_height = 10;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(m);

				auto invalidations = 0;
				const auto c = lv.invalidate += [&] (const void *r) {
					assert_null(r); invalidations++;
				};

				// ACT
				m->invalidated(10);

				// ASSERT
				assert_equal(1, invalidations);

				// INIT
				lv.set_model(shared_ptr<table_model>());
				invalidations = 0;

				// ACT
				m->invalidated(100);

				// ASSERT
				assert_equal(0, invalidations);
			}


			test( VerticalScrollModelIsInvalidatedOnlyWhenModelCountChanges )
			{
				// INIT
				auto invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m = create_model(1000, 1);

				lv.item_height = 10;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));

				const auto c = sm->invalidated += [&] { invalidations++; };

				lv.set_model(m);

				// ACT
				m->invalidated(1000);

				// ASSERT
				assert_equal(0, invalidations);

				// ACT
				m->invalidated(100);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				m->invalidated(100);

				// ASSERT
				assert_equal(1, invalidations);
			}


			test( ScrollModelsAreInvalidatedOnResize )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

				// ACT
				lv.resize(100, 40, nviews);

				// ASSERT
				assert_equal(0, invalidations);
				assert_equal(1, scroll_invalidations);
			}


			test( ScrollingInvalidatesScrollModel )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

				// ACT
				sm->scroll_window(10, 0);

				// ASSERT
				assert_equal(1, invalidations);
				assert_equal(1, scroll_invalidations);

				// ACT
				sm->scroll_window(100, 0);

				// ASSERT
				assert_equal(2, invalidations);
				assert_equal(2, scroll_invalidations);
			}


			test( MakingVisibleScrollsTheView )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(10, 28, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

				// ACT
				lv.make_visible(31);

				// ASSERT
				assert_equal(1, scroll_invalidations);
				assert_equal(1, invalidations);
				assert_approx_equal(26.4, sm->get_window().first, 0.001);

				// ACT
				lv.make_visible(13);

				// ASSERT
				assert_equal(2, scroll_invalidations);
				assert_equal(2, invalidations);
				assert_approx_equal(13.0, sm->get_window().first, 0.001);

				// ACT
				lv.make_visible(39);

				// ASSERT
				assert_equal(3, scroll_invalidations);
				assert_equal(3, invalidations);
				assert_approx_equal(34.4, sm->get_window().first, 0.001);

				// ACT
				lv.make_visible(33);

				// ASSERT
				assert_equal(4, scroll_invalidations);
				assert_equal(4, invalidations);
				assert_approx_equal(33.0, sm->get_window().first, 0.001);
			}


			test( MakingVisibleToNPosScrollsNowhere )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(10, 28, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

				lv.make_visible(31);
				lv.make_visible(5);

				invalidations = 0;
				scroll_invalidations = 0;

				// ACT
				lv.make_visible(table_model::npos());

				// ASSERT
				assert_approx_equal(5.0, sm->get_window().first, 0.001);
				assert_equal(0, invalidations);
				assert_equal(0, scroll_invalidations);
			}


			test( MakingVisibleDoesNothingForVisibleItems )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(10, 28, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

				lv.make_visible(31);
				invalidations = 0;
				scroll_invalidations = 0;

				// ACT
				lv.make_visible(27);
				lv.make_visible(28);
				lv.make_visible(29);
				lv.make_visible(30);
				lv.make_visible(31);

				// ASSERT
				assert_equal(0, scroll_invalidations);
				assert_equal(0, invalidations);
				assert_approx_equal(26.4, sm->get_window().first, 0.001);

				lv.make_visible(20);
				invalidations = 0;
				scroll_invalidations = 0;

				// ACT
				lv.make_visible(20);
				lv.make_visible(21);
				lv.make_visible(22);
				lv.make_visible(23);
				lv.make_visible(24);

				// ASSERT
				assert_equal(0, scroll_invalidations);
				assert_equal(0, invalidations);
				assert_approx_equal(20.0, sm->get_window().first, 0.001);
			}


			test( ColumnsModelChangeLeadsToListViewInvalidation )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::columns_model::create(L"", 13);
				auto invalidations = 0;

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				const auto c = lv.invalidate += [&] (const void *r) { assert_null(r); invalidations++; };

				// ACT
				cm->invalidated();

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				cm->invalidated();
				cm->invalidated();

				// ASSERT
				assert_equal(3, invalidations);

				// ACT
				lv.set_columns_model(nullptr);
				cm->invalidated();

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( ColumnsModelInvalidationLeadsToHorizontalScrollModelInvalidation )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto cm = mocks::columns_model::create(L"", 13);
				auto invalidations = 0;

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				const auto c = sm->invalidated += [&] { invalidations++; };

				// ACT
				cm->invalidated();

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				cm->invalidated();
				cm->invalidated();

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( ScrollingOfAHorizontalModelLeadsToChangeOfRange )
			{
				// INIT
				auto invalidations = 0;
				auto sinvalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				columns_model::column columns[] = {
					columns_model::column(L"#1", 103), columns_model::column(L"#2", 53), columns_model::column(L"#3", 43),
				};
				const auto cm = mocks::columns_model::create(columns, columns_model::npos(), false);

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(101, 25, nviews);

				const auto c = lv.invalidate += [&](const void* r) { assert_null(r); invalidations++; };
				const auto c2 = sm->invalidated += [&] { sinvalidations++; };

				// ACT
				sm->scroll_window(10, 101);

				// ACT / ASSERT
				assert_equal_pred(make_pair(10, 101), sm->get_window(), eq());
				assert_equal(1, invalidations);
				assert_equal(1, sinvalidations);

				// ACT
				sm->scroll_window(71.37, 101);

				// ACT / ASSERT
				assert_equal_pred(make_pair(71.37, 101), sm->get_window(), eq());

				// ACT
				sm->scroll_window(-50, 101);

				// ACT / ASSERT
				assert_equal_pred(make_pair(-50, 101), sm->get_window(), eq());
				assert_equal(3, invalidations);
				assert_equal(3, sinvalidations);
			}


			test( DoubleClickingOnItemActivatesIt )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::columns_model::create(L"", 13);
				vector<table_model::index_type> log;
				const auto c = lv.item_activate += [&](table_model::index_type i) { log.push_back(i); };

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.resize(100, 35, nviews);

				// ACT
				lv.mouse_double_click(mouse_input::left, 0, 13, 8);

				// ASSERT
				table_model::index_type reference1[] = { 1u, };

				assert_equal(reference1, log);

				// ACT
				lv.mouse_double_click(mouse_input::left, 0, 18, 24);

				// ASSERT
				table_model::index_type reference2[] = { 1u, 3u, };

				assert_equal(reference2, log);
			}


			test( DoubleClickingOnEmptySpaceActivatesNothing )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::columns_model::create(L"", 13);
				vector<table_model::index_type> log;
				const auto c = lv.item_activate += [&] (...) { assert_is_true(false); };

				lv.set_columns_model(cm);
				lv.set_model(create_model(5, 1));

				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.resize(100, 40, nviews);

				// ACT / ASSERT
				lv.mouse_double_click(mouse_input::left, 0, 13, -1);
				lv.mouse_double_click(mouse_input::left, 0, 18, 35);
			}


			test( KeyboardEventsDoNothingOnNoModel )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::columns_model::create(L"", 13);
				const auto c1 = lv.selection_changed += [&] (...) { assert_is_true(false); };
				const auto c2 = lv.item_activate += [&] (...) { assert_is_true(false); };

				lv.set_columns_model(cm);

				// ACT / ASSERT
				lv.key_down(keyboard_input::up, keyboard_input::control);
				lv.key_down(keyboard_input::up, 0);
				lv.key_down(keyboard_input::down, keyboard_input::control);
				lv.key_down(keyboard_input::down, 0);
				lv.key_down(keyboard_input::page_up, keyboard_input::control);
				lv.key_down(keyboard_input::page_up, 0);
				lv.key_down(keyboard_input::page_down, keyboard_input::control);
				lv.key_down(keyboard_input::page_down, 0);
			}


			test( KeyboardEventsDoNothingOnEmptyModel )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::columns_model::create(L"", 13);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(0, 1));
				const auto c1 = lv.selection_changed += [&] (...) { assert_is_true(false); };
				const auto c2 = lv.item_activate += [&] (...) { assert_is_true(false); };

				lv.set_columns_model(cm);
				lv.set_model(create_model(0, 1));

				// ACT / ASSERT
				lv.key_down(keyboard_input::up, keyboard_input::control);
				lv.key_down(keyboard_input::up, 0);
				lv.key_down(keyboard_input::down, keyboard_input::control);
				lv.key_down(keyboard_input::down, 0);
				lv.key_down(keyboard_input::page_up, keyboard_input::control);
				lv.key_down(keyboard_input::page_up, 0);
				lv.key_down(keyboard_input::page_down, keyboard_input::control);
				lv.key_down(keyboard_input::page_down, 0);

				// ASSERT
				assert_is_empty(m->tracking_requested);
			}

		end_test_suite


		begin_test_suite( ListViewSelectionCoreTests )
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> ren;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext> ctx;
			gcontext::rasterizer_ptr ras;
			view::positioned_native_views nviews;

			vector< pair<table_model::index_type, unsigned /*state*/> > get_visible_states_raw(tracking_listview &lv)
			{
				vector< pair<table_model::index_type, unsigned /*state*/> > selection;

				lv.events.clear();
				lv.draw(*ctx, ras);
				for (auto i = lv.events.begin(); i != lv.events.end(); ++i)
				{
					if (i->state)
						selection.push_back(make_pair(i->item, i->state));
				}
				return selection;
			}

			vector< pair<table_model::index_type, unsigned /*state*/> > get_visible_states(tracking_listview &lv)
			{
				lv.item_height = 1;
				lv.reported_events = item_self;
				lv.resize(1, 1000, nviews);
				return get_visible_states_raw(lv);
			}

			init( Init )
			{
				surface.reset(new gcontext::surface_type(1000, 1000, 0));
				ren.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ctx.reset(new gcontext(*surface, *ren, *text_engine, agge::zero()));
				ras.reset(new gcontext::rasterizer_type);
			}


			test( SelectedElementsHaveCorrespondingStateOnDraw )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 1;
				lv.reported_events = item_background | subitem_background | item_self | subitem_self;
				lv.resize(1, 4, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.select(2, true);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),

					tracking_listview::drawing_event(item_background, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(subitem_background, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(subitem_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());

				// INIT
				lv.events.clear();
				lv.reported_events = item_self;

				// ACT
				lv.select(0, true);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(0, controls::listview_core::selected),
				};

				assert_equal(reference2, get_visible_states(lv));

				// INIT
				lv.events.clear();

				// ACT
				lv.select(1, false);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(0, controls::listview_core::selected),
					make_pair(1, controls::listview_core::selected),
				};

				assert_equal(reference3, get_visible_states(lv));
			}


			test( SelectionAcquiresTrackable )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(1, 27, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				// ACT
				lv.select(3, true);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(3));

				// ACT
				lv.select(31, true);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(31));

				// ACT
				lv.select(1, false);
				lv.select(7, false);

				// ASSERT
				assert_equal(3u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(1));
				assert_equal(1u, m->auto_trackables->count(7));
				assert_equal(1u, m->auto_trackables->count(31));

				// ACT
				lv.select(table_model::npos(), false); // does nothing

				// ASSERT
				assert_equal(3u, m->auto_trackables->size());

				// ACT
				lv.select(0, true);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(0));

				// ACT
				lv.select(1, true);
				lv.select(table_model::npos(), true);

				// ASSERT
				assert_is_empty(*m->auto_trackables);
			}


			test( SelectionFollowsTrackableValueChange )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(1, 27, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.select(0, false);
				lv.select(1, false);
				lv.select(3, false);

				// ACT
				m->move_tracking(0, 2);
				m->invalidated(1000);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(1, controls::listview_core::selected),
					make_pair(2, controls::listview_core::selected),
					make_pair(3, controls::listview_core::selected),
				};

				assert_equal(reference1, get_visible_states_raw(lv));

				// ACT
				m->move_tracking(1, 10);
				m->move_tracking(3, 15);
				m->invalidated(1000);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(2, controls::listview_core::selected),
				};

				assert_equal(reference2, get_visible_states_raw(lv));

				// ACT
				sm->scroll_window(10, 5.4);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(10, controls::listview_core::selected),
					make_pair(15, controls::listview_core::selected),
				};

				assert_equal(reference3, get_visible_states_raw(lv));
			}


			test( FocusedElementsHaveCorrespondingStateOnDraw )
			{
				// INIT
				tracking_listview lv;
				int invalidated = 0;

				lv.item_height = 1;
				lv.reported_events = item_self;
				lv.resize(1, 4, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.select(2, true);

				const auto conn = lv.invalidate += [&] (const void *r) {	assert_null(r); invalidated++;	};

				// ACT
				lv.focus(1);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, controls::listview_core::focused),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
				};

				assert_equal_pred(reference1, lv.events, rect_eq());
				assert_equal(1, invalidated);

				// INIT
				lv.events.clear();

				// ACT
				lv.focus(0);
				lv.focus(3);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, controls::listview_core::focused),
				};

				assert_equal_pred(reference2, lv.events, rect_eq());
				assert_equal(3, invalidated);
			}


			test( NothingHappensOnFocusingWhileNoModelIsSet )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 2;
				lv.resize(1, 9, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));

				const auto conn = lv.invalidate += [] (const void*) {	assert_is_true(false);	};

				// ACT / ASSERT
				lv.focus(1);
			}


			test( FocusingElementAcquiresTrackable )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);

				lv.item_height = 2;
				lv.reported_events = item_self;
				lv.resize(1, 9, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				// INIT / ASSERT
				assert_is_empty(*m->auto_trackables);

				// ACT
				lv.focus(1);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(1));

				// ACT
				lv.focus(7);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(7));
			}


			test( FocusFollowsTrackableValueChange )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.focus(1);

				// ACT
				m->move_tracking(1, 4);
				m->invalidated(1000);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(4, controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states(lv));

				// ACT
				m->move_tracking(4, 0);
				m->invalidated(1000);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(0, controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states(lv));
			}


			test( FocusIsRemovedWhenNoPosIsFocused )
			{
				// INIT
				tracking_listview lv;
				int invalidated = 0;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.resize(10, 100, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.focus(200);

				const auto conn = lv.invalidate += [&] (const void *r) {	assert_null(r); invalidated++;	};

				// ACT
				lv.focus(table_model::npos());

				// ASSERT
				assert_is_empty(get_visible_states(lv));
				assert_is_empty(*m->auto_trackables);
				assert_approx_equal(191.0, sm->get_window().first, 0.001);
			}


			test( FocusingInvisibleElementMakesItVisible )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 1;
				lv.reported_events = item_self;
				lv.resize(1, 4, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				// ACT
				lv.focus(20);

				// ASSERT
				assert_approx_equal(17.0, sm->get_window().first, 0.001);

				// ACT
				lv.focus(14);

				// ASSERT
				assert_approx_equal(14.0, sm->get_window().first, 0.001);

				// ACT
				lv.focus(16);

				// ASSERT
				assert_approx_equal(14.0, sm->get_window().first, 0.001);
			}


			test( InvalidationIsCalledWhenSelectionIsMade )
			{
				// INIT
				tracking_listview lv;
				int invalidated = 0;

				lv.item_height = 1;
				lv.reported_events = item_self;
				lv.resize(1, 4, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				auto conn = lv.invalidate += [&] (const void *) {

					// ASSERT
					invalidated++;

					pair<table_model::index_type, unsigned /*state*/> reference[] = {
						make_pair(3, controls::listview_core::selected),
					};

					assert_equal(reference, get_visible_states_raw(lv));
				};

				// ACT
				lv.select(3, true);

				// ASSERT
				assert_equal(1, invalidated);

				// ACT
				lv.select(table_model::npos(), false);

				// ASSERT
				assert_equal(1, invalidated);

				// INIT
				conn = lv.invalidate += [&] (const void *r) {
					invalidated++;

					// ASSERT
					assert_null(r);
				};

				// ACT
				lv.select(table_model::npos(), true);

				// ASSERT
				assert_equal(2, invalidated);
			}


			test( SelectionIsChangedOnArrowKeys )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(0, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(1, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(2, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference3, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::up, 0);

				// ASSERT
				assert_equal(reference2, get_visible_states(lv));
			}


			test( FirstVisibleIsFocusedOnPageUpNotFromTop )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(0, controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states_raw(lv));

				// INIT
				lv.focus(37);
				lv.focus(29); // < becomes first visible
				lv.focus(32); // somewhere in the middle of the window

				// ACT
				lv.key_down(keyboard_input::page_up, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(29, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states_raw(lv));
			}


			test( FirstVisibleIsFocusedOnPageUpFromNoFocus )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.make_visible(51);
				lv.make_visible(10); // leave it unfocused

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(10, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( PreviousPageIsDisplayedOnPageUpFromTop )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 8;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.focus(100);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(96, controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states_raw(lv));

				// ACT
				lv.key_down(keyboard_input::page_up, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(92, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states_raw(lv));

				// INIT
				lv.resize(100, 39, nviews);
				lv.focus(61);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(56, controls::listview_core::focused),
				};

				assert_equal(reference3, get_visible_states_raw(lv));
			}


			test( FocusDoesNoGoBelowZeroOnPageUp )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.focus(0);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(0, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( FocusDoesNoGoBelowZeroOnPageUpOfOverhangScroll )
			{
				// INIT
				tracking_listview lv;
				auto sm = lv.get_vscroll_model();

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				sm->scroll_window(-1, 4.71);
				lv.focus(2);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(0, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( LastVisibleIsFocusedOnPageDownNotFromBottom )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(4, controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states_raw(lv));

				// INIT
				lv.focus(17); // < becomes last visible
				lv.focus(14); // somewhere in the middle of the window

				// ACT
				lv.key_down(keyboard_input::page_down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(17, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states_raw(lv));
			}


			test( LastVisibleIsFocusedOnPageDownFromNoFocus )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.make_visible(51); // leave it unfocused

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(51, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( NextPageIsDisplayedOnPageDownFromBottom )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.focus(10);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(17, controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states_raw(lv));

				// ACT
				lv.key_down(keyboard_input::page_down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(24, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states_raw(lv));

				// INIT
				lv.resize(100, 32, nviews);
				lv.focus(61);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(67, controls::listview_core::focused),
				};

				assert_equal(reference3, get_visible_states_raw(lv));
			}


			test( FocusDoesNoGoAboveMaxElementOnPageDown )
			{
				// INIT
				tracking_listview lv;

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(100, 1));

				lv.focus(99);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(99, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( FocusDoesNoGoAboveMaxElementOnPageDownOfOverhangScroll )
			{
				// INIT
				tracking_listview lv;
				auto sm = lv.get_vscroll_model();

				lv.resize(100, 33, nviews);
				lv.item_height = 7;
				lv.reported_events = item_self;
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(100, 1));

				sm->scroll_window(96, 4.71);
				lv.focus(96);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(99, controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states_raw(lv));
			}


			test( SelectionIsNotChangedOnGoingLowerThanZero )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.key_down(keyboard_input::up, 0);
				lv.key_up(keyboard_input::up, 0);

				// ASSERT
				assert_is_empty(get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);
				lv.key_down(keyboard_input::up, 0);
				lv.key_up(keyboard_input::up, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(0, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference, get_visible_states(lv));
			}


			test( SelectionIsNotChangedOnGoingAboiveItemCount )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(3, 1);

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);

				// ACT
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(2, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference1, get_visible_states(lv));

				// INIT
				m->items.resize(4, vector<wstring>(1)); // we don't invalidate the model - get_count() is used instead.

				// ACT
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(3, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states(lv));
			}


			test( OnlyFocusChangesOnArrowKeysWithCtrl )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));
				lv.select(1, true);

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(0, controls::listview_core::focused),
					make_pair(1, controls::listview_core::selected),
				};

				assert_equal(reference1, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(1, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(1, controls::listview_core::selected),
					make_pair(2, controls::listview_core::focused),
				};

				assert_equal(reference3, get_visible_states(lv));

				// ACT
				lv.key_down(keyboard_input::up, keyboard_input::control);

				// ASSERT
				assert_equal(reference2, get_visible_states(lv));
			}


			test( OnlyFocusChangesOnArrowKeysWithCtrl2 )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));
				lv.select(0, true);

				// ACT
				lv.key_up(keyboard_input::down, keyboard_input::control);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference[] = {
					make_pair(0, controls::listview_core::selected),
				};

				assert_equal(reference, get_visible_states(lv));
			}


			test( ClickingOnEmptySpaceWithNoModelDoesNothing )
			{
				// INIT
				tracking_listview lv;
				auto c = lv.selection_changed += [] (...) { assert_is_true(false); };

				lv.set_columns_model(mocks::columns_model::create(L"", 1));

				lv.item_height = 5;
				lv.resize(100, 25, nviews);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 10);
				lv.mouse_up(mouse_input::left, 0, 10, 10);

				// ACT / ASSERT
				assert_is_empty(get_visible_states_raw(lv));
			}


			test( ClickingOnItemMakesItSelected )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 0);
				lv.mouse_up(mouse_input::left, 0, 10, 0);
				const auto s1 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(0, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference1, s1);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 4);
				lv.mouse_up(mouse_input::left, 0, 10, 4);
				const auto s2 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference1, s2);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 99, 5);
				lv.mouse_up(mouse_input::left, 0, 99, 5);
				const auto s3 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(1, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference3, s3);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 1, 19);
				lv.mouse_up(mouse_input::left, 0, 99, 19);
				const auto s4 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference4[] = {
					make_pair(3, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference4, s4);

				// INIT
				lv.resize(10, 100, nviews);
				lv.item_height = 4.3;

				// ACT
				lv.mouse_down(mouse_input::left, 0, 0, 0);
				lv.mouse_up(mouse_input::left, 0, 0, 0);
				const auto s5 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference1, s5);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 3);
				lv.mouse_up(mouse_input::left, 0, 19, 3);
				const auto s6 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference1, s6);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 4);
				lv.mouse_up(mouse_input::left, 0, 19, 4);
				const auto s7 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference3, s7);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 8);
				lv.mouse_up(mouse_input::left, 0, 19, 8);
				const auto s8 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference3, s8);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 9);
				lv.mouse_up(mouse_input::left, 0, 19, 9);
				const auto s9 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference9[] = {
					make_pair(2, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference9, s9);
			}


			test( ClickingOutsideItemsIslandResetsSelection )
			{
				// INIT
				tracking_listview lv;
				auto m = create_model(100, 1);

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				lv.mouse_down(mouse_input::left, 0, 10, 0);
				lv.mouse_up(mouse_input::left, 0, 10, 0);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, -1);
				lv.mouse_up(mouse_input::left, 0, 10, -1);

				// ASSERT
				assert_is_empty(get_visible_states_raw(lv));

				// INIT
				lv.mouse_down(mouse_input::left, 0, 10, 15);
				lv.mouse_up(mouse_input::left, 0, 10, 15);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 500);
				lv.mouse_up(mouse_input::left, 0, 10, 500);

				// ASSERT
				assert_is_empty(get_visible_states_raw(lv));
			}


			test( MouseTranslationIsAffectedByScrollWindow )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.item_height = 4.3;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);
				sm->scroll_window(3.8, 0);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 1);
				lv.mouse_up(mouse_input::left, 0, 10, 1);
				const auto s1 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(4, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference1, s1);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 4);
				lv.mouse_up(mouse_input::left, 0, 10, 4);
				const auto s2 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference1, s2);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 5);
				lv.mouse_up(mouse_input::left, 0, 10, 5);
				const auto s3 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference3[] = {
					make_pair(5, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference3, s3);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 9);
				lv.mouse_up(mouse_input::left, 0, 10, 9);
				const auto s4 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference4[] = {
					make_pair(6, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference4, s4);
			}


			test( ClickingOnItemWithControlSwitchesItsSelection )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);
				const auto s1 = get_visible_states_raw(lv);

				// ASSERT
				assert_is_empty(s1);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);
				const auto s2 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(0, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, s2);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);
				const auto s3 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference2, s3);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);
				const auto s4 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference4[] = {
					make_pair(0, controls::listview_core::focused),
				};

				assert_equal(reference4, s4);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 10);
				const auto s5 = get_visible_states_raw(lv);

				// ASSERT
				assert_equal(reference4, s5);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 10);
				const auto s6 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference6[] = {
					make_pair(2, controls::listview_core::selected | controls::listview_core::focused),
				};
				assert_equal(reference6, s6);
			}


			test( MultipleItemsAreSelectedWhenClickedWithControl )
			{
				// INIT
				tracking_listview lv;

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = item_self;
				lv.resize(100, 25, nviews);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);
				lv.mouse_down(mouse_input::left, keyboard_input::control, 11, 12);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 11, 12);
				const auto s1 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference1[] = {
					make_pair(0, controls::listview_core::selected),
					make_pair(2, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference1, s1);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 11, 16);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 11, 16);
				const auto s2 = get_visible_states_raw(lv);

				// ASSERT
				pair<table_model::index_type, unsigned /*state*/> reference2[] = {
					make_pair(0, controls::listview_core::selected),
					make_pair(2, controls::listview_core::selected),
					make_pair(3, controls::listview_core::selected | controls::listview_core::focused),
				};

				assert_equal(reference2, s2);
			}


			test( TrackablesAreReleasedOnModelChange )
			{
				// INIT
				tracking_listview lv;
				const auto m1 = create_model(1000, 1);
				const auto m2 = create_model(1000, 1);

				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m1);

				lv.select(10, true);
				lv.select(13, false);
				lv.focus(10);

				// ACT
				lv.set_model(m1);

				// ASSERT
				assert_equal(2u, m1->auto_trackables->size());
				assert_equal(1u, m1->auto_trackables->count(10));
				assert_equal(1u, m1->auto_trackables->count(13));

				// ACT
				lv.set_model(m2);

				// ASSERT
				assert_is_empty(*m1->auto_trackables);

				// INIT / ACT
				lv.select(3, true);
				lv.focus(3);

				// ASSERT
				assert_is_empty(*m1->auto_trackables);

				// ACT
				lv.set_model(nullptr);

				// ASSERT
				assert_is_empty(*m2->auto_trackables);
			}


			test( FocusedItemKeptVisibleOnTrackableMove )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.resize(100, 33, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);

				lv.focus(10);

				// ACT
				m->move_tracking(10, 97);
				m->invalidated(1000);

				// ASSERT
				assert_approx_equal(91.4, sm->get_window().first, 0.001);

				// ACT
				m->move_tracking(97, 37);
				m->invalidated(1000);

				// ASSERT
				assert_approx_equal(37.0, sm->get_window().first, 0.001);
			}


			test( FocusedItemVisibilityPreservanceIsStoppedOnScrolling )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 5;
				lv.resize(100, 33, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(m);
				lv.focus(25);

				// ACT
				sm->scrolling(true);
				sm->scroll_window(40, 33.0 / 5);
				m->move_tracking(25, 100);
				m->invalidated(1000);

				// ASSERT
				assert_approx_equal(40.0, sm->get_window().first, 0.001);
				assert_equal(1u, m->auto_trackables->size());

				// ACT
				lv.focus(25);

				// ASSERT
				assert_approx_equal(40.0, sm->get_window().first, 0.001);

				// ACT
				sm->scrolling(false);

				// ASSERT
				assert_approx_equal(40.0, sm->get_window().first, 0.001);

				// ACT
				m->move_tracking(25, 0);
				m->invalidated(1000);

				// ASSERT
				assert_approx_equal(40.0, sm->get_window().first, 0.001);

				// ACT
				lv.focus(25);

				// ASSERT
				assert_approx_equal(25.0, sm->get_window().first, 0.001);

				// ACT
				m->move_tracking(25, 10);
				m->invalidated(1000);

				// ASSERT
				assert_approx_equal(10.0, sm->get_window().first, 0.001);
			}


			test( SelectionChangeNotificationIsFiredOnSelectionChange )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 5;
				lv.resize(100, 33, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				vector< pair<table_model::index_type, bool> > selections;
				auto c = lv.selection_changed += [&] (table_model::index_type item, bool selected) {
					selections.push_back(make_pair(item, selected));
				};

				// ACT
				lv.select(10, true);

				// ASSERT
				pair<table_model::index_type, bool> reference1[] = {
					make_pair(10u, true),
				};

				assert_equivalent(reference1, selections);

				// INIT
				selections.clear();

				// ACT
				lv.select(11, false);
				lv.select(111, false);

				// ASSERT
				pair<table_model::index_type, bool> reference2[] = {
					make_pair(11u, true),
					make_pair(111u, true),
				};

				assert_equivalent(reference2, selections);

				// INIT
				selections.clear();

				// ACT
				lv.select(123, true);

				// ASSERT
				pair<table_model::index_type, bool> reference3[] = {
					make_pair(10u, false),
					make_pair(11u, false),
					make_pair(111u, false),
					make_pair(123u, true),
				};

				assert_equivalent(reference3, selections);

				// INIT
				selections.clear();

				// ACT
				lv.select(table_model::npos(), false);

				// ASSERT
				assert_is_empty(selections);
			}


			test( OnlyAClickedItemIsNotifiedInSelectionChange )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 5;
				lv.resize(100, 33, nviews);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.select(7, true);
				lv.select(11, false);
				lv.select(147, false);

				vector< pair<table_model::index_type, bool> > selections;
				auto c = lv.selection_changed += [&] (table_model::index_type item, bool selected) {
					selections.push_back(make_pair(item, selected));
				};

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 0, 6);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 0, 6);

				// ASSERT
				pair<table_model::index_type, bool> reference1[] = {
					make_pair(1u, true),
				};

				assert_equivalent(reference1, selections);

				// INIT
				selections.clear();

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 0, 0);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 0, 0);
				lv.mouse_down(mouse_input::left, keyboard_input::control, 0, 17);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 0, 17);
				lv.mouse_down(mouse_input::left, keyboard_input::control, 0, 6);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 0, 6);

				// ASSERT
				pair<table_model::index_type, bool> reference2[] = {
					make_pair(0u, true),
					make_pair(1u, false),
					make_pair(3u, true),
				};

				assert_equivalent(reference2, selections);
			}
		end_test_suite
	}
}

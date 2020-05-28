#include <wpl/ui/controls/listview.h>

#include "helpers.h"
#include "MockupsListView.h"
#include "predicates.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				typedef columns_model::column column_t;
				enum drawing_event_type { item_background = 1, subitem_background = 2, item_self = 4, subitem_self = 8, };

				class tracking_listview : public controls::listview_core
				{
				public:
					struct drawing_event
					{
						template <typename T>
						drawing_event(drawing_event_type type_, gcontext &context_,
								const gcontext::rasterizer_ptr &rasterizer_, const agge::rect<T> &box_, index_type item_,
								unsigned state_, index_type subitem_ = 0u, wstring text_ = L"")
							: type(type_), context(&context_), rasterizer(rasterizer_.get()), item(item_),
								subitem(subitem_), state(state_), text(text_)
						{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2; }

						drawing_event_type type;
						gcontext *context;
						gcontext::rasterizer_type *rasterizer;
						agge::rect<double> box;
						index_type item;
						index_type subitem;
						unsigned state;
						wstring text;
					};

				public:
					tracking_listview()
						: item_height(0), reported_events(item_background | subitem_background | item_self | subitem_self)
					{	}

				public:
					mutable vector<drawing_event> events;
					double item_height;
					unsigned reported_events;

				private:
					virtual agge::real_t get_item_height() const
					{	return (agge::real_t)item_height;	}

					virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
						const agge::rect_r &box, index_type item, unsigned state) const
					{
						if (item_background & reported_events)
							events.push_back(drawing_event(item_background, ctx, rasterizer, box, item, state));
					}

					virtual void draw_subitem_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
						const agge::rect_r &box, index_type item, unsigned state,
						index_type subitem) const
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

					virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
						const agge::rect_r &box, index_type item, unsigned state, index_type subitem,
						const std::wstring &text) const
					{
						if (subitem_self & reported_events)
							events.push_back(drawing_event(subitem_self, ctx, rasterizer, box, item, state, subitem, text));
					}
				};

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

				shared_ptr<gcontext::surface_type> surface;
				shared_ptr<gcontext::renderer_type> ren;
				shared_ptr<gcontext> ctx;
				gcontext::rasterizer_ptr ras;
				wpl::ui::view::positioned_native_views nviews;

				init( Init )
				{
					surface.reset(new gcontext::surface_type(1000, 1000, 0));
					ren.reset(new gcontext::renderer_type(1));
					ctx.reset(new gcontext(*surface, *ren, make_rect(0, 0, 1000, 1000)));
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
					lv1.set_columns_model(mocks::listview_columns_model::create(L"Name", 100));
					lv2.set_model(mocks::model_ptr(new mocks::listview_model(10)));

					// ACT
					lv1.draw(*ctx, ras);
					lv2.draw(*ctx, ras);

					// ASSERT
					assert_is_empty(lv1.events);
					assert_is_empty(lv2.events);
				}


				test( DrawingSequenceIsItemBgSubitemBgItemFgSubitemFg )
				{
					// INIT
					tracking_listview lv;

					lv.resize(1000, 1000, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"Name"));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(1, 1)));

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
					lv.set_columns_model(mocks::listview_columns_model::create(c, columns_model::npos(), true));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(4, 3)));

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
					lv.set_columns_model(mocks::listview_columns_model::create(c1, columns_model::npos(), true));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(3, 2)));

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
					lv.set_columns_model(mocks::listview_columns_model::create(c2, columns_model::npos(), true));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(2, 3)));
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
					mocks::columns_model_ptr cm = mocks::listview_columns_model::create(c, columns_model::npos(), true);
					mocks::model_ptr m(new mocks::listview_model(2, 2));

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
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 10));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(4, 1)));

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
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();

					// ASSERT
					assert_not_null(sm);

					// ACT / ASSERT
					assert_equal_pred(make_pair(0, 0), sm->get_range(), eq());
					assert_equal_pred(make_pair(0, 0), sm->get_window(), eq());

					// ACT / ASSERT
					assert_equal(sm, lv.get_vscroll_model());
				}


				test( DestructionOfTheCoreLeavesVerticalScrollFunctional )
				{
					// INIT
					shared_ptr<tracking_listview> lv(new tracking_listview);
					shared_ptr<scroll_model> sm = lv->get_vscroll_model();
					mocks::model_ptr m(new mocks::listview_model(1, 1));

					lv->resize(1000, 31, nviews);
					lv->item_height = 15.4;
					lv->set_columns_model(mocks::listview_columns_model::create(L"", 10));
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
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();
					mocks::model_ptr m(new mocks::listview_model(1, 1));

					lv.resize(1000, 31, nviews);
					lv.item_height = 15.4;
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 10));
					lv.set_model(m);

					// ACT / ASSERT
					assert_equal_pred(make_pair(0, 1), sm->get_range(), eq());

					// INIT
					m->items.resize(17, vector<wstring>(1));

					// ACT / ASSERT
					assert_equal_pred(make_pair(0, 17), sm->get_range(), eq());
				}


				test( VerticalScrollModelWindowSizeIsProportionalToWindowHeight )
				{
					// INIT
					tracking_listview lv;
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();
					mocks::model_ptr m(new mocks::listview_model(1000, 1));

					lv.item_height = 5;
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 10));
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
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();
					mocks::model_ptr m(new mocks::listview_model(1, 1));

					m->items.resize(111, vector<wstring>(1));
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 10));
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
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();

					lv.resize(100, 1000, nviews);
					lv.item_height = 10;
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 10));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(1, 1)));

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


				test( OnlyVisibleAndPartiallyVisibleItemsAreDrawnIfWindowIsApplied )
				{
					// INIT
					tracking_listview lv;
					shared_ptr<scroll_model> sm = lv.get_vscroll_model();

					lv.item_height = 10;
					lv.reported_events = item_self;
					lv.resize(100, 40, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 100));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(1000, 1)));

					// ACT
					sm->scroll_window(10.3 /*first visible*/, 0 /*we don't care yet*/);
					lv.draw(*ctx, ras);

					// ACT / ASSERT
					assert_equal(10.3, sm->get_window().first);

					// ASSERT
					tracking_listview::drawing_event reference1[] = {
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, -3, 100, 7), 10, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 7, 100, 17), 11, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 17, 100, 27), 12, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 27, 100, 37), 13, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0, 37, 100, 47), 14, 0),
					};

					assert_equal_pred(reference1, lv.events, rect_eq());

					// INIT
					lv.events.clear();
					lv.item_height = 7.5;

					// ACT
					sm->scroll_window(-3.1, 0);
					lv.draw(*ctx, ras);

					// ACT / ASSERT
					assert_equal(-3.1, sm->get_window().first);

					// ASSERT
					tracking_listview::drawing_event reference2[] = {
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 23.25, 100.0, 30.75), 0, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 30.75, 100.0, 38.25), 1, 0),
						tracking_listview::drawing_event(item_self, *ctx, ras, make_rect(0.0, 38.25, 100.0, 45.75), 2, 0),
					};

					assert_equal_pred(reference2, lv.events, rect_eq());
				}


				test( ListViewIsInvalidatedOnModelInvalidation )
				{
					// INIT
					auto invalidations = 0;
					tracking_listview lv;
					mocks::model_ptr m(new mocks::listview_model(1000, 1));

					lv.item_height = 10;
					lv.reported_events = item_self;
					lv.resize(100, 40, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 100));
					lv.set_model(m);

					auto c = lv.invalidate += [&] (const void *r) { assert_null(r); invalidations++; };

					// ACT
					m->invalidated(10);

					// ASSERT
					assert_equal(1, invalidations);

					// ACT
					lv.set_model(shared_ptr<table_model>());
					m->invalidated(100);

					// ASSERT
					assert_equal(1, invalidations);
				}


				test( VerticalScrollModelIsInvalidatedOnlyWhenModelCountChanges )
				{
					// INIT
					auto invalidations = 0;
					tracking_listview lv;
					auto sm = lv.get_vscroll_model();
					mocks::model_ptr m(new mocks::listview_model(1000, 1));

					lv.item_height = 10;
					lv.reported_events = item_self;
					lv.resize(100, 40, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 100));

					auto c = sm->invalidated += [&] { invalidations++; };

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


				test( ScrollModelsAndListViewAreInvalidatedOnResize )
				{
					// INIT
					auto invalidations = 0;
					auto scroll_invalidations = 0;
					tracking_listview lv;
					auto sm = lv.get_vscroll_model();

					lv.item_height = 10;
					lv.reported_events = item_self;
					lv.resize(100, 40, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 100));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(1000, 1)));

					auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
					auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

					// ACT
					lv.resize(100, 40, nviews);

					// ASSERT
					assert_equal(1, invalidations);
					assert_equal(1, scroll_invalidations);
				}


				test( ScrollingInvalidatesScrollModel )
				{
					// INIT
					auto invalidations = 0;
					auto scroll_invalidations = 0;
					tracking_listview lv;
					auto sm = lv.get_vscroll_model();

					lv.item_height = 10;
					lv.reported_events = item_self;
					lv.resize(100, 40, nviews);
					lv.set_columns_model(mocks::listview_columns_model::create(L"", 100));
					lv.set_model(mocks::model_ptr(new mocks::listview_model(1000, 1)));

					auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
					auto c2 = sm->invalidated += [&] { scroll_invalidations++; };

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

			end_test_suite
		}
	}
}

#include <wpl/controls/listview_core.h>

#include "helpers-listview.h"

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/Mockups.h>
#include <tests/common/MockupsListView.h>
#include <tests/common/predicates.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef mocks::headers_model::column column_t;

			mocks::autotrackable_table_model_ptr create_model(string_table_model::index_type count,
				headers_model::index_type columns_count = 1)
			{	return mocks::autotrackable_table_model_ptr(new mocks::autotrackable_table_model(count, columns_count));	}

			typedef tracking_listview::item_state_flags item_state;
		}

		begin_test_suite( ListViewCoreTests )

			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> ren;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext> ctx;
			gcontext::rasterizer_ptr ras;

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
				assert_is_true(provides_tabstoppable_view(lv));
			}


			test( NothingIsDrawnIfModelsAreMissing )
			{
				// INIT
				tracking_listview lv1, lv2;

				resize(lv1, 1000, 1000);
				resize(lv2, 1000, 1000);

				// ACT
				lv1.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(lv1.events);

				// INIT
				lv1.set_columns_model(mocks::headers_model::create("Name", 100));
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

				resize(lv, 1000, 1000);

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

				resize(lv, 1000, 1000);
				lv.set_columns_model(mocks::headers_model::create("Name", 1));
				lv.set_model(create_model(1, 1));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0, 0, 1, 0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0, 0, 1, 0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, 0, 1, 0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0, 0, 1, 0), 0, 0),
				};

				assert_equal_pred(reference, lv.events, listview_event_eq());
			}


			test( DrawingSequenceIsRowsCols )
			{
				// INIT
				auto z = create_rect(0.0f, 0.0f, 3.0f, 0.0f);
				rect_r zz[] = {
					create_rect(0.0f, 0.0f, 1.0f, 0.0f),
					create_rect(1.0f, 0.0f, 2.0f, 0.0f),
					create_rect(2.0f, 0.0f, 3.0f, 0.0f),
				};
				tracking_listview lv;
				column_t c[] = {	{"", 1	}, {	"", 1	}, {	"", 1	},	};

				resize(lv, 1000, 1000);
				lv.set_columns_model(mocks::headers_model::create(c, headers_model::npos(), true));
				lv.set_model(create_model(4, 3));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, z, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[0], 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[1], 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[2], 0, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, z, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[0], 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[1], 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[2], 0, 0, 2),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, z, 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[0], 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[1], 1, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[2], 1, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, z, 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[0], 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[1], 1, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[2], 1, 0, 2),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, z, 2, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[0], 2, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[1], 2, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[2], 2, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, z, 2, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[0], 2, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[1], 2, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[2], 2, 0, 2),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, z, 3, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[0], 3, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[1], 3, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, zz[2], 3, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, z, 3, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[0], 3, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[1], 3, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, zz[2], 3, 0, 2),
				};

				assert_equal_pred(reference, lv.events, listview_event_eq());
			}


			test( CoordinatesArePassedToAspectDrawers )
			{
				// INIT
				tracking_listview lv;
				column_t c1[] = {	{"", 10	}, {	"", 30	},	};
				column_t c2[] = {	{"", 10	}, {	"", 13	}, {	"", 29	},	};

				resize(lv, 1000, 1000);
				lv.item_height = 3;
				lv.set_columns_model(mocks::headers_model::create(c1, headers_model::npos(), true));
				lv.set_model(create_model(3, 2));

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0, 0, 40, 3), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0, 0, 10, 3), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(10, 0, 40, 3), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, 0, 40, 3), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0, 0, 10, 3), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(10, 0, 40, 3), 0, 0, 1),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0, 3, 40, 6), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0, 3, 10, 6), 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(10, 3, 40, 6), 1, 0, 1),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, 3, 40, 6), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0, 3, 10, 6), 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(10, 3, 40, 6), 1, 0, 1),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0, 6, 40, 9), 2, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0, 6, 10, 9), 2, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(10, 6, 40, 9), 2, 0, 1),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, 6, 40, 9), 2, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0, 6, 10, 9), 2, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(10, 6, 40, 9), 2, 0, 1),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.item_height = 5.1;
				lv.set_columns_model(mocks::headers_model::create(c2, headers_model::npos(), true));
				lv.set_model(create_model(2, 3));
				lv.events.clear();

				// ACT
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0.0, 0.0, 52.0, 5.1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0.0, 0.0, 10.0, 5.1), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(10.0, 0.0, 23.0, 5.1), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(23.0, 0.0, 52.0, 5.1), 0, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0.0, 0.0, 52.0, 5.1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0.0, 0.0, 10.0, 5.1), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(10.0, 0.0, 23.0, 5.1), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(23.0, 0.0, 52.0, 5.1), 0, 0, 2),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0.0, 5.1, 52.0, 10.2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0.0, 5.1, 10.0, 10.2), 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(10.0, 5.1, 23.0, 10.2), 1, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(23.0, 5.1, 52.0, 10.2), 1, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0.0, 5.1, 52.0, 10.2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0.0, 5.1, 10.0, 10.2), 1, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(10.0, 5.1, 23.0, 10.2), 1, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(23.0, 5.1, 52.0, 10.2), 1, 0, 2),
				};

				assert_equal_pred(reference2, lv.events, listview_event_eq());
			}


			test( OnlyVerticallyVisibleItemsGetsToRendering )
			{
				// INIT
				tracking_listview lv;

				resize(lv, 1000, 31);
				lv.item_height = 15.4;
				lv.set_columns_model(mocks::headers_model::create("", 10));
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
				unique_ptr<tracking_listview> lv(new tracking_listview);
				const auto sm = lv->get_vscroll_model();
				const auto m = create_model(1, 1);

				resize(*lv, 1000, 31);
				lv->item_height = 15.4;
				lv->set_columns_model(mocks::headers_model::create("", 10));
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

				resize(*lv, 1000, 31);
				lv->item_height = 15.4;
				lv->set_columns_model(mocks::headers_model::create("", 10));
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

				resize(lv, 1000, 31);
				lv.item_height = 15.4;
				lv.set_columns_model(mocks::headers_model::create("", 10));
				lv.set_model(m);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 1), sm->get_range(), eq());

				// INIT
				m->set_count(17);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 17), sm->get_range(), eq());
			}


			test( HorizontalScrollModelRangeEqualsTotalColumnsWidth )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				const auto m(create_model(1, 1));
				column_t columns1[] = {	{"", 19	}, {	"", 13	}, {	"", 37	},	};

				resize(lv, 1000, 31);
				lv.set_columns_model(mocks::headers_model::create(columns1, headers_model::npos(), true));

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 69), sm->get_range(), eq());

				// INIT
				column_t columns2[] = {	{"", 19	}, {	"", 37	},	};

				lv.set_columns_model(mocks::headers_model::create(columns2, headers_model::npos(), true));

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
				resize(lv, 1000, 31);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0, 1000), sm->get_window(), eq());

				// ACT
				resize(lv, 171, 31);

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

				resize(lv, 1000, 31);

				const auto c = sm->invalidate += [&] (bool invalidate_range) {
					invalidations++;
					assert_is_true(invalidate_range);
				};

				// ACT
				lv.set_columns_model(mocks::headers_model::create("", 123));

				// ACT / ASSERT
				assert_equal(1, invalidations);

				// ACT
				lv.set_columns_model(mocks::headers_model::create("", 123));
				lv.set_columns_model(mocks::headers_model::create("", 127));

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

				lv.set_columns_model(mocks::headers_model::create("", 123));

				const auto c = sm->invalidate += [&] (bool invalidate_range) {
					invalidations++;
					assert_is_true(invalidate_range);
				};

				// ACT
				resize(lv, 1000, 31);

				// ACT / ASSERT
				assert_equal(1, invalidations);

				// ACT
				resize(lv, 1000, 31);
				resize(lv, 100, 100);

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
				lv.set_columns_model(mocks::headers_model::create("", 10));
				lv.set_model(m);

				// ACT
				resize(lv, 1000, 31);
				const auto v1 = sm->get_window();

				// ASSERT
				assert_approx_equal(0.0, v1.first, 0.0001);

				resize(lv, 1000, 39);
				const auto v2 = sm->get_window();

				// ASSERT
				assert_approx_equal(0.0, v2.first, 0.0001);
				assert_approx_equal(31.0 / 39.0, v1.second / v2.second, 0.0001);

				// ACT
				resize(lv, 1000, 397);
				const auto v3 = sm->get_window().second;

				// ASSERT
				assert_approx_equal(39.0 / 397.0, v2.second / v3, 0.0001);
			}


			test( VerticalScrollModelWindowSizeIsInverselyProportionalToItemHeight )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m = create_model(111, 1);

				lv.set_columns_model(mocks::headers_model::create("", 10));
				lv.set_model(m);
				resize(lv, 1000, 1000);

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

				resize(lv, 100, 1000);
				lv.item_height = 10;
				lv.set_columns_model(mocks::headers_model::create("", 10));
				lv.set_model(create_model(1, 1));

				// ACT
				sm->scroll_window(-0.8, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0.0, 8.0, 10.0, 18.0), 0, 0),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(0.31, 0);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0.0, -3.1, 10.0, 6.9), 0, 0),
				};

				assert_equal_pred(reference2, lv.events, listview_event_eq());
			}


			test( UpdatingHorizontalScrollModelOffsetsDisplayHorizontally )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				column_t columns[] = {
					{	"", 17	}, {	"", 31	},{	"", 23	},
				};
				auto cm = mocks::headers_model::create(columns, headers_model::npos(), false);

				lv.reported_events = tracking_listview::item_background | tracking_listview::subitem_background | tracking_listview::item_self | tracking_listview::subitem_self;
				lv.item_height = 1;
				resize(lv, 30, 1000);
				lv.set_columns_model(cm);
				lv.set_model(create_model(1, 3));

				// ACT
				sm->scroll_window(-13, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(13.0, 0.0, 84.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(13.0, 0.0, 30.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(13.0, 0.0, 84.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(13.0, 0.0, 30.0, 1.0), 0, 0, 0),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(0, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(0.0, 0.0, 71.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(0.0, 0.0, 17.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(17.0, 0.0, 48.0, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0.0, 0.0, 71.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(0.0, 0.0, 17.0, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(17.0, 0.0, 48.0, 1.0), 0, 0, 1),
				};

				assert_equal_pred(reference2, lv.events, listview_event_eq());

				// INIT
				cm->columns[1].width = 10;
				lv.events.clear();

				// ACT
				sm->scroll_window(7.4, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference3[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(-7.4, 0.0, 42.6, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(-7.4, 0.0, 9.6, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(9.6, 0.0, 19.6, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(19.6, 0.0, 42.6, 1.0), 0, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(-7.4, 0.0, 42.6, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(-7.4, 0.0, 9.6, 1.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(9.6, 0.0, 19.6, 1.0), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(19.6, 0.0, 42.6, 1.0), 0, 0, 2),
				};

				assert_equal_pred(reference3, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(30, 0 /*we don't care yet*/);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference4[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, create_rect(-30.0, 0.0, 20.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, create_rect(-3.0, 0.0, 20.0, 1.0), 0, 0, 2),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(-30.0, 0.0, 20.0, 1.0), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(-3.0, 0.0, 20.0, 1.0), 0, 0, 2),
				};

				assert_equal_pred(reference4, lv.events, listview_event_eq());
			}


			test( OnlyVisibleAndPartiallyVisibleItemsAreDrawnIfVerticalWindowIsApplied )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(100, 1));

				// ACT
				sm->scroll_window(98.3 /*first visible*/, 0 /*we don't care yet*/);
				lv.events.clear();
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, -3, 100, 7), 98, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, create_rect(0, 7, 100, 17), 99, 0),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(101, 0);
				lv.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(lv.events);
			}


			test( OnlyVisibleAndPartiallyVisibleItemsAreDrawnIfHorizontalWindowIsApplied )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				column_t columns[] = {
					{	"", 15	}, {	"", 30	}, {	"", 25	}, {	"", 55	}, {	"", 10	},
				};
				auto cm = mocks::headers_model::create(columns, headers_model::npos(), false);

				lv.item_height = 10;
				lv.reported_events = tracking_listview::subitem_self;
				resize(lv, 100, 10);
				lv.set_columns_model(cm);
				lv.set_model(create_model(1, 5));

				// ACT
				sm->scroll_window(-30 /*first visible*/, 0 /*we don't care yet*/);
				lv.events.clear();
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(30.0, 0.0, 45.0, 10.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(45.0, 0.0, 75.0, 10.0), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(75.0, 0.0, 100.0, 10.0), 0, 0, 2),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(-25 /*first visible*/, 0 /*we don't care yet*/);
				lv.events.clear();
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(25.0, 0.0, 40.0, 10.0), 0, 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(40.0, 0.0, 70.0, 10.0), 0, 0, 1),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(70.0, 0.0, 95.0, 10.0), 0, 0, 2),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(95.0, 0.0, 150.0, 10.0), 0, 0, 3),
				};

				assert_equal_pred(reference2, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();

				// ACT
				sm->scroll_window(71 /*first visible*/, 0 /*we don't care yet*/);
				lv.events.clear();
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference3[] = {
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(-1.0, 0.0, 54.0, 10.0), 0, 0, 3),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, create_rect(54.0, 0.0, 64.0, 10.0), 0, 0, 4),
				};

				assert_equal_pred(reference3, lv.events, listview_event_eq());
			}


			test( ListViewIsInvalidatedOnModelInvalidation )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(1000, 1);

				lv.item_height = 10;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(m);

				auto invalidations = 0;
				const auto c = lv.invalidate += [&] (const void *r) {
					assert_null(r); invalidations++;
				};

				// ACT
				m->invalidate(string_table_model::npos());

				// ASSERT
				assert_equal(1, invalidations);

				// INIT
				lv.set_model(shared_ptr<richtext_table_model>());
				invalidations = 0;

				// ACT
				m->invalidate(string_table_model::npos());

				// ASSERT
				assert_equal(0, invalidations);
			}


			test( VerticalScrollModelRangeIsInvalidatedOnlyWhenModelCountChanges )
			{
				// INIT
				auto invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();
				const auto m = create_model(1000, 1);

				lv.item_height = 10;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);
				lv.set_columns_model(mocks::headers_model::create("", 100));

				lv.set_model(m);

				const auto c = sm->invalidate += [&] (bool invalidate_range) {
					invalidations++;
					assert_is_true(invalidate_range);
				};

				// ACT
				m->set_count(1000);

				// ASSERT
				assert_equal(0, invalidations);

				// ACT
				m->set_count(100);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				m->set_count(100);

				// ASSERT
				assert_equal(1, invalidations);
			}


			test( VerticalScrollModelsIsInvalidatedOnResize )
			{
				// INIT
				auto invalidations = 0;
				auto scroll_invalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool invalidate_range) {
					scroll_invalidations++;
					assert_is_true(invalidate_range);
				};

				// ACT
				resize(lv, 100, 40);

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool invalidate_range) {
					scroll_invalidations++;
					assert_is_false(invalidate_range);
				};

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 10, 28);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool invalidate_range) {
					scroll_invalidations++;
					assert_is_false(invalidate_range);
				};

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 10, 28);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool) { scroll_invalidations++; };

				lv.make_visible(31);
				lv.make_visible(5);

				invalidations = 0;
				scroll_invalidations = 0;

				// ACT
				lv.make_visible(string_table_model::npos());

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 10, 28);
				lv.set_columns_model(mocks::headers_model::create("", 100));
				lv.set_model(create_model(1000, 1));

				const auto c1 = lv.invalidate += [&] (const void *) { invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool) {
					scroll_invalidations++;
				};

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
				column_t columns[] = {	{	"", 17	}, {	"", 29	}, {	"", 25	},	};
				const auto cm = mocks::headers_model::create(columns, headers_model::npos(), true);
				auto invalidations = 0;

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

				const auto c = lv.invalidate += [&] (const void *r) { assert_null(r); invalidations++; };

				// ACT
				cm->invalidate(1);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				cm->invalidate(2);
				cm->invalidate(0);

				// ASSERT
				assert_equal(3, invalidations);

				// ACT
				lv.set_columns_model(nullptr);
				cm->invalidate(1);

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( ColumnsModelInvalidationLeadsToHorizontalScrollModelRangeInvalidation )
			{
				// INIT
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				column_t columns[] = {	{	"", 17	}, {	"", 29	}, {	"", 25	},	};
				const auto cm = mocks::headers_model::create(columns, headers_model::npos(), true);
				auto invalidations = 0;

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

				const auto c = sm->invalidate += [&] (bool invalidate_range) {
					invalidations++;
					assert_is_true(invalidate_range);
				};

				// ACT
				cm->invalidate(1);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				cm->invalidate(1);
				cm->invalidate(0);

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( ScrollingOfAHorizontalModelLeadsToChangeOfScrollWindow )
			{
				// INIT
				auto invalidations = 0;
				auto sinvalidations = 0;
				tracking_listview lv;
				const auto sm = lv.get_hscroll_model();
				column_t columns[] = {
					{	"#1", 103	}, {	"#2", 53	}, {	"#3", 43	},
				};
				const auto cm = mocks::headers_model::create(columns, headers_model::npos(), false);

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 101, 25);

				const auto c = lv.invalidate += [&](const void* r) { assert_null(r); invalidations++; };
				const auto c2 = sm->invalidate += [&] (bool invalidate_range) {
					sinvalidations++;
					assert_is_false(invalidate_range);
				};

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
				const auto cm = mocks::headers_model::create("", 13);
				vector<string_table_model::index_type> log;
				const auto c = lv.item_activate += [&](string_table_model::index_type i) { log.push_back(i); };

				lv.set_columns_model(cm);
				lv.set_model(create_model(1000, 1));

				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 35);

				// ACT
				lv.mouse_double_click(mouse_input::left, 0, 13, 8);

				// ASSERT
				string_table_model::index_type reference1[] = { 1u, };

				assert_equal(reference1, log);

				// ACT
				lv.mouse_double_click(mouse_input::left, 0, 18, 24);

				// ASSERT
				string_table_model::index_type reference2[] = { 1u, 3u, };

				assert_equal(reference2, log);
			}


			test( DoubleClickingOnEmptySpaceActivatesNothing )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 13);
				vector<string_table_model::index_type> log;
				const auto c = lv.item_activate += [&] (...) { assert_is_true(false); };

				lv.set_columns_model(cm);
				lv.set_model(create_model(5, 1));

				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 40);

				// ACT / ASSERT
				lv.mouse_double_click(mouse_input::left, 0, 13, -1);
				lv.mouse_double_click(mouse_input::left, 0, 18, 35);
			}


			test( KeyboardEventsDoNothingOnNoModel )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 13);
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
				const auto cm = mocks::headers_model::create("", 13);
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


			test( MinHeightIsZeroForMissingOrEmptyModel )
			{
				// INIT
				tracking_listview lv;
				const control &as_control = lv;
				const auto cm = mocks::headers_model::create("", 10);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(0, 1));

				lv.set_columns_model(cm);
				lv.item_height = 19.3;

				// ACT / ASSERT
				assert_equal(0, as_control.min_height());

				// INIT
				lv.set_model(m);

				// ACT / ASSERT
				assert_equal(0, as_control.min_height());
			}


			test( MinHeightEqualsItemSizeTimesItemsCount )
			{
				// INIT
				tracking_listview lv;
				const control &as_control = lv;
				const auto cm = mocks::headers_model::create("", 10);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(11, 1));

				lv.set_columns_model(cm);
				lv.set_model(m);
				lv.item_height = 19.3;

				// ACT / ASSERT
				assert_equal(213, as_control.min_height(10));
				assert_equal(213, as_control.min_height());

				// INIT
				lv.item_height = 5.1;

				// ACT / ASSERT
				assert_equal(57, as_control.min_height(10));
				assert_equal(57, as_control.min_height());

				// INIT
				m->set_count(3);
				lv.item_height = 5;

				// ACT / ASSERT
				assert_equal(15, as_control.min_height(10));
				assert_equal(15, as_control.min_height());
			}


			test( LayoutIsChangedOnChangingModel )
			{
				// INIT
				auto layout_changes = 0;
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 10);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(11, 1));

				lv.set_columns_model(cm);

				const auto conn = lv.layout_changed += [&] (bool hierarchy_changed) {
					assert_is_false(hierarchy_changed);
					layout_changes++;
				};

				// ACT
				lv.set_model(m);

				// ASSERT
				assert_equal(1, layout_changes);

				// ACT
				lv.set_model(m);

				// ASSERT
				assert_equal(1, layout_changes);

				// ACT
				lv.set_model(nullptr);

				// ASSERT
				assert_equal(2, layout_changes);
			}


			test( LayoutIsChangedOnItemCountChange )
			{
				// INIT
				auto layout_changes = 0;
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 10);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(11, 1));

				lv.set_columns_model(cm);
				lv.set_model(m);

				const auto conn = lv.layout_changed += [&] (bool hierarchy_changed) {
					assert_is_false(hierarchy_changed);
					layout_changes++;
				};

				// ACT
				m->set_count(11);

				// ASSERT
				assert_equal(0, layout_changes);

				// ACT
				m->set_count(13);

				// ASSERT
				assert_equal(1, layout_changes);
			}


			test( ModelIsPrecachedOnSetting )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(10, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				resize(lv, 100, 40);

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference1[] = {
					make_pair(0, 6),
				};

				assert_equal(reference1, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.item_height = 9;
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference2[] = {
					make_pair(0, 5),
				};

				assert_equal(reference2, m->precached);

				// INIT
				lv.set_model(nullptr);
				resize(lv, 100, 70);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference3[] = {
					make_pair(0, 8),
				};

				assert_equal(reference3, m->precached);

				// INIT
				lv.set_model(nullptr);
				resize(lv, 100, 72);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference4[] = {
					make_pair(0, 8),
				};

				assert_equal(reference4, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(2, 8);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference5[] = {
					make_pair(2, 8),
				};

				assert_equal(reference5, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(1.5, 8);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference6[] = {
					make_pair(1, 9),
				};

				assert_equal(reference6, m->precached);
			}


			test( PrecachingIsLimitedToModelRange )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(11, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				lv.get_vscroll_model()->scroll_window(-3.3, 40.0 / 7);
				resize(lv, 100, 40);

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference1[] = {
					make_pair(0, 3),
				};

				assert_equal(reference1, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(-5.7143, 40.0 / 7);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference2[] = {
					make_pair(0, 0),
				};

				assert_equal(reference2, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(-50, 40.0 / 7);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				assert_equal(reference2, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(9.3, 40.0 / 7);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference3[] = {
					make_pair(9, 2),
				};

				assert_equal(reference3, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(11.0, 40.0 / 7);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference4[] = {
					make_pair(11, 0),
				};

				assert_equal(reference4, m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(13.0, 40.0 / 7);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				assert_equal(reference4, m->precached);
			}


			test( SettingTheSameModelPrecachesIt )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(11, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				lv.get_vscroll_model()->scroll_window(-3.3, 40.0 / 7);
				resize(lv, 100, 40);
				lv.set_model(m);
				lv.set_model(nullptr);
				m->precached.clear();

				// ACT
				lv.set_model(m);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference[] = {
					make_pair(0, 3),
				};

				assert_equal(reference, m->precached);
			}


			test( ResizingLeadsToPrecaching )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(13, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				resize(lv, 100, 40);
				lv.set_model(m);
				m->precached.clear();

				// ACT
				resize(lv, 100, 65);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference1[] = {
					make_pair(0, 10),
				};

				assert_equal(reference1, m->precached);

				// ACT
				resize(lv, 100, 150);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference2[] = {
					make_pair(0, 10), make_pair(0, 13),
				};

				assert_equal(reference2, m->precached);
			}


			test( ResizingDoesNotLeadToPrecachingIfRangeDoesNotChange )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(13, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				resize(lv, 100, 40);
				lv.set_model(m);
				m->precached.clear();

				// ACT
				resize(lv, 100, 41);
				resize(lv, 100, 36);

				// ASSERT
				assert_is_empty(m->precached);

				// INIT
				resize(lv, 100, 150);
				m->precached.clear();

				// ACT
				resize(lv, 100, 140);

				// ASSERT
				assert_is_empty(m->precached);

				// INIT
				lv.set_model(nullptr);
				lv.get_vscroll_model()->scroll_window(4.9, 140.0 / 7);
				lv.set_model(m);
				m->precached.clear();

				// ACT
				resize(lv, 100, 143);
			}


			test( ScrollingPrecachesModel )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(19, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				resize(lv, 100, 45);
				lv.set_model(m);
				m->precached.clear();

				// ACT
				lv.get_vscroll_model()->scroll_window(4.9, 45.0 / 7);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference1[] = {
					make_pair(4, 8),
				};

				assert_equal(reference1, m->precached);

				// ACT
				lv.get_vscroll_model()->scroll_window(2.1, 45.0 / 7);

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference2[] = {
					make_pair(4, 8), make_pair(2, 7),
				};

				assert_equal(reference2, m->precached);
			}


			test( ModelCountChangePrecachesIt )
			{
				// INIT
				tracking_listview lv;
				const auto cm = mocks::headers_model::create("", 100);
				const shared_ptr<mocks::listview_model> m(new mocks::listview_model(19, 1));

				lv.item_height = 7;
				lv.set_columns_model(cm);
				resize(lv, 100, 400);
				lv.set_model(m);
				m->precached.clear();

				// ACT
				m->items.resize(23, vector<string>(1));
				m->invalidate(string_table_model::npos());

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference1[] = {
					make_pair(0, 23),
				};

				assert_equal(reference1, m->precached);

				// INIT
				lv.get_vscroll_model()->scroll_window(2.7, 400.0 / 7);
				m->precached.clear();

				// ACT
				m->items.resize(20, vector<string>(1));
				m->invalidate(string_table_model::npos());

				// ASSERT
				pair<string_table_model::index_type, string_table_model::index_type> reference2[] = {
					make_pair(2, 18),
				};

				assert_equal(reference2, m->precached);
			}
		end_test_suite
	}
}

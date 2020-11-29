#include <wpl/controls/listview_core.h>

#include "helpers.h"
#include "helpers-listview.h"
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

			mocks::autotrackable_table_model_ptr create_model(table_model::index_type count,
				columns_model::index_type columns_count = 1)
			{	return mocks::autotrackable_table_model_ptr(new mocks::autotrackable_table_model(count, columns_count));	}

			typedef tracking_listview::item_state_flags item_state;
		}


		begin_test_suite( ListViewCoreSelectionTests )
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> ren;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext> ctx;
			gcontext::rasterizer_ptr ras;

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 1000);
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
				lv.reported_events = tracking_listview::item_background | tracking_listview::subitem_background | tracking_listview::item_self | tracking_listview::subitem_self;
				resize(lv, 1, 4);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				// ACT
				lv.select(2, true);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),

					tracking_listview::drawing_event(tracking_listview::item_background, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_background, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
					tracking_listview::drawing_event(tracking_listview::subitem_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());

				// INIT
				lv.events.clear();
				lv.reported_events = tracking_listview::item_self;

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 27);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 27);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 4);
				lv.set_columns_model(mocks::columns_model::create(L"", 1));
				lv.set_model(create_model(1000, 1));

				lv.select(2, true);

				const auto conn = lv.invalidate += [&] (const void *r) {	assert_null(r); invalidated++;	};

				// ACT
				lv.focus(1);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference1[] = {
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, controls::listview_core::focused),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, 0),
				};

				assert_equal_pred(reference1, lv.events, listview_event_eq());
				assert_equal(1, invalidated);

				// INIT
				lv.events.clear();

				// ACT
				lv.focus(0);
				lv.focus(3);
				lv.draw(*ctx, ras);

				// ASSERT
				tracking_listview::drawing_event reference2[] = {
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 0, 1, 1), 0, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 1, 1, 2), 1, 0),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 2, 1, 3), 2, controls::listview_core::selected),
					tracking_listview::drawing_event(tracking_listview::item_self, *ctx, ras, make_rect(0, 3, 1, 4), 3, controls::listview_core::focused),
				};

				assert_equal_pred(reference2, lv.events, listview_event_eq());
				assert_equal(3, invalidated);
			}


			test( NothingHappensOnFocusingWhileNoModelIsSet )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 2;
				resize(lv, 1, 9);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 9);
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
				resize(lv, 10, 100);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 4);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 4);
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 8;
				lv.reported_events = tracking_listview::item_self;
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
				resize(lv, 100, 39);
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
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
				resize(lv, 100, 32);
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
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
				resize(lv, 100, 25);

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

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
				resize(lv, 10, 100);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);
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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

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
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

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
				resize(lv, 100, 33);
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
				resize(lv, 100, 33);
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
				resize(lv, 100, 33);
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
				resize(lv, 100, 33);
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

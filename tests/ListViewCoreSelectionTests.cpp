#include <wpl/controls/listview_core.h>

#include "helpers-listview.h"
#include "mock-dynamic_set.h"

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

			mocks::autotrackable_table_model_ptr create_model(table_model_base::index_type count,
				headers_model::index_type columns_count = 1)
			{	return mocks::autotrackable_table_model_ptr(new mocks::autotrackable_table_model(count, columns_count));	}

			typedef tracking_listview::item_state_flags item_state;
		}


		begin_test_suite( ListViewCoreSelectionTests )
			shared_ptr<mocks::dynamic_set_model> selection;

			init( Init )
			{
				selection.reset(new mocks::dynamic_set_model);
			}


			test( NothingHappensOnFocusingWhileNoModelIsSet )
			{
				// INIT
				tracking_listview lv;

				lv.item_height = 2;
				resize(lv, 1, 9);
				lv.set_columns_model(mocks::headers_model::create("", 1));

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
				lv.set_columns_model(mocks::headers_model::create("", 1));
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


			test( FocusIsRemovedWhenNoPosIsFocused )
			{
				// INIT
				tracking_listview lv;
				int invalidate = 0;
				const auto m = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.item_height = 10;
				resize(lv, 10, 100);
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(m);

				lv.focus(200);

				const auto conn = lv.invalidate += [&] (const void *r) {	assert_null(r); invalidate++;	};

				// ACT
				lv.focus(string_table_model::npos());

				// ASSERT
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
				lv.set_columns_model(mocks::headers_model::create("", 1));
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


			test( InvalidationIsCalledWhenSelectionIsInvalidated )
			{
				// INIT
				tracking_listview lv;
				int invalidate = 0;
				auto model = create_model(1000, 1);

				lv.item_height = 1;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 1, 4);
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				auto conn = lv.invalidate += [&] (const void *r) {
					invalidate++;

					// ASSERT
					assert_null(r);
				};

				// ACT
				selection->invalidate(dynamic_set_model::npos());

				// ASSERT
				assert_equal(1, invalidate);

				// ACT
				selection->invalidate(dynamic_set_model::npos());
				selection->invalidate(dynamic_set_model::npos());

				// ASSERT
				assert_equal(3, invalidate);

				// INIT
				lv.set_selection_model(shared_ptr<dynamic_set_model>());

				// ACT
				selection->invalidate(dynamic_set_model::npos());

				// ASSERT
				assert_equal(3, invalidate);

				// ACT
				model->invalidate(dynamic_set_model::npos());

				// ASSERT
				assert_equal(4, invalidate);
			}


			test( SelectionIsChangedOnArrowKeys )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				unsigned reference1[] = {	0,	};

				assert_equal(reference1, selection->items);
				assert_equal(reference1, model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				unsigned reference2[] = {	1,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::down, 0);

				// ASSERT
				unsigned reference3[] = {	2,	};

				assert_equal(reference3, selection->items);
				assert_equal(reference3, model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::up, 0);

				// ASSERT
				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());
			}


			test( FirstVisibleIsFocusedOnPageUpNotFromTop )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference1[] = {	0,	};

				assert_is_empty(selection->items);
				assert_equal(reference1, model->get_trackables());

				// INIT
				lv.focus(37);
				lv.focus(29); // < becomes first visible
				lv.focus(32); // somewhere in the middle of the window

				// ACT
				lv.key_down(keyboard_input::page_up, 0);

				// ASSERT
				unsigned reference2[] = {	29,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());
			}


			test( FirstVisibleIsFocusedOnPageUpFromNoFocus )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				lv.make_visible(51);
				lv.make_visible(10); // leave it unfocused

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	10,	};

				assert_equal(reference, model->get_trackables());
			}


			test( PreviousPageIsDisplayedOnPageUpFromTop )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 8;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.focus(100);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference1[] = {	96,	};

				assert_is_empty(selection->items);
				assert_equal(reference1, model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::page_up, 0);

				// ASSERT
				unsigned reference2[] = {	92,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());

				// INIT
				resize(lv, 100, 39);
				lv.focus(61);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference3[] = {	56,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference3, model->get_trackables());
			}


			test( FocusDoesNoGoBelowZeroOnPageUp )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				lv.focus(0);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	0,	};

				assert_is_empty(selection->items);
				assert_equal(reference, model->get_trackables());
			}


			test( FocusDoesNoGoBelowZeroOnPageUpOfOverhangScroll )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);
				auto sm = lv.get_vscroll_model();

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				sm->scroll_window(-1, 4.71);
				lv.focus(2);

				// ACT
				lv.key_down(keyboard_input::page_up, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	0,	};

				assert_is_empty(selection->items);
				assert_equal(reference, model->get_trackables());
			}


			test( LastVisibleIsFocusedOnPageDownNotFromBottom )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference1[] = {	4,	};

				assert_is_empty(selection->items);
				assert_equal(reference1, model->get_trackables());

				// INIT
				lv.focus(17); // < becomes last visible
				lv.focus(14); // somewhere in the middle of the window

				// ACT
				lv.key_down(keyboard_input::page_down, 0);

				// ASSERT
				unsigned reference2[] = {	17,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());
			}


			test( LastVisibleIsFocusedOnPageDownFromNoFocus )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				lv.make_visible(51); // leave it unfocused

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	51,	};

				assert_is_empty(selection->items);
				assert_equal(reference, model->get_trackables());
			}


			test( NextPageIsDisplayedOnPageDownFromBottom )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				resize(lv, 100, 33);
				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.focus(10);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference1[] = {	17,	};

				assert_is_empty(selection->items);
				assert_equal(reference1, model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::page_down, 0);

				// ASSERT
				unsigned reference2[] = {	24,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, model->get_trackables());

				// INIT
				resize(lv, 100, 32);
				lv.focus(61);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference3[] = {	67,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference3, model->get_trackables());
			}


			test( FocusDoesNoGoAboveMaxElementOnPageDown )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(100, 1);

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				lv.focus(99);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	99,	};

				assert_is_empty(selection->items);
				assert_equal(reference, model->get_trackables());
			}


			test( FocusDoesNoGoAboveMaxElementOnPageDownOfOverhangScroll )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(100, 1);
				auto sm = lv.get_vscroll_model();

				resize(lv, 100, 33);
				lv.item_height = 7;
				lv.reported_events = tracking_listview::item_self;
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);

				sm->scroll_window(96, 4.71);
				lv.focus(96);

				// ACT
				lv.key_down(keyboard_input::page_down, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	99,	};

				assert_equal(reference, model->get_trackables());
			}


			test( SelectionIsNotChangedOnGoingLowerThanZero )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(100, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				// ACT
				lv.key_down(keyboard_input::up, 0);
				lv.key_up(keyboard_input::up, 0);

				// ASSERT
				assert_is_empty(model->get_trackables());

				// ACT
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);
				lv.key_down(keyboard_input::up, 0);
				lv.key_up(keyboard_input::up, 0);

				// ASSERT
				unsigned reference[] = {	0,	};

				assert_equal(reference, model->get_trackables());
			}


			test( SelectionIsNotChangedOnGoingAboiveItemCount )
			{
				// INIT
				tracking_listview lv;
				const auto m = create_model(3, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(m);
				lv.set_selection_model(selection);

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
				unsigned reference1[] = {	2,	};

				assert_equal(reference1, selection->items);
				assert_equal(reference1, m->get_trackables());

				// INIT
				m->set_count(4);

				// ACT
				lv.key_down(keyboard_input::down, 0);
				lv.key_up(keyboard_input::down, 0);

				// ASSERT
				unsigned reference2[] = {	3,	};

				assert_equal(reference2, selection->items);
				assert_equal(reference2, m->get_trackables());
			}


			test( OnlyFocusChangesOnArrowKeysWithCtrl )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				selection->items.insert(1);

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				unsigned focus1[] = {	0,	};
				unsigned reference1[] = {	1,	};

				assert_equal(focus1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.key_down(keyboard_input::down, keyboard_input::control);

				// ASSERT
				unsigned focus2[] = {	2,	};

				assert_equal(focus2, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.key_down(keyboard_input::up, keyboard_input::control);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);
			}


			test( OnlyFocusChangesOnArrowKeysWithCtrl2 )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				selection->items.insert(0);

				// ACT
				lv.key_up(keyboard_input::down, keyboard_input::control);

				// ASSERT
				unsigned reference[] = {	0,	};

				assert_is_empty(model->get_trackables());
				assert_equal(reference, selection->items);
			}


			test( ClickingOnItemMakesItSelected )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 0);
				lv.mouse_up(mouse_input::left, 0, 10, 0);

				// ASSERT
				unsigned reference1[] = {	0,	};

				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 4);
				lv.mouse_up(mouse_input::left, 0, 10, 4);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 99, 5);
				lv.mouse_up(mouse_input::left, 0, 99, 5);

				// ASSERT
				unsigned reference3[] = {	1,	};

				assert_equal(reference3, model->get_trackables());
				assert_equal(reference3, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 1, 19);
				lv.mouse_up(mouse_input::left, 0, 99, 19);

				// ASSERT
				unsigned reference4[] = {	3,	};

				assert_equal(reference4, model->get_trackables());
				assert_equal(reference4, selection->items);

				// INIT
				resize(lv, 10, 100);
				lv.item_height = 4.3;

				// ACT
				lv.mouse_down(mouse_input::left, 0, 0, 0);
				lv.mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 3);
				lv.mouse_up(mouse_input::left, 0, 19, 3);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 4);
				lv.mouse_up(mouse_input::left, 0, 19, 4);

				// ASSERT
				assert_equal(reference3, model->get_trackables());
				assert_equal(reference3, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 8);
				lv.mouse_up(mouse_input::left, 0, 19, 8);

				// ASSERT
				assert_equal(reference3, model->get_trackables());
				assert_equal(reference3, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 19, 9);
				lv.mouse_up(mouse_input::left, 0, 19, 9);

				// ASSERT
				unsigned reference9[] = {	2,	};

				assert_equal(reference9, model->get_trackables());
				assert_equal(reference9, selection->items);
			}


			test( ClickingOutsideItemsIslandResetsSelection )
			{
				// INIT
				tracking_listview lv;
				auto m = create_model(100, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
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
				assert_is_empty(m->get_trackables());
				assert_is_empty(selection->items);

				// INIT
				lv.mouse_down(mouse_input::left, 0, 10, 15);
				lv.mouse_up(mouse_input::left, 0, 10, 15);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 500);
				lv.mouse_up(mouse_input::left, 0, 10, 500);

				// ASSERT
				assert_is_empty(m->get_trackables());
				assert_is_empty(selection->items);
			}


			test( MouseTranslationIsAffectedByScrollWindow )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);
				const auto sm = lv.get_vscroll_model();

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.item_height = 4.3;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);
				sm->scroll_window(3.8, 0);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 1);
				lv.mouse_up(mouse_input::left, 0, 10, 1);

				// ASSERT
				unsigned reference1[] = {	4,	};

				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 4);
				lv.mouse_up(mouse_input::left, 0, 10, 4);

				// ASSERT
				assert_equal(reference1, model->get_trackables());
				assert_equal(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 5);
				lv.mouse_up(mouse_input::left, 0, 10, 5);

				// ASSERT
				unsigned reference3[] = {	5,	};

				assert_equal(reference3, model->get_trackables());
				assert_equal(reference3, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, 0, 10, 9);
				lv.mouse_up(mouse_input::left, 0, 10, 9);

				// ASSERT
				unsigned reference4[] = {	6,	};

				assert_equal(reference4, model->get_trackables());
				assert_equal(reference4, selection->items);
			}


			test( ClickingOnItemWithControlSwitchesItsSelection )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);

				// ASSERT
				assert_is_empty(model->get_trackables());
				assert_is_empty(selection->items);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);

				// ASSERT
				unsigned reference2[] = {	0,	};

				assert_equal(reference2, model->get_trackables());
				assert_equal(reference2, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);

				// ASSERT
				assert_equal(reference2, model->get_trackables());
				assert_equal(reference2, selection->items);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);

				// ASSERT
				assert_equal(reference2, model->get_trackables());
				assert_is_empty(selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 10);

				// ASSERT
				assert_equal(reference2, model->get_trackables());
				assert_is_empty(selection->items);

				// ACT
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 10);

				// ASSERT
				unsigned reference6[] = {	2,	};

				assert_equal(reference6, model->get_trackables());
				assert_equal(reference6, selection->items);
			}


			test( MultipleItemsAreSelectedWhenClickedWithControl )
			{
				// INIT
				tracking_listview lv;
				auto model = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(model);
				lv.set_selection_model(selection);

				lv.item_height = 5;
				lv.reported_events = tracking_listview::item_self;
				resize(lv, 100, 25);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 10, 2);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 10, 2);
				lv.mouse_down(mouse_input::left, keyboard_input::control, 11, 12);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 11, 12);

				// ASSERT
				unsigned focus1[] = {	2,	};
				unsigned reference1[] = {	0, 2,	};

				assert_equal(focus1, model->get_trackables());
				assert_equivalent(reference1, selection->items);

				// ACT
				lv.mouse_down(mouse_input::left, keyboard_input::control, 11, 16);
				lv.mouse_up(mouse_input::left, keyboard_input::control, 11, 16);

				// ASSERT
				unsigned focus2[] = {	3,	};
				unsigned reference2[] = {	0, 2, 3,	};

				assert_equal(focus2, model->get_trackables());
				assert_equivalent(reference2, selection->items);
			}


			test( FocusTrackablesIsReleasedOnModelChange )
			{
				// INIT
				tracking_listview lv;
				const auto m1 = create_model(1000, 1);
				const auto m2 = create_model(1000, 1);

				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(m1);

				lv.focus(10);

				// ACT
				lv.set_model(m1);

				// ASSERT
				assert_equal(1u, m1->auto_trackables->size());
				assert_equal(1u, m1->auto_trackables->count(10));

				// ACT
				lv.set_model(m2);

				// ASSERT
				assert_is_empty(*m1->auto_trackables);

				// INIT / ACT
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
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(m);

				lv.focus(10);

				// ACT
				m->move_tracking(10, 97);
				m->invalidate(1000);

				// ASSERT
				assert_approx_equal(91.4, sm->get_window().first, 0.001);

				// ACT
				m->move_tracking(97, 37);
				m->invalidate(1000);

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
				lv.set_columns_model(mocks::headers_model::create("", 1));
				lv.set_model(m);
				lv.focus(25);

				// ACT
				sm->scrolling(true);
				sm->scroll_window(40, 33.0 / 5);
				m->move_tracking(25, 100);
				m->invalidate(1000);

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
				m->invalidate(1000);

				// ASSERT
				assert_approx_equal(40.0, sm->get_window().first, 0.001);

				// ACT
				lv.focus(25);

				// ASSERT
				assert_approx_equal(25.0, sm->get_window().first, 0.001);

				// ACT
				m->move_tracking(25, 10);
				m->invalidate(1000);

				// ASSERT
				assert_approx_equal(10.0, sm->get_window().first, 0.001);
			}
		end_test_suite
	}
}

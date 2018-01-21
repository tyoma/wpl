#include <wpl/ui/form.h>

#include <wpl/ui/listview.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			typedef pair<shared_ptr<form>, HWND> form_and_handle;

			form_and_handle create_form_with_handle();

			begin_test_suite( ContainerTests )
				WindowManager windowManager;

				init( Init )
				{
					windowManager.Init();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}

				test( ConstructNestedContainer )
				{
					// INIT
					shared_ptr<form> f = form::create();
					shared_ptr<container> root = f->get_root_container();
					window_tracker wt;

					// ACT
					shared_ptr<container> nc1 = root->create_container(L"1");
					shared_ptr<container> nc2 = root->create_container(L"2");

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_not_null(nc1);
					assert_not_null(!!nc2);
					assert_not_equal(nc1, nc2);

					// ACT
					shared_ptr<container> nc11 = nc1->create_container(L"3");
					shared_ptr<container> nc12 = nc1->create_container(L"4");
					shared_ptr<container> nc21 = nc2->create_container(L"5");

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_not_null(nc11);
					assert_not_null(nc12);
					assert_not_null(nc21);
				}


				test( NestedContainerIsHeldByTheParent )
				{
					// INIT
					shared_ptr<form> f = form::create();
					shared_ptr<container> root = f->get_root_container();

					// ACT
					weak_ptr<container> nc = root->create_container(L"1");

					// ASSERT
					assert_is_false(nc.expired());
				}


				test( NestedContainerIsDestroyedAlongWithParent )
				{
					// INIT
					shared_ptr<form> f = form::create();
					weak_ptr<container> nc = f->get_root_container()->create_container(L"1");

					// ACT
					f = shared_ptr<form>();

					// ASSERT
					assert_is_true(nc.expired());
				}


				test( NestedChildIsFosteredByTopLevelParent )
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> nc1 = root->create_container(L"1");
					shared_ptr<container> nc2 = root->create_container(L"2");
					window_tracker wt(L"SysListView32");

					// ACT
					nc1->create_widget(L"listview", L"1");
					nc2->create_widget(L"listview", L"1");
					wt.checkpoint();

					// ASSERT
					assert_equal(2u, wt.created.size());
					assert_equal(f.second, ::GetParent(wt.created[0]));
					assert_equal(f.second, ::GetParent(wt.created[1]));
				}


				test( WidgetsAreEnumeratedOnLayout )
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<container> c[] = {
						f[0].first->get_root_container(),
						f[1].first->get_root_container()->create_container(L"1"),
					};
					widget_ptr w1[] = {
						c[0]->create_widget(L"listview", L"2"),
						c[0]->create_widget(L"listview", L"3"),
					};
					widget_ptr w2[] = {
						c[1]->create_widget(L"listview", L"2"),
						c[1]->create_widget(L"listview", L"3"),
						c[1]->create_widget(L"listview", L"4"),
					};
					shared_ptr<mocks::logging_layout_manager> lm[] = {
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
					};

					c[0]->layout = lm[0];
					c[1]->layout = lm[1];
					f[1].first->get_root_container()->layout.reset(new mocks::logging_layout_manager);

					// ACT
					::MoveWindow(f[0].second, 10, 11, 150, 60, TRUE);
					::MoveWindow(f[1].second, 10, 11, 160, 70, TRUE);

					// ASSERT
					assert_equal(2u, lm[0]->last_widgets.size());
					assert_equal(w1[0], lm[0]->last_widgets[0].first);
					assert_equal(w1[1], lm[0]->last_widgets[1].first);
					
					assert_equal(3u, lm[1]->last_widgets.size());
					assert_equal(w2[0], lm[1]->last_widgets[0].first);
					assert_equal(w2[1], lm[1]->last_widgets[1].first);
					assert_equal(w2[2], lm[1]->last_widgets[2].first);
				}


				test( ContainersAreEnumeratedOnLayout )
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<container> c[] = {
						f[0].first->get_root_container(),
						f[1].first->get_root_container(),
					};
					shared_ptr<container> c1[] = {
						c[0]->create_container(L"2"),
						c[0]->create_container(L"3"),
						c[0]->create_container(L"4"),
					};
					shared_ptr<container> c2[] = {
						c[1]->create_container(L"2"),
						c[1]->create_container(L"3"),
					};
					shared_ptr<mocks::logging_layout_manager> lm[] = {
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
						shared_ptr<mocks::logging_layout_manager>(new mocks::logging_layout_manager),
					};
					shared_ptr<mocks::logging_layout_manager> dummy_lm(new mocks::logging_layout_manager);

					c[0]->layout = lm[0];
					c[1]->layout = lm[1];
					c1[0]->layout = dummy_lm;
					c1[1]->layout = dummy_lm;
					c1[2]->layout = dummy_lm;
					c2[0]->layout = dummy_lm;
					c2[1]->layout = dummy_lm;

					// ACT
					::MoveWindow(f[0].second, 10, 11, 150, 60, TRUE);
					::MoveWindow(f[1].second, 10, 11, 160, 70, TRUE);

					// ASSERT
					assert_equal(3u, lm[0]->last_widgets.size());
					assert_equal(c1[0], lm[0]->last_widgets[0].first);
					assert_equal(c1[1], lm[0]->last_widgets[1].first);
					assert_equal(c1[2], lm[0]->last_widgets[2].first);

					assert_equal(2u, lm[1]->last_widgets.size());
					assert_equal(c2[0], lm[1]->last_widgets[0].first);
					assert_equal(c2[1], lm[1]->last_widgets[1].first);
				}


				test( WidgetsAndContainersAreEnumeratedOnLayout )
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> c1 = root->create_container(L"2");
					widget_ptr w2 = root->create_widget(L"listview", L"3");
					shared_ptr<container> c3 = root->create_container(L"4");
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr<mocks::logging_layout_manager> dummy_lm(new mocks::logging_layout_manager);

					root->layout = lm;
					c1->layout = dummy_lm;
					c3->layout = dummy_lm;

					// ACT
					::MoveWindow(f.second, 10, 11, 150, 60, TRUE);

					// ASSERT
					assert_equal(3u, lm->last_widgets.size());
					assert_equal(c1, lm->last_widgets[0].first);
					assert_equal(w2, lm->last_widgets[1].first);
					assert_equal(c3, lm->last_widgets[2].first);
				}


				test( ChildrenAreRepositionedAccordinglyToTheContainerOffset )
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					window_tracker wt(L"SysListView32");
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> c1 = root->create_container(L"1");
					widget_ptr w11 = create_widget(wt, *c1, L"listview", L"2");
					widget_ptr w12 = create_widget(wt, *c1, L"listview", L"3");
					widget_ptr w2 = create_widget(wt, *root, L"listview", L"4");
					shared_ptr<container> c3 = root->create_container(L"5");
					widget_ptr w31 = create_widget(wt, *c3, L"listview", L"6");
					widget_ptr w32 = create_widget(wt, *c3, L"listview", L"7");
					widget_ptr w33 = create_widget(wt, *c3, L"listview", L"8");

					wt.checkpoint();
					shared_ptr<mocks::logging_layout_manager> lm(new mocks::logging_layout_manager);
					shared_ptr<mocks::logging_layout_manager> lm1(new mocks::logging_layout_manager);
					shared_ptr<mocks::logging_layout_manager> lm3(new mocks::logging_layout_manager);

					root->layout = lm;
					c1->layout = lm1;
					c3->layout = lm3;

					layout_manager::position p10[] = {
						{ 5, 7, 10, 10 },
						{ 5, 37, 85, 30 },
						{ 5, 70, 10, 10 },
					};
					layout_manager::position p11[] = {
						{ 0, 0, 30, 25 },
						{ 35, 0, 50, 25 },
					};
					layout_manager::position p13[] = {
						{ 1, 2, 45, 35 },
						{ 51, 2, 50, 35 },
						{ 106, 2, 50, 35 },
					};

					lm->positions.assign(begin(p10), end(p10));
					lm1->positions.assign(begin(p11), end(p11));
					lm3->positions.assign(begin(p13), end(p13));

					// ACT
					::MoveWindow(f.second, 10, 11, 150, 60, TRUE);

					// ASSERT
					assert_equal(rect(5, 7, 30, 25), get_window_rect(wt.created[0]));
					assert_equal(rect(40, 7, 50, 25), get_window_rect(wt.created[1]));
					assert_equal(rect(5, 37, 85, 30), get_window_rect(wt.created[2]));
					assert_equal(rect(6, 72, 45, 35), get_window_rect(wt.created[3]));
					assert_equal(rect(56, 72, 50, 35), get_window_rect(wt.created[4]));
					assert_equal(rect(111, 72, 50, 35), get_window_rect(wt.created[5]));
				}
			end_test_suite
		}
	}
}

#include <wpl/ui/listview.h>
#include <wpl/ui/form.h>

#include <wpl/ui/win32/controls.h>

#include "MockupsListView.h"
#include "TestHelpers.h"

#include <windows.h>
#include <commctrl.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				typedef pair<shared_ptr<form>, HWND> form_and_handle;

				form_and_handle create_form_with_handle()
				{
					window_tracker wt(L"#32770");
					shared_ptr<form> f(form::create());

					wt.checkpoint();

					if (wt.created.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, wt.created[0]);
				}

				template <typename T>
				void push_back(vector<T> &v, const T &value)
				{	v.push_back(value);	}
			}

			begin_test_suite( IntegrationTests )
				test( ListViewCooperatesWhenPlacedOnForm )
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					window_tracker wt;
					shared_ptr<listview> lv = wpl::ui::create_listview();
					vector<listview::index_type> selections;
					slot_connection c =
						lv->selection_changed += bind(&push_back<listview::index_type>, ref(selections), _1);

					wt.checkpoint();

					RECT rc;
					HWND hlv = wt.find_created(WC_LISTVIEW)[0];
					listview::columns_model::column columns[] = {
						listview::columns_model::column(L"", 100),
						listview::columns_model::column(L"", 100),
						listview::columns_model::column(L"", 50),
					};
					mocks::columns_model_ptr cm(mocks::listview_columns_model::create(columns, listview::columns_model::npos, false));
					mocks::model_ptr m(new mocks::listview_model(3, 3));

					lv->set_columns_model(cm);
					lv->set_model(m);
					f.first->set_view(lv);
					::MoveWindow(f.second, 0, 0, 300, 200, TRUE);

					// ACT
					ListView_GetItemRect(hlv, 0, &rc, LVIR_SELECTBOUNDS);
 					::PostMessage(hlv, WM_LBUTTONUP, 0, pack_coordinates(rc.left, rc.top));
					::SendMessage(hlv, WM_LBUTTONDOWN, 0, pack_coordinates(rc.left, rc.top));

					// ASSERT
					listview::index_type reference1[] = { 0, };

					assert_equal(reference1, selections);

					// ACT
					ListView_GetItemRect(hlv, 2, &rc, LVIR_SELECTBOUNDS);
 					::PostMessage(hlv, WM_LBUTTONUP, 0, pack_coordinates(rc.left, rc.top));
					::SendMessage(hlv, WM_LBUTTONDOWN, 0, pack_coordinates(rc.left, rc.top));
					ListView_GetItemRect(hlv, 1, &rc, LVIR_SELECTBOUNDS);
 					::PostMessage(hlv, WM_LBUTTONUP, 0, pack_coordinates(rc.left, rc.top));
					::SendMessage(hlv, WM_LBUTTONDOWN, 0, pack_coordinates(rc.left, rc.top));

					// ASSERT
					listview::index_type reference2[] = { 0, listview::npos, 2, listview::npos, 1, };

					assert_equal(reference2, selections);
				}
			end_test_suite
		}
	}
}


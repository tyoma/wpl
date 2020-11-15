#include <wpl/win32/listview.h>
#include <wpl/win32/form.h>

#include "helpers-win32.h"
#include "MockupsListView.h"

#include <windows.h>
#include <commctrl.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef pair<shared_ptr<form>, HWND> form_and_handle;

			form_and_handle create_form_with_handle()
			{
				window_tracker wt(L"#32770");
				shared_ptr<form> f(new win32::form(form_context()));

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
				shared_ptr<listview> lv(new win32::listview());
				vector<table_model::index_type> selections;
				slot_connection c =
					lv->selection_changed += bind(&push_back<table_model::index_type>, ref(selections), _1);
				RECT rc;

				wt.checkpoint();
				wt.created.clear();
				f.first->set_visible(true);

				columns_model::column columns[] = {
					columns_model::column(L"", 100),
					columns_model::column(L"", 100),
					columns_model::column(L"", 50),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(3, 3));

				lv->set_columns_model(cm);
				lv->set_model(m);

				// ACT
				f.first->set_view(lv->get_view());

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.find_created(WC_LISTVIEW).size());

				// INIT
				HWND hlv = wt.find_created(WC_LISTVIEW)[0];

				// ACT
				::MoveWindow(f.second, 0, 0, 300, 200, TRUE);
				ListView_GetItemRect(hlv, 0, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);

				// ASSERT
				table_model::index_type reference1[] = { 0, };

				assert_equal(reference1, selections);

				// ACT
				ListView_GetItemRect(hlv, 2, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);
				ListView_GetItemRect(hlv, 1, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);

				// ASSERT
				table_model::index_type reference2[] = { 0, table_model::npos(), 2, table_model::npos(), 1, };

				assert_equal(reference2, selections);
			}
		end_test_suite
	}
}

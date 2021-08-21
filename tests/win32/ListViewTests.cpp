#include <wpl/win32/listview.h>

#include "helpers-win32.h"

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/mock-dynamic_set.h>
#include <tests/common/MockupsListView.h>

#include <windows.h>
#include <commctrl.h>
#include <map>
#include <olectl.h>
#include <tchar.h>
#include <wpl/win32/utf8.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef mocks::headers_model::column column_t;

			enum sort_direction	{	dir_none, dir_ascending, dir_descending	};

			template <typename T>
			void push_back(vector<T> &v, const T &value)
			{	v.push_back(value);	}

			size_t get_columns_count(HWND hlv)
			{	return Header_GetItemCount(ListView_GetHeader(hlv));	}

			sort_direction get_column_direction(HWND hlv, headers_model::index_type column)
			{
				HDITEM item = { };
				HWND hheader = ListView_GetHeader(hlv);
					
				item.mask = HDI_FORMAT;
				Header_GetItem(hheader, column, &item);
				return (item.fmt & HDF_SORTUP) ? dir_ascending : (item.fmt & HDF_SORTDOWN) ? dir_descending : dir_none;
			}

			string get_column_text(HWND hlv, headers_model::index_type column)
			{
				HDITEMW item = { };
				HWND hheader = ListView_GetHeader(hlv);
				vector<wchar_t> buffer(item.cchTextMax = 100);
				win32::utf_converter c;

				item.mask = HDI_TEXT;
				item.pszText = &buffer[0];
				::SendMessageW(hheader, HDM_GETITEMW, column, (LPARAM)&item);
				return c(item.pszText);
			}

			int get_column_width(HWND hlv, headers_model::index_type column)
			{
				HDITEM item = { };
				HWND hheader = ListView_GetHeader(hlv);
					
				item.mask = HDI_WIDTH;
				Header_GetItem(hheader, column, &item);
				return item.cxy;
			}

			vector<size_t> get_matching_indices(HWND hlv, unsigned int mask)
			{
				int i = -1;
				vector<size_t> result;

				mask &= LVNI_STATEMASK;
				while (i = ListView_GetNextItem(hlv, i, LVNI_ALL | mask), i != -1)
					result.push_back(i);
				return result;
			}

			bool is_item_visible(HWND hlv, int item)
			{
				RECT rc1, rc2, rc;

				::GetClientRect(hlv, &rc1);
				ListView_GetItemRect(hlv, item, &rc2, LVIR_BOUNDS);
				return !!IntersectRect(&rc, &rc1, &rc2);
			}

			RECT get_visible_items_rect(HWND hlv)
			{
				RECT client, enclosing = { 10000, 10000, -10000, -10000 };

				::GetClientRect(hlv, &client);
				for (int i = ListView_GetTopIndex(hlv); i != ListView_GetItemCount(hlv); ++i)
				{
					RECT rc, intersection;

					if (ListView_GetItemRect(hlv, i, &rc, LVIR_BOUNDS) && ::IntersectRect(&intersection, &rc, &client))
						::UnionRect(&enclosing, &enclosing, &rc);
				}
				return enclosing;
			}
		}

		begin_test_suite( ListViewTests )

			shared_ptr<mocks::dynamic_set_model> selection;
			window_manager windowManager;
			HWND hparent;

			init( Initialize )
			{
				selection.reset(new mocks::dynamic_set_model);
				hparent = create_parent_window();
			}

			teardown( Cleanup )
			{
				windowManager.Cleanup();
			}

			HWND create_parent_window()
			{
				HWND hwnd = windowManager.create_visible_window();

				::MoveWindow(hwnd, 50, 50, 400, 400, FALSE);
				windowManager.enable_reflection(hwnd);
				return hwnd;
			}

			pair<shared_ptr<listview>, HWND> create_listview()
			{
				shared_ptr<listview> lv(new win32::listview);

				return make_pair(lv, get_window_and_resize(lv, hparent, 200, 150));
			}


			test( ListViewItemsCountSetOnSettingModel )
			{
				// INIT
				auto lv = create_listview();

				// ACT
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(11)));

				// ASSERT
				assert_equal(11, ListView_GetItemCount(lv.second));

				// ACT
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(23)));

				// ASSERT
				assert_equal(23, ListView_GetItemCount(lv.second));
				assert_is_true(provides_tabstoppable_native_view(lv.first));
			}


			test( ColumnsByTextAreBeingAddedAccordinglyToTheColumnsModel )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"Contract", 10	},
					{	"Price", 10	},
					{	"Volume", 10	},
				};
					
				// ACT
				lv.first->set_columns_model(mocks::headers_model::create(columns, headers_model::npos(), false));

				// ASSERT
				assert_equal(3u, get_columns_count(lv.second));
				assert_equal("Contract", get_column_text(lv.second, 0));
				assert_equal("Price", get_column_text(lv.second, 1));
				assert_equal("Volume", get_column_text(lv.second, 2));
			}


			test( ColumnsAreResetOnNewColumnsModelSetting )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"Contract", 10	},
					{	"Price", 10	},
				};
					
				// ACT
				lv.first->set_columns_model(mocks::headers_model::create(columns, headers_model::npos(), false));
				lv.first->set_columns_model(mocks::headers_model::create("Team"));

				// ASSERT
				assert_equal(1u, get_columns_count(lv.second));
				assert_equal("Team", get_column_text(lv.second, 0));
			}


			test( ListViewInvalidatedOnModelInvalidate )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(11, 1));

				lv.first->set_columns_model(mocks::headers_model::create("test", 13));
				lv.first->set_model(m);

				// ACT
				::UpdateWindow(lv.second);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(lv.second, NULL, FALSE));

				// ACT
				m->invalidate(11);

				// ASSERT
				assert_is_true(!!::GetUpdateRect(lv.second, NULL, FALSE));
			}


			test( ChangingItemsCountIsTrackedByListView )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);

				// ACT
				m->set_count(3);

				// ASSERT
				assert_equal(3, ListView_GetItemCount(lv.second));

				// ACT
				m->set_count(13);

				// ASSERT
				assert_equal(13, ListView_GetItemCount(lv.second));
			}


			test( InvalidationsFromOldModelsAreNotAccepted )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m1(new mocks::listview_model(5)), m2(new mocks::listview_model(7));

				lv.first->set_model(m1);

				// ACT
				lv.first->set_model(m2);
				m1->set_count(3);

				// ASSERT
				assert_equal(7, ListView_GetItemCount(lv.second));
			}


			test( ResettingModelSetsZeroItemsCountAndDisconnectsFromInvalidationEvent )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(5));

				lv.first->set_model(m);

				// ACT
				lv.first->set_model(shared_ptr<richtext_table_model>());

				// ASSERT
				assert_equal(0, ListView_GetItemCount(lv.second));

				// ACT
				m->set_count(7);

				// ASSERT
				assert_equal(0, ListView_GetItemCount(lv.second));
			}


			test( GettingDispInfoOtherThanTextIsNotFailing )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));
				NMLVDISPINFO nmlvdi = {
					{	0, 0, LVN_GETDISPINFO	},
					{ /* mask = */ LVIF_STATE, /* item = */ 0, /* subitem = */ 0, 0, 0, 0, 0, }
				};

				m->set_count(1), m->items[0].resize(3);
				m->items[0][0] = "one", m->items[0][1] = "two", m->items[0][2] = "three";
				lv.first->set_model(m);

				// ACT / ASSERT (must not throw)
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
			}


			test( GettingItemTextNoTruncationUnicode )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));
				wchar_t buffer[100] = { 0 };
				NMLVDISPINFOW nmlvdi = {
					{	0, 0, LVN_GETDISPINFOW	},
					{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
				};

				m->set_count(3), m->items[0].resize(3), m->items[1].resize(3), m->items[2].resize(3);
				m->items[0][0] = "one", m->items[0][1] = "two", m->items[0][2] = "three";
				m->items[1][0] = "four", m->items[1][1] = "five", m->items[1][2] = "six";
				m->items[2][0] = "seven", m->items[2][1] = "eight", m->items[2][2] = "NINE";
				lv.first->set_model(m);

				// ACT / ASSERT
				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"one", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"two", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"three", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"four", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"five", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"six", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"seven", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"eight", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"NINE", buffer));
			}


			test( GettingItemTextWithTruncationUnicode )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));
				wchar_t buffer[4] = { 0 };
				NMLVDISPINFOW nmlvdi = {
					{	0, 0, LVN_GETDISPINFOW	},
					{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
				};

				m->set_count(3), m->items[0].resize(3);
				m->items[0][0] = "one", m->items[0][1] = "two", m->items[0][2] = "three";
				lv.first->set_model(m);

				// ACT / ASSERT
				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"one", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"two", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, wcscmp(L"thr", buffer));
			}


			test( ItemSelectionStateCorrespondsToSelectionModel )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(5, 100));
				NMLVDISPINFOW nmlvdi = {
					{	0, 0, LVN_GETDISPINFOW	},
					{ /* mask = */ LVIF_STATE, /* item = */ 0, /* subitem = */ 0, 0, 0, 0, 0, }
				};

				lv.first->set_model(m);
				selection->items.insert(2);
				selection->items.insert(4);

				// ACT / ASSERT (no selection model)
				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 == (LVIS_SELECTED & nmlvdi.item.state));

				// INIT
				lv.first->set_selection_model(selection);

				// ACT / ASSERT
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 == (LVIS_SELECTED & nmlvdi.item.state));

				nmlvdi.item.iItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 == (LVIS_SELECTED & nmlvdi.item.state));

				nmlvdi.item.iItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 != (LVIS_SELECTED & nmlvdi.item.state));

				nmlvdi.item.iItem = 3;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 == (LVIS_SELECTED & nmlvdi.item.state));

				nmlvdi.item.iItem = 4;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 != (LVIS_SELECTED & nmlvdi.item.state));

				// INIT
				selection->items.insert(1);

				// ACT / ASSERT
				nmlvdi.item.iItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_is_true(0 != (LVIS_SELECTED & nmlvdi.item.state));
			}


			test( ClickingAColumnCausesColumnActivation )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
					{	"", 10	},
				};
				mocks::columns_model_ptr cm(mocks::headers_model::create(columns, headers_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(0));
				NMLISTVIEW nmlvdi = {
					{	0, 0, LVN_COLUMNCLICK	},
					/* iItem = */ LVIF_TEXT /* see if wndproc differentiates notifications */, /* iSubItem = */ 0,
				};

				lv.first->set_columns_model(cm);
				lv.first->set_model(m);

				// ACT
				nmlvdi.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

				// ASSERT
				assert_equal(1u, cm->column_activation_log.size());
				assert_equal(2u, cm->column_activation_log[0]);

				// ACT
				nmlvdi.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				nmlvdi.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

				// ASSERT
				assert_equal(3u, cm->column_activation_log.size());
				assert_equal(0u, cm->column_activation_log[1]);
				assert_equal(1u, cm->column_activation_log[2]);
			}


			test( PreorderingHandlesNullModelWell )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
				};
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create(columns, 1, false));

				// ACT / ASSERT (must not throw)
				lv.first->set_model(shared_ptr<richtext_table_model>());
			}


			test( SettingColumnsModelWhithOrderingWhenModelIsMissingIsOkay )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
				};
				mocks::columns_model_ptr cm(mocks::headers_model::create(columns, 1, true));

				// ACT / ASSERT (must not fail)
				lv.first->set_columns_model(cm);
			}


			test( ColumnMarkerIsSetOnResettingColumnsModel )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
				};
				mocks::columns_model_ptr cm1(mocks::headers_model::create(columns, headers_model::npos(), false));
				mocks::columns_model_ptr cm2(mocks::headers_model::create(columns, 0, false));
				mocks::columns_model_ptr cm3(mocks::headers_model::create(columns, 1, true));

				// ACT
				lv.first->set_columns_model(cm1);

				// ASSERT
				assert_equal(dir_none, get_column_direction(lv.second, 0));
				assert_equal(dir_none, get_column_direction(lv.second, 1));

				// ACT
				lv.first->set_columns_model(cm2);

				// ASSERT
				assert_equal(dir_descending, get_column_direction(lv.second, 0));
				assert_equal(dir_none, get_column_direction(lv.second, 1));

				// ACT
				lv.first->set_columns_model(cm3);

				// ASSERT
				assert_equal(dir_none, get_column_direction(lv.second, 0));
				assert_equal(dir_ascending, get_column_direction(lv.second, 1));
			}


			test( ColumnMarkerIsSetOnSortChange )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
				};
				mocks::columns_model_ptr cm(mocks::headers_model::create(columns, headers_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);
				lv.first->set_columns_model(cm);

				// ACT
				cm->set_sort_order(0, true);

				// ASSERT
				assert_equal(dir_ascending, get_column_direction(lv.second, 0));
				assert_equal(dir_none, get_column_direction(lv.second, 1));

				// ACT
				cm->set_sort_order(1, false);

				// ASSERT
				assert_equal(dir_none, get_column_direction(lv.second, 0));
				assert_equal(dir_descending, get_column_direction(lv.second, 1));

				// ACT
				cm->set_sort_order(1, true);

				// ASSERT
				assert_equal(dir_none, get_column_direction(lv.second, 0));
				assert_equal(dir_ascending, get_column_direction(lv.second, 1));
			}


			test( ColumnMarkersAreChangedOnSortChangeWhenSortedColumnsModelWasInitiallySet )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"", 10	},
					{	"", 10	},
				};
				mocks::columns_model_ptr cm(mocks::headers_model::create(columns, 1, false));

				lv.first->set_columns_model(cm);

				// ACT
				cm->set_sort_order(0, true);

				// ASSERT
				assert_equal(dir_ascending, get_column_direction(lv.second, 0));
				assert_equal(dir_none, get_column_direction(lv.second, 1));
			}


			test( ItemActivationFiresCorrespondingEvent )
			{
				// INIT
				vector<string_table_model::index_type> selections;
				auto lv = create_listview();
				slot_connection c =
					lv.first->item_activate += bind(&push_back<string_table_model::index_type>, ref(selections), _1);
				NMITEMACTIVATE nm = {	{	0, 0, LVN_ITEMACTIVATE	},	};

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(10)));

				// ACT
				nm.iItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));

				// ASSERT
				assert_equal(1u, selections.size());
				assert_equal(1u, selections[0]);

				// ACT
				nm.iItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));
				nm.iItem = 3;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));
				nm.iItem = 5;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));

				// ASSERT
				assert_equal(4u, selections.size());
				assert_equal(0u, selections[1]);
				assert_equal(3u, selections[2]);
				assert_equal(5u, selections[3]);
			}


			test( ItemSelectionsModifySelectionModel )
			{
				// INIT
				auto lv = create_listview();
				NMLISTVIEW nmlv = {
					{	0, 0, LVN_ITEMCHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				lv.first->set_columns_model(mocks::headers_model::create("zzz", 100));
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(100)));
				lv.first->set_selection_model(selection);

				// ACT
				nmlv.iItem = 0, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 3, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equivalent(plural + 0u + 3u, selection->items);

				// ACT
				nmlv.iItem = 9, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 4, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 0, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equivalent(plural + 3u + 9u + 4u, selection->items);

				// ACT
				nmlv.iItem = -1, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				
				// ASSERT
				assert_is_empty(selection->items);
			}


			test( ItemMultiSelectionsModifySelectionModel )
			{
				// INIT
				auto lv = create_listview();
				NMLVODSTATECHANGE nmlv = {
					{	0, 0, LVN_ODSTATECHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				lv.first->set_columns_model(mocks::headers_model::create("zzz", 100));
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(100)));
				lv.first->set_selection_model(selection);

				// ACT
				nmlv.iFrom = 1, nmlv.iTo = 3, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equivalent(plural + 1u + 2u + 3u, selection->items);

				// ACT
				nmlv.iFrom = 7, nmlv.iTo = 13, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equivalent(plural + 1u + 2u + 3u + 7u + 8u + 9u + 10u + 11u + 12u + 13u, selection->items);

				// ACT
				nmlv.iFrom = 8, nmlv.iTo = 11, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equivalent(plural + 1u + 2u + 3u + 7u + 12u + 13u, selection->items);
			}


			test( ColumnWidthIsSetInPixelsIfSpecifiedInColumn )
			{
				// INIT
				auto lv1 = create_listview();
				auto lv2 = create_listview();
				column_t columns3[] = {
					{	"one", 13	},
					{	"two", 17	},
					{	"three", 19	},
				};
				column_t columns2[] = {
					{	"Pears", 23	},
					{	"Apples", 29	},
				};

				lv1.first->set_columns_model(mocks::headers_model::create(columns3, headers_model::npos(), false));
				lv2.first->set_columns_model(mocks::headers_model::create(columns2, headers_model::npos(), false));

				// ACT
				int w10 = get_column_width(lv1.second, 0);
				int w11 = get_column_width(lv1.second, 1);
				int w12 = get_column_width(lv1.second, 2);
				int w20 = get_column_width(lv2.second, 0);
				int w21 = get_column_width(lv2.second, 1);

				// ASSERT
				assert_equal(13, w10);
				assert_equal(17, w11);
				assert_equal(19, w12);

				assert_equal(23, w20);
				assert_equal(29, w21);
			}


			test( ChangingColumnWidthUpdatesColumnsModel )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = {
					{	"one", 13	},
					{	"two", 17	},
					{	"three", 19	},
				};
				mocks::columns_model_ptr cm = mocks::headers_model::create(columns, headers_model::npos(), false);
				HDITEM item = {
					HDI_WIDTH,
				};
				NMHEADER nmheader = {
					{ ListView_GetHeader(lv.second), static_cast<UINT_PTR>(::GetDlgCtrlID(ListView_GetHeader(lv.second))), HDN_ITEMCHANGED },
					0,
					0,
					&item
				};

				lv.first->set_columns_model(cm);

				// ACT
				nmheader.iItem = 0;
				item.cxy = 111;
				::SendMessage(lv.second, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmheader));

				// ASSERT
				assert_equal(111, cm->columns[0].width);

				// ACT
				nmheader.iItem = 2;
				item.cxy = 313;
				::SendMessage(lv.second, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmheader));

				// ASSERT
				assert_equal(313, cm->columns[2].width);
			}


			test( NonColumnWidthChangeDoesNotAffectColumnsModel )
			{
				// INIT
				auto lv = create_listview();
				column_t columns[] = { {	"one", 13	},	};
				mocks::columns_model_ptr cm = mocks::headers_model::create(columns, headers_model::npos(), false);
				HDITEM item = {
					HDI_TEXT,
				};
				NMHEADER nmheader = {
					{ ListView_GetHeader(lv.second), static_cast<UINT_PTR>(::GetDlgCtrlID(ListView_GetHeader(lv.second))), HDN_ITEMCHANGED },
					0,
					0,
					&item
				};

				lv.first->set_columns_model(cm);

				// ACT
				nmheader.iItem = 0;
				item.cxy = 111;
				::SendMessage(lv.second, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmheader));

				// ASSERT
				assert_equal(13, cm->columns[0].width);
			}


			test( SelectionIsEmptyAtConstruction )
			{
				// INIT
				auto lv = create_listview();

				// ACT
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(7)));

				// ASSERT
				assert_is_empty(get_matching_indices(lv.second, LVNI_SELECTED));
			}


			test( RequestProperTrackableOnFocusChange )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				NMLISTVIEW nmlv = {
					{	0, 0, LVN_ITEMCHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));

				// ACT
				nmlv.iItem = 2, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equal(1u, m->tracking_requested.size());
				assert_equal(2u, m->tracking_requested[0]);

				// ACT
				nmlv.iItem = 3, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equal(2u, m->tracking_requested.size());
				assert_equal(3u, m->tracking_requested[1]);

				// ACT
				nmlv.iItem = 5, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = LVIS_SELECTED | LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				ListView_SetItemState(lv.second, 7, LVIS_FOCUSED, LVIS_FOCUSED);

				// ASSERT
				assert_equal(4u, m->tracking_requested.size());
				assert_equal(5u, m->tracking_requested[2]);
				assert_equal(7u, m->tracking_requested[3]);
			}


			test( NotEnteringToFocusedDoesNotLeadToTracking )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				NMLISTVIEW nmlv = {
					{	0, 0, LVN_ITEMCHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));

				// ACT
				nmlv.iItem = 2, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 3, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 2, nmlv.uOldState = LVIS_SELECTED | LVIS_FOCUSED, nmlv.uNewState = LVIS_SELECTED | LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 3, nmlv.uOldState = LVIS_ACTIVATING | LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_is_empty(m->tracking_requested);
			}


			test( TakeOwnershipOverTrackable )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				shared_ptr<const trackable> t(new mocks::listview_trackable());
				weak_ptr<const trackable> wt(t);
				vector<int> matched;

				lv.first->set_model(m);
				swap(m->trackables[5], t);

				// ACT
				ListView_SetItemState(lv.second, 5, LVIS_FOCUSED, LVIS_FOCUSED);
				m->trackables.clear();
				t = mocks::trackable_ptr();

				// ASSERT
				assert_is_false(wt.expired());
			}


			test( ReleaseTrackableOnRemoveFocus )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				weak_ptr<const trackable> wt(mocks::listview_trackable::add(m->trackables, 5));
				vector<int> matched;

				lv.first->set_model(m);
				ListView_SetItemState(lv.second, 5, LVIS_FOCUSED, LVIS_FOCUSED);
				m->trackables.clear();

				// ACT
				ListView_SetItemState(lv.second, 5, 0, LVIS_FOCUSED);

				// ASSERT
				assert_is_true(wt.expired());
			}


			test( ReleaseTrackableOnNewFocus )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				weak_ptr<const trackable> wt(mocks::listview_trackable::add(m->trackables, 5));
				vector<int> matched;

				lv.first->set_model(m);
				ListView_SetItemState(lv.second, 5, LVIS_FOCUSED, LVIS_FOCUSED);
				m->trackables.clear();

				// ACT
				ListView_SetItemState(lv.second, 7, LVIS_FOCUSED, LVIS_FOCUSED);

				// ASSERT
				assert_is_true(wt.expired());
			}


			test( ResetFocusOnInvalidation )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 5));
				vector<size_t> matched;

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 5, LVIS_FOCUSED, LVIS_FOCUSED);
				m->tracking_requested.clear();

				// ACT
				t->track_result = 7;
				m->invalidate(100);

				// ASSERT
				matched = get_matching_indices(lv.second, LVNI_FOCUSED);
				assert_equal(1u, matched.size());
				assert_equal(7u, matched[0]);
				assert_is_empty(m->tracking_requested);

				// ACT
				t->track_result = 13;
				m->invalidate(100);

				// ASSERT
				matched = get_matching_indices(lv.second, LVNI_FOCUSED);
				assert_equal(1u, matched.size());
				assert_equal(13u, matched[0]);
				assert_is_empty(m->tracking_requested);
			}


			test( ReleaseTrackableOnBecameInvalid )
			{
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 5));
				weak_ptr<const trackable> wt(t);
				vector<int> matched;

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 5, LVIS_FOCUSED, LVIS_FOCUSED);
				m->trackables.clear();

				// ACT
				t->track_result = string_table_model::npos();
				t.reset();
				m->invalidate(100);

				// ASSERT
				assert_is_true(wt.expired());
				assert_is_empty(get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( IgnoreNullTrackableForSelected )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
					
				lv.first->set_model(m);
				ListView_SetItemState(lv.second, 7, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 8, LVIS_SELECTED, LVIS_SELECTED);

				// ACT / ASSERT (must not throw)
				m->set_count(100);

				// ASSERT
				string_table_model::index_type expected[] = {	7, 8,	};

				assert_equivalent(expected, get_matching_indices(lv.second, LVNI_SELECTED));
			}


			test( ReleaseTrackablesOnModelChange )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				weak_ptr<const trackable> wt[] = {
					mocks::listview_trackable::add(m->trackables, 4),
					mocks::listview_trackable::add(m->trackables, 8),
					mocks::listview_trackable::add(m->trackables, 16),
				};

				lv.first->set_model(m);
				ListView_SetItemState(lv.second, 4, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 8, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 16, LVIS_FOCUSED, LVIS_FOCUSED);
				m->trackables.clear();

				// ACT
				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(130)));

				// ASSERT
				assert_is_true(wt[0].expired());
				assert_is_true(wt[1].expired());
				assert_is_true(wt[2].expired());
			}


			test( FocusingItemChangesListViewItemState )
			{
				// INIT
				auto lv = create_listview();

				lv.first->set_model(mocks::autotrackable_table_model_ptr(new mocks::autotrackable_table_model(100, 1)));
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));

				// ACT
				lv.first->focus(99);

				// ASSERT
				string_table_model::index_type reference1[] = { 99u, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				lv.first->focus(49);

				// ASSERT
				string_table_model::index_type reference2[] = { 49u, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				lv.first->focus(string_table_model::npos());

				// ASSERT
				assert_is_empty(get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( FocusingItemMakesItVisisble )
			{
				// INIT
				auto lv = create_listview();

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(100, 1)));
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));

				// ACT
				lv.first->focus(99);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				// ACT
				lv.first->focus(49);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				// ACT
				lv.first->focus(0);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_true(is_item_visible(lv.second, 0));
			}


			test( CaptureTrackableOnFocusingItem )
			{
				// INIT
				auto lv = create_listview();
				mocks::autotrackable_table_model_ptr m(new mocks::autotrackable_table_model(100, 1));

				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->set_model(m);

				// ACT
				lv.first->focus(0);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(0));

				// ACT
				lv.first->focus(1);
				lv.first->focus(7);

				// ASSERT
				assert_equal(1u, m->auto_trackables->size());
				assert_equal(1u, m->auto_trackables->count(7));
			}


			test( ReleaseTrackableOnChangingFocus )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				weak_ptr<const trackable> wt[] = {
					mocks::listview_trackable::add(m->trackables, 4),
					mocks::listview_trackable::add(m->trackables, 7),
				};

				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->set_model(m);

				// ACT
				lv.first->focus(4);
				lv.first->focus(7);
				m->trackables.clear();

				// ASSERT
				assert_is_true(wt[0].expired());
				assert_is_false(wt[1].expired());
			}


			test( ReleaseFocusTrackableOnChangingModel )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m1(new mocks::listview_model(100, 1)), m2(new mocks::listview_model(100, 1));
				weak_ptr<const trackable> wt = mocks::listview_trackable::add(m1->trackables, 5);

				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->set_model(m1);
				lv.first->focus(5);
				m1->trackables.clear();

				// ACT
				lv.first->set_model(m2);

				// ASSERT
				assert_is_true(wt.expired());
			}


			test( EnsureItemVisibilityInDynamics )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 0));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->focus(0);

				// ACT
				t->track_result = 99;
				m->set_count(100);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				string_table_model::index_type reference1[] = { 99, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				t->track_result = 49;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				string_table_model::index_type reference2[] = { 49, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				t->track_result = 0;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_true(is_item_visible(lv.second, 0));

				string_table_model::index_type reference3[] = { 0, };

				assert_equal(reference3, get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( VisibilityTrackingBreaksWhenItemIsScrolledOut1 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 0));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->focus(0);

				// ACT
				ListView_EnsureVisible(lv.second, 49, FALSE);
				t->track_result = 99;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 0));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));

				string_table_model::index_type reference1[] = { 99, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				ListView_EnsureVisible(lv.second, 0, FALSE);
				t->track_result = 49;
				m->set_count(100);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 0));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));

				string_table_model::index_type reference2[] = { 49, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( VisibilityTrackingBreaksWhenItemIsScrolledOut2 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 49));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->focus(49);

				// ACT
				ListView_EnsureVisible(lv.second, 0, FALSE);
				t->track_result = 99;
				m->set_count(100);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 0));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));
			}


			test( VisibilityTrackingIsRestoredWhenFocusBecomesVisisble )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 49));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->focus(49);
				ListView_EnsureVisible(lv.second, 0, FALSE);
				t->track_result = 99;
				m->set_count(100);

				// ACT
				ListView_EnsureVisible(lv.second, 99, FALSE);
				t->track_result = 49;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 0));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));
			}


			test( FirstVisibleItemIsNotObscuredByHeader )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(10, 2));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 9));
				column_t columns[] = {
					{	"iiii", 50	},
					{	"wwwwwwwwwwwww", 100	},
				};

				lv.first->set_columns_model(mocks::headers_model::create(columns, headers_model::npos(), false));
				lv.first->set_model(m);

				::SetWindowLong(lv.second, GWL_STYLE, ::GetWindowLong(lv.second, GWL_STYLE) | LVS_NOCOLUMNHEADER);
				DWORD dwsize = ListView_ApproximateViewRect(lv.second, -1, -1, -1);
				::SetWindowLong(lv.second, GWL_STYLE, ::GetWindowLong(lv.second, GWL_STYLE) & ~LVS_NOCOLUMNHEADER);

				::MoveWindow(::GetParent(lv.second), 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);
				::MoveWindow(lv.second, 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);

				// ACT
				lv.first->focus(9);
				int first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_not_equal(0, first_visible);

				// ACT
				t->track_result = 0;
				m->set_count(10);
				first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_equal(0, first_visible);

				// INIT (check that header location mapping is working)
				::MoveWindow(::GetParent(lv.second), 37, 71, LOWORD(dwsize), HIWORD(dwsize), TRUE);

				// ACT
				lv.first->focus(9);
				first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_not_equal(0, first_visible);

				// ACT
				t->track_result = 0;
				m->set_count(10);
				first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_equal(0, first_visible);
			}


			test( ListViewIsNotScrolledIfNoHeaderAndViewRectMatchesNumberOfItems )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(10, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 9));

				lv.first->set_columns_model(mocks::headers_model::create("iiii", 50));
				lv.first->set_model(m);

				::SetWindowLong(lv.second, GWL_STYLE, ::GetWindowLong(lv.second, GWL_STYLE) | LVS_NOCOLUMNHEADER);
				DWORD dwsize = ListView_ApproximateViewRect(lv.second, -1, -1, -1);

				::MoveWindow(::GetParent(lv.second), 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);
				::MoveWindow(lv.second, 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);

				// ACT
				lv.first->focus(9);
				int first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_equal(0, first_visible);

				// ACT
				t->track_result = 0;
				m->set_count(10);
				first_visible = ListView_GetTopIndex(lv.second);

				// ASSERT
				assert_equal(0, first_visible);
			}


			test( ListViewDoesNotCreateAWindowOnConstruction )
			{
				// INIT
				window_tracker wt;

				wt.checkpoint();

				// INIT / ACT
				shared_ptr<listview> lv(new win32::listview);

				// ASSERT
				assert_not_null(lv);
				wt.checkpoint();

				assert_is_empty(wt.find_created(WC_LISTVIEWW));
			}


			test( ListViewIsCreatedOnGettingAWindow )
			{
				// INIT
				window_tracker wt;
				shared_ptr<listview> lv(new win32::listview);
				vector<placed_view> v;
				agge::box<int> b = { 100, 300 };

				// ACT
				lv->layout(make_appender(v), b);

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, v.size());
				assert_not_null(v[0].native);
				assert_is_empty(wt.find_created(WC_LISTVIEW));

				// ACT
				HWND hlv = v[0].native->get_window(hparent);

				// ASSERT
				wt.checkpoint();
				vector<HWND> created = wt.find_created(WC_LISTVIEWW);
				assert_equal(1u, created.size());
				assert_equal(created[0], hlv);
			}


			test( ListViewWindowReceivesExpectedStyles )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);

				// ACT
				HWND hwnd = get_window_and_resize(lv, hparent);

				// ASSERT
				enum {
					expected_style = LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | WS_BORDER,
					expected_lv_style = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,
				};

				assert_equal(expected_style, expected_style & ::GetWindowLongA(hwnd, GWL_STYLE));
				assert_equal((DWORD)expected_lv_style, expected_lv_style & ListView_GetExtendedListViewStyle(hwnd));
			}


			test( ColumnsAreSetupAccordinglyToTheModelOnWindowCreation )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);
				HWND hparent2 = create_parent_window();
				column_t columns1[] = {
					{	"iiii", 23	},
					{	"wwwwwwwwwwwww", 100	},
				};
				column_t columns2[] = {
					{	"a", 130	},
					{	"w", 100	},
					{	"x", 50	},
				};

				lv->set_columns_model(mocks::headers_model::create(columns1, headers_model::npos(), false));

				// ACT
				HWND hwnd = get_window_and_resize(lv, hparent);

				// ASSERT
				assert_equal(2u, get_columns_count(hwnd));
				assert_equal("iiii", get_column_text(hwnd, 0));
				assert_equal(23, get_column_width(hwnd, 0));
				assert_equal(dir_none, get_column_direction(hwnd, 0));
				assert_equal("wwwwwwwwwwwww", get_column_text(hwnd, 1));
				assert_equal(100, get_column_width(hwnd, 1));
				assert_equal(dir_none, get_column_direction(hwnd, 1));

				// INIT / ACT
				lv->set_columns_model(mocks::headers_model::create(columns2, 2, false));

				// ACT
				HWND hwnd2 = get_window_and_resize(lv, hparent2);

				// ASSERT
				assert_equal(3u, get_columns_count(hwnd2));
				assert_equal("a", get_column_text(hwnd2, 0));
				assert_equal(130, get_column_width(hwnd2, 0));
				assert_equal(dir_none, get_column_direction(hwnd2, 0));
				assert_equal("w", get_column_text(hwnd2, 1));
				assert_equal("x", get_column_text(hwnd2, 2));
				assert_equal(dir_descending, get_column_direction(hwnd2, 2));
			}


			test( ItemCountIsSetupAccordinglyToTheModelOnWindowCreation )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);
				HWND hparent2 = create_parent_window();
				shared_ptr<richtext_table_model> model1(new mocks::listview_model(5, 1));
				shared_ptr<richtext_table_model> model2(new mocks::listview_model(1311, 1));

				lv->set_columns_model(mocks::headers_model::create("Name", 100));
				lv->set_model(model1);

				// ACT
				HWND hwnd = get_window_and_resize(lv, hparent);

				// ASSERT
				assert_equal(5, ListView_GetItemCount(hwnd));

				// INIT
				lv->set_model(model2);

				// ACT
				HWND hwnd2 = get_window_and_resize(lv, hparent2);

				// ASSERT
				assert_equal(1311, ListView_GetItemCount(hwnd2));
			}


			test( IndependenttListViewHandlesNativeNotifications )
			{
				// INIT
				window_tracker wt;
				shared_ptr<listview> lv(new win32::listview);
				HWND hlv = get_window_and_resize(lv, hparent);

				wt.checkpoint();

				RECT rc;
				column_t columns[] = {
					{	"", 100	},
					{	"", 100	},
					{	"", 50	},
				};
				mocks::columns_model_ptr cm(mocks::headers_model::create(columns, headers_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(3, 3));

				::MoveWindow(hlv, 0, 0, 300, 200, TRUE);
				lv->set_columns_model(cm);
				lv->set_model(m);
				lv->set_selection_model(selection);

				// ACT
				ListView_GetItemRect(hlv, 0, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);

				// ASSERT
				assert_equivalent(plural + 0u, selection->items);

				// ACT
				ListView_GetItemRect(hlv, 2, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);

				// ASSERT
				assert_equivalent(plural + 2u, selection->items);

				// ACT
				ListView_GetItemRect(hlv, 1, &rc, LVIR_SELECTBOUNDS);
				emulate_click(hlv, rc.left, rc.top, mouse_input::left, 0);

				// ASSERT
				assert_equivalent(plural + 1u, selection->items);
			}
		end_test_suite
	}
}

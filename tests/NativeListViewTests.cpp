#include <wpl/win32/listview.h>

#include "helpers.h"
#include "helpers-visual.h"
#include "helpers-win32.h"
#include "MockupsListView.h"

#include <windows.h>
#include <commctrl.h>
#include <map>
#include <olectl.h>
#include <tchar.h>
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
			typedef columns_model::column column_def;

			enum sort_direction	{	dir_none, dir_ascending, dir_descending	};

			template <typename T>
			void push_back(vector<T> &v, const T &value)
			{	v.push_back(value);	}

			size_t get_columns_count(HWND hlv)
			{	return Header_GetItemCount(ListView_GetHeader(hlv));	}

			sort_direction get_column_direction(HWND hlv, columns_model::index_type column)
			{
				HDITEM item = { };
				HWND hheader = ListView_GetHeader(hlv);
					
				item.mask = HDI_FORMAT;
				Header_GetItem(hheader, column, &item);
				return (item.fmt & HDF_SORTUP) ? dir_ascending : (item.fmt & HDF_SORTDOWN) ? dir_descending : dir_none;
			}

			basic_string<TCHAR> get_column_text(HWND hlv, columns_model::index_type column)
			{
				HDITEM item = { };
				HWND hheader = ListView_GetHeader(hlv);
				vector<TCHAR> buffer(item.cchTextMax = 100);
					
				item.mask = HDI_TEXT;
				item.pszText = &buffer[0];
				Header_GetItem(hheader, column, &item);
				return item.pszText;
			}

			int get_column_width(HWND hlv, columns_model::index_type column)
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

		begin_test_suite( NativeListViewTests )

			window_manager windowManager;
			HWND hparent;

			init( Initialize )
			{
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
				column_def columns[] = {
					column_def(L"Contract"),
					column_def(L"Price"),
					column_def(L"Volume"),
				};
					
				// ACT
				lv.first->set_columns_model(mocks::columns_model::create(columns, columns_model::npos(), false));

				// ASSERT
				assert_equal(3u, get_columns_count(lv.second));
				assert_equal(_T("Contract"), get_column_text(lv.second, 0));
				assert_equal(_T("Price"), get_column_text(lv.second, 1));
				assert_equal(_T("Volume"), get_column_text(lv.second, 2));
			}


			test( ColumnsAreResetOnNewColumnsModelSetting )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"Contract"),
					column_def(L"Price"),
				};
					
				// ACT
				lv.first->set_columns_model(mocks::columns_model::create(columns, columns_model::npos(), false));
				lv.first->set_columns_model(mocks::columns_model::create(L"Team"));

				// ASSERT
				assert_equal(1u, get_columns_count(lv.second));
				assert_equal(_T("Team"), get_column_text(lv.second, 0));
			}


			test( ListViewInvalidatedOnModelInvalidate )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(11, 1));

				lv.first->set_columns_model(mocks::columns_model::create(L"test", 13));
				lv.first->set_model(m);

				// ACT
				::UpdateWindow(lv.second);

				// ASSERT
				assert_is_false(!!::GetUpdateRect(lv.second, NULL, FALSE));

				// ACT
				m->invalidated(11);

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
				lv.first->set_model(shared_ptr<table_model>());

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
				m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
				lv.first->set_model(m);

				// ACT / ASSERT (must not throw)
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
			}


			test( GettingItemTextNoTruncationANSI )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));
				char buffer[100] = { 0 };
				NMLVDISPINFOA nmlvdi = {
					{	0, 0, LVN_GETDISPINFOA	},
					{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
				};

				m->set_count(3), m->items[0].resize(3), m->items[1].resize(3), m->items[2].resize(3);
				m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
				m->items[1][0] = L"four", m->items[1][1] = L"five", m->items[1][2] = L"six";
				m->items[2][0] = L"seven", m->items[2][1] = L"eight", m->items[2][2] = L"NINE";
				lv.first->set_model(m);

				// ACT / ASSERT
				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("one", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("two", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("three", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("four", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("five", buffer));

				nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("six", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("seven", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("eight", buffer));

				nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("NINE", buffer));
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
				m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
				m->items[1][0] = L"four", m->items[1][1] = L"five", m->items[1][2] = L"six";
				m->items[2][0] = L"seven", m->items[2][1] = L"eight", m->items[2][2] = L"NINE";
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


			test( GettingItemTextWithTruncationANSI )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(0));
				char buffer[4] = { 0 };
				NMLVDISPINFOA nmlvdi = {
					{	0, 0, LVN_GETDISPINFOA	},
					{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
				};

				m->set_count(3), m->items[0].resize(3);
				m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
				lv.first->set_model(m);

				// ACT / ASSERT
				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("one", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("two", buffer));

				nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				assert_equal(0, strcmp("thr", buffer));
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
				m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
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


			test( ModelIsOrderedAccordinglyToColumnsModelSet )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);

				// ACT
				lv.first->set_columns_model(mocks::columns_model::create(columns, 0, true));

				// ASSERT
				assert_equal(1u, m->ordering.size());
				assert_equal(0u, m->ordering[0].first);
				assert_equal(true, m->ordering[0].second);

				// ACT
				lv.first->set_columns_model(mocks::columns_model::create(columns, 2, false));
				lv.first->set_columns_model(mocks::columns_model::create(columns, 1, true));

				// ASSERT
				assert_equal(3u, m->ordering.size());
				assert_equal(2u, m->ordering[1].first);
				assert_equal(false, m->ordering[1].second);
				assert_equal(1u, m->ordering[2].first);
				assert_equal(true, m->ordering[2].second);
			}


			test( ModelIsNotOrderedIfColumnsModelIsNotOrdered )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);

				// ACT
				lv.first->set_columns_model(mocks::columns_model::create(columns, columns_model::npos(), true));

				// ASSERT
				assert_is_empty(m->ordering);
			}


			test( ModelIsSortedOnSortOrderChangedEvent )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::model_ptr m(new mocks::listview_model(0));
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));

				lv.first->set_model(m);
				lv.first->set_columns_model(cm);

				// ACT
				cm->set_sort_order(1, false);

				// ASSERT
				assert_equal(1u, m->ordering.size());
				assert_equal(1u, m->ordering[0].first);
				assert_equal(false, m->ordering[0].second);

				// ACT
				cm->set_sort_order(2, false);
				cm->set_sort_order(0, true);

				// ASSERT
				assert_equal(3u, m->ordering.size());
				assert_equal(2u, m->ordering[1].first);
				assert_equal(false, m->ordering[1].second);
				assert_equal(0u, m->ordering[2].first);
				assert_equal(true, m->ordering[2].second);
			}


			test( ClickingAColumnCausesColumnActivation )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));
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
				assert_equal(2, cm->column_activation_log[0]);

				// ACT
				nmlvdi.iSubItem = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				nmlvdi.iSubItem = 1;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

				// ASSERT
				assert_equal(3u, cm->column_activation_log.size());
				assert_equal(0, cm->column_activation_log[1]);
				assert_equal(1, cm->column_activation_log[2]);
			}


			test( PreorderingOnChangingModel )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::model_ptr m1(new mocks::listview_model(0)), m2(new mocks::listview_model(0));
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, 0, true));

				lv.first->set_model(m1);
				lv.first->set_columns_model(cm);

				// ACT
				lv.first->set_model(m2);

				// ASSERT
				assert_equal(1u, m2->ordering.size());
				assert_equal(0u, m2->ordering[0].first);
				assert_is_true(m2->ordering[0].second);

				// INIT
				cm->set_sort_order(1, false);
				m1->ordering.clear();

				// ACT
				lv.first->set_model(m1);

				// ASSERT
				assert_equal(1u, m1->ordering.size());
				assert_equal(1u, m1->ordering[0].first);
				assert_is_false(m1->ordering[0].second);
			}


			test( PreorderingHandlesNullModelWell )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::model_ptr m(new mocks::listview_model(0));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::columns_model::create(columns, 1, false));

				// ACT / ASSERT (must not throw)
				lv.first->set_model(shared_ptr<table_model>());
			}


			test( SettingColumnsModelWhithOrderingWhenModelIsMissingIsOkay )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, 1, true));

				// ACT / ASSERT (must not fail)
				lv.first->set_columns_model(cm);
			}


			test( ColumnMarkerIsSetOnResettingColumnsModel )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::columns_model_ptr cm1(mocks::columns_model::create(columns, columns_model::npos(), false));
				mocks::columns_model_ptr cm2(mocks::columns_model::create(columns, 0, false));
				mocks::columns_model_ptr cm3(mocks::columns_model::create(columns, 1, true));

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
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));
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
				column_def columns[] = {
					column_def(L"", 10),
					column_def(L"", 10),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, 1, false));

				lv.first->set_columns_model(cm);

				// ACT
				cm->set_sort_order(0, true);

				// ASSERT
				assert_equal(dir_ascending, get_column_direction(lv.second, 0));
				assert_equal(dir_none, get_column_direction(lv.second, 1));
			}				


			test( TheWholeListViewIsInvalidatedOnReorder )
			{
				// INIT
				auto lv = create_listview();
				column_def columns[] = {
					column_def(L"a", 23),
					column_def(L"b", 15),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(1, 4));
				RECT rc_invalidated = { };

				lv.first->set_model(m);
				lv.first->set_columns_model(cm);
				::UpdateWindow(lv.second);

				// ACT
				cm->set_sort_order(1, true);

				// ASSERT
				::GetUpdateRect(lv.second, &rc_invalidated, FALSE);
				assert_equal(get_visible_items_rect(lv.second), rc_invalidated);

				// INIT
				m->set_count(3);
				::UpdateWindow(lv.second);

				// ACT
				cm->set_sort_order(0, false);

				// ASSERT
				::GetUpdateRect(lv.second, &rc_invalidated, FALSE);
				assert_equal(get_visible_items_rect(lv.second), rc_invalidated);
			}


			test( ItemActivationFiresCorrespondingEvent )
			{
				// INIT
				vector<table_model::index_type> selections;
				auto lv = create_listview();
				slot_connection c =
					lv.first->item_activate += bind(&push_back<table_model::index_type>, ref(selections), _1);
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


			test( ItemChangeWithSelectionRemainingDoesNotFireEvent )
			{
				// INIT
				vector<table_model::index_type> selection_indices;
				vector<bool> selection_states;
				auto lv = create_listview();
				slot_connection
					c1 = lv.first->selection_changed += bind(&push_back<table_model::index_type>, ref(selection_indices), _1),
					c2 = lv.first->selection_changed += bind(&push_back<bool>, ref(selection_states), _2);
				NMLISTVIEW nmlv = {
					{	0, 0, LVN_ITEMCHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				// ACT
				nmlv.iItem = 0, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 1, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 3, nmlv.uOldState = LVIS_FOCUSED | LVIS_SELECTED, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_is_empty(selection_indices);
			}


			test( ItemChangeWithSelectionChangingDoesFireEvent )
			{
				// INIT
				vector<table_model::index_type> selection_indices;
				vector<bool> selection_states;
				auto lv = create_listview();
				slot_connection
					c1 = lv.first->selection_changed += bind(&push_back<table_model::index_type>, ref(selection_indices), _1),
					c2 = lv.first->selection_changed += bind(&push_back<bool>, ref(selection_states), _2);
				NMLISTVIEW nmlv = {
					{	0, 0, LVN_ITEMCHANGED	},
					/* iItem = */ 0, /* iSubItem = */ 0,
				};

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(10)));

				// ACT
				nmlv.iItem = 1, nmlv.uOldState = LVIS_FOCUSED | LVIS_SELECTED, nmlv.uNewState = 0;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 2, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = LVIS_FOCUSED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
				nmlv.iItem = 7, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED | LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equal(3u, selection_indices.size());
				assert_equal(3u, selection_states.size());
				assert_equal(1u, selection_indices[0]);
				assert_is_false(selection_states[0]);
				assert_equal(2u, selection_indices[1]);
				assert_is_false(selection_states[1]);
				assert_equal(7u, selection_indices[2]);
				assert_is_true(selection_states[2]);

				// ACT
				nmlv.iItem = 9, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
				::SendMessage(lv.second, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

				// ASSERT
				assert_equal(4u, selection_indices.size());
				assert_equal(4u, selection_states.size());
				assert_equal(9u, selection_indices[3]);
				assert_is_true(selection_states[3]);
			}


			test( AutoAdjustColumnWidths )
			{
				// INIT
				auto lv1 = create_listview();
				auto lv2 = create_listview();
				column_def columns3[] = {
					column_def(L"_____ww"),
					column_def(L"_____wwwwww"),
					column_def(L"_____WW"),
				};
				column_def columns2[] = {
					column_def(L"_____ii"),
					column_def(L"_____iiii"),
				};

				lv1.first->set_columns_model(mocks::columns_model::create(columns3, columns_model::npos(), false));
				lv2.first->set_columns_model(mocks::columns_model::create(columns2, columns_model::npos(), false));

				// ACT
				lv1.first->adjust_column_widths();
				lv2.first->adjust_column_widths();

				// ASSERT
				int w10 = get_column_width(lv1.second, 0);
				int w11 = get_column_width(lv1.second, 1);
				int w12 = get_column_width(lv1.second, 2);
				int w20 = get_column_width(lv2.second, 0);
				int w21 = get_column_width(lv2.second, 1);

				assert_is_true(w10 < w11);
				assert_is_true(w12 < w11);
				assert_is_true(w10 < w12);

				assert_is_true(w20 < w10);
				assert_is_true(w20 < w21);
			}


			test( ColumnWidthIsSetInPixelsIfSpecifiedInColumn )
			{
				// INIT
				auto lv1 = create_listview();
				auto lv2 = create_listview();
				column_def columns3[] = {
					column_def(L"one", 13),
					column_def(L"two", 17),
					column_def(L"three", 19),
				};
				column_def columns2[] = {
					column_def(L"Pears", 23),
					column_def(L"Apples", 29),
				};

				lv1.first->set_columns_model(mocks::columns_model::create(columns3, columns_model::npos(), false));
				lv2.first->set_columns_model(mocks::columns_model::create(columns2, columns_model::npos(), false));

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
				column_def columns[] = {
					column_def(L"one", 13),
					column_def(L"two", 17),
					column_def(L"three", 19),
				};
				mocks::columns_model_ptr cm = mocks::columns_model::create(columns, columns_model::npos(), false);
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
				column_def columns[] = { column_def(L"one", 13),	};
				mocks::columns_model_ptr cm = mocks::columns_model::create(columns, columns_model::npos(), false);
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


			test( ResetSelection )
			{
				// INIT
				auto lv = create_listview();
				vector<size_t> selection;

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(7)));

				// ACT
				lv.first->select(0, true);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(1u, selection.size());
				assert_equal(0u, selection[0]);

				// ACT
				lv.first->select(3, true);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(1u, selection.size());
				assert_equal(3u, selection[0]);

				// ACT
				lv.first->select(5, true);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(1u, selection.size());
				assert_equal(5u, selection[0]);
			}


			test( AppendSelection )
			{
				// INIT
				auto lv = create_listview();
				vector<size_t> selection;

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(7)));

				// ACT
				lv.first->select(1, false);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(1u, selection.size());
				assert_equal(1u, selection[0]);

				// ACT
				lv.first->select(2, false);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(2u, selection.size());
				assert_equal(1u, selection[0]);
				assert_equal(2u, selection[1]);

				// ACT
				lv.first->select(6, false);

				// ASSERT
				selection = get_matching_indices(lv.second, LVNI_SELECTED);

				assert_equal(3u, selection.size());
				assert_equal(1u, selection[0]);
				assert_equal(2u, selection[1]);
				assert_equal(6u, selection[2]);
			}


			test( ClearNonEmptySelection )
			{
				// INIT
				auto lv = create_listview();

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(7)));

				// ACT
				lv.first->select(1, false);
				lv.first->select(table_model::npos(), true);

				// ASSERT
				assert_is_empty(get_matching_indices(lv.second, LVNI_SELECTED));

				// ACT
				lv.first->select(2, false);
				lv.first->select(3, false);
				lv.first->select(table_model::npos(), true);

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
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();

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
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();

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
				m->invalidated(100);

				// ASSERT
				matched = get_matching_indices(lv.second, LVNI_FOCUSED);
				assert_equal(1u, matched.size());
				assert_equal(7u, matched[0]);
				assert_is_empty(m->tracking_requested);

				// ACT
				t->track_result = 13;
				m->invalidated(100);

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
				t->track_result = table_model::npos();
				t.reset();
				m->invalidated(100);

				// ASSERT
				assert_is_true(wt.expired());
				assert_is_empty(get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( ResetSelectionOnInvalidation1 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t[] = {
					mocks::listview_trackable::add(m->trackables, 5),
					mocks::listview_trackable::add(m->trackables, 7),
					mocks::listview_trackable::add(m->trackables, 17),
				};
				vector<int> matched;

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 7, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 5, LVIS_SELECTED, LVIS_SELECTED);	// order is disturbed intentionally!
				ListView_SetItemState(lv.second, 17, LVIS_SELECTED, LVIS_SELECTED);
				m->tracking_requested.clear();

				// ACT
				t[2]->track_result = 21;
				m->invalidated(100);

				// ASSERT
				table_model::index_type expected1[] = {	5, 7, 21,	};

				assert_equivalent(expected1, get_matching_indices(lv.second, LVNI_SELECTED));

				// ACT
				t[0]->track_result = 13;
				m->invalidated(100);

				// ASSERT
				table_model::index_type expected2[] = {	13, 7, 21,	};

				assert_equivalent(expected2, get_matching_indices(lv.second, LVNI_SELECTED));

				// ACT
				t[1]->track_result = table_model::npos();
				m->invalidated(100);

				// ASSERT
				table_model::index_type expected3[] = {	13, 21,	};

				assert_equivalent(expected3, get_matching_indices(lv.second, LVNI_SELECTED));
				assert_is_empty(m->tracking_requested);
			}


			test( ResetSelectionOnInvalidation2 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t[] = {
					mocks::listview_trackable::add(m->trackables, 5),
					mocks::listview_trackable::add(m->trackables, 7),
					mocks::listview_trackable::add(m->trackables, 17),
					mocks::listview_trackable::add(m->trackables, 19),
					mocks::listview_trackable::add(m->trackables, 23),
				};
				vector<int> matched;

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 19, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 5, LVIS_SELECTED, LVIS_SELECTED);	// order is disturbed intentionally!
				ListView_SetItemState(lv.second, 17, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 7, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 23, LVIS_SELECTED, LVIS_SELECTED);
				m->tracking_requested.clear();

				// ACT
				t[2]->track_result = 3;
				t[4]->track_result = 47;
				m->set_count(100);

				// ASSERT
				table_model::index_type expected1[] = {	5, 7, 3, 19, 47,	};

				assert_equivalent(expected1, get_matching_indices(lv.second, LVNI_SELECTED));

				// ACT
				t[3]->track_result = 1;
				m->set_count(100);

				// ASSERT
				table_model::index_type expected2[] = {	5, 7, 3, 1, 47,	};

				assert_equivalent(expected2, get_matching_indices(lv.second, LVNI_SELECTED));
				assert_is_empty(m->tracking_requested);
			}


			test( ReleaseLostTrackables )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 4));
				weak_ptr<const trackable> wt[] = {
					t,
					mocks::listview_trackable::add(m->trackables, 8),
					mocks::listview_trackable::add(m->trackables, 16),
				};

				lv.first->set_model(m);
				ListView_SetItemState(lv.second, 4, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 8, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 16, LVIS_SELECTED, LVIS_SELECTED);
				m->trackables.clear();

				// ACT
				t->track_result = table_model::npos();
				t.reset();
				m->set_count(99);

				// ASSERT
				assert_is_true(wt[0].expired());
				assert_is_false(wt[1].expired());
				assert_is_false(wt[2].expired());
			}


			test( ReleaseTrackablesForDeselectedItems )
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
				ListView_SetItemState(lv.second, 16, LVIS_SELECTED, LVIS_SELECTED);
				m->trackables.clear();

				// ACT
				ListView_SetItemState(lv.second, 4, 0, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 16, 0, LVIS_SELECTED);

				// ASSERT
				assert_is_true(wt[0].expired());
				assert_is_false(wt[1].expired());
				assert_is_true(wt[2].expired());
			}


			test( AbandonDeselectedItemTracking )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t[] = {
					mocks::listview_trackable::add(m->trackables, 5),
					mocks::listview_trackable::add(m->trackables, 7),
					mocks::listview_trackable::add(m->trackables, 17),
				};

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 7, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 5, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 17, LVIS_SELECTED, LVIS_SELECTED);

				// ACT
				ListView_SetItemState(lv.second, 17, 0, LVIS_SELECTED);
				t[0]->track_result = 3;
				t[2]->track_result = 21;
				m->set_count(100);

				// ASSERT
				table_model::index_type expected1[] = {	3, 7,	};

				assert_equivalent(expected1, get_matching_indices(lv.second, LVNI_SELECTED));

				// ACT
				ListView_SetItemState(lv.second, 3, 0, LVIS_SELECTED);
				t[0]->track_result = 13;
				m->set_count(100);

				// ASSERT
				table_model::index_type expected2[] = {	7,	};

				assert_equivalent(expected2, get_matching_indices(lv.second, LVNI_SELECTED));
			}


			test( AbandonTrackingOnDeselectAllItems )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t[] = {
					mocks::listview_trackable::add(m->trackables, 5),
					mocks::listview_trackable::add(m->trackables, 7),
					mocks::listview_trackable::add(m->trackables, 17),
				};

				lv.first->set_model(m);

				ListView_SetItemState(lv.second, 7, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 5, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(lv.second, 17, LVIS_SELECTED, LVIS_SELECTED);
				m->tracking_requested.clear();

				// ACT
				ListView_SetItemState(lv.second, -1, 0, LVIS_SELECTED);
				t[0]->track_result = 3;
				t[0]->track_result = 4;
				t[2]->track_result = 21;
				m->set_count(100);

				// ASSERT
				assert_is_empty(get_matching_indices(lv.second, LVNI_SELECTED));
				assert_is_empty(m->tracking_requested);
			}


			test( FocusAndSelectionRequestTrackablesTwice )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 7));
					
				lv.first->set_model(m);
				m->tracking_requested.clear();

				// ACT
				ListView_SetItemState(lv.second, 7, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

				// ASSERT
				assert_equal(2u, m->tracking_requested.size());
				assert_equal(7u, m->tracking_requested[0]);
				assert_equal(7u, m->tracking_requested[1]);
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
				table_model::index_type expected[] = {	7, 8,	};

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
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();

				// ACT
				lv.first->focus(99);

				// ASSERT
				table_model::index_type reference1[] = { 99u, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				lv.first->focus(49);

				// ASSERT
				table_model::index_type reference2[] = { 49u, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				lv.first->focus(table_model::npos());

				// ASSERT
				assert_is_empty(get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( FocusingItemMakesItVisisble )
			{
				// INIT
				auto lv = create_listview();

				lv.first->set_model(mocks::model_ptr(new mocks::listview_model(100, 1)));
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();

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

				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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

				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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

				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
				lv.first->focus(0);

				// ACT
				t->track_result = 99;
				m->set_count(100);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				table_model::index_type reference1[] = { 99, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				t->track_result = 49;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 0));

				table_model::index_type reference2[] = { 49, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				t->track_result = 0;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 99));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_true(is_item_visible(lv.second, 0));

				table_model::index_type reference3[] = { 0, };

				assert_equal(reference3, get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( VisibilityTrackingBreaksWhenItemIsScrolledOut1 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 0));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
				lv.first->focus(0);

				// ACT
				ListView_EnsureVisible(lv.second, 49, FALSE);
				t->track_result = 99;
				m->set_count(100);

				// ASSERT
				assert_is_false(is_item_visible(lv.second, 0));
				assert_is_true(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));

				table_model::index_type reference1[] = { 99, };

				assert_equal(reference1, get_matching_indices(lv.second, LVNI_FOCUSED));

				// ACT
				ListView_EnsureVisible(lv.second, 0, FALSE);
				t->track_result = 49;
				m->set_count(100);

				// ASSERT
				assert_is_true(is_item_visible(lv.second, 0));
				assert_is_false(is_item_visible(lv.second, 49));
				assert_is_false(is_item_visible(lv.second, 99));

				table_model::index_type reference2[] = { 49, };

				assert_equal(reference2, get_matching_indices(lv.second, LVNI_FOCUSED));
			}


			test( VisibilityTrackingBreaksWhenItemIsScrolledOut2 )
			{
				// INIT
				auto lv = create_listview();
				mocks::model_ptr m(new mocks::listview_model(100, 1));
				mocks::trackable_ptr t(mocks::listview_trackable::add(m->trackables, 49));

				lv.first->set_model(m);
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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
				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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
				column_def columns[] = {
					column_def(L"iiii"),
					column_def(L"wwwwwwwwwwwww"),
				};

				lv.first->set_columns_model(mocks::columns_model::create(columns, columns_model::npos(), false));
				lv.first->adjust_column_widths();
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

				lv.first->set_columns_model(mocks::columns_model::create(L"iiii"));
				lv.first->adjust_column_widths();
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
				column_def columns1[] = {
					column_def(L"iiii", 23),
					column_def(L"wwwwwwwwwwwww", 100),
				};
				column_def columns2[] = {
					column_def(L"a", 130),
					column_def(L"w", 100),
					column_def(L"x", 50),
				};

				lv->set_columns_model(mocks::columns_model::create(columns1, columns_model::npos(), false));

				// ACT
				HWND hwnd = get_window_and_resize(lv, hparent);

				// ASSERT
				assert_equal(2u, get_columns_count(hwnd));
				assert_equal(L"iiii", get_column_text(hwnd, 0));
				assert_equal(23, get_column_width(hwnd, 0));
				assert_equal(dir_none, get_column_direction(hwnd, 0));
				assert_equal(L"wwwwwwwwwwwww", get_column_text(hwnd, 1));
				assert_equal(100, get_column_width(hwnd, 1));
				assert_equal(dir_none, get_column_direction(hwnd, 1));

				// INIT / ACT
				lv->set_columns_model(mocks::columns_model::create(columns2, 2, false));

				// ACT
				HWND hwnd2 = get_window_and_resize(lv, hparent2);

				// ASSERT
				assert_equal(3u, get_columns_count(hwnd2));
				assert_equal(L"a", get_column_text(hwnd2, 0));
				assert_equal(130, get_column_width(hwnd2, 0));
				assert_equal(dir_none, get_column_direction(hwnd2, 0));
				assert_equal(L"w", get_column_text(hwnd2, 1));
				assert_equal(L"x", get_column_text(hwnd2, 2));
				assert_equal(dir_descending, get_column_direction(hwnd2, 2));
			}


			test( ItemCountIsSetupAccordinglyToTheModelOnWindowCreation )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);
				HWND hparent2 = create_parent_window();
				shared_ptr<table_model> model1(new mocks::listview_model(5, 1));
				shared_ptr<table_model> model2(new mocks::listview_model(1311, 1));

				lv->set_columns_model(mocks::columns_model::create(L"Name", 100));
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


			test( SelectionIsPreservedOnWindowRecreation )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);
				HWND hparent2 = create_parent_window();
				shared_ptr<table_model> model(new mocks::listview_model(5, 1));

				lv->set_columns_model(mocks::columns_model::create(L"Name", 100));
				lv->set_model(model);

				HWND hwnd = get_window_and_resize(lv, hparent);

				::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) & ~LVS_SINGLESEL);

				lv->select(1, true);
				lv->select(3, false);

				// ACT
				hwnd = get_window_and_resize(lv, hparent2);

				// ASSERT
				vector<table_model::index_type> selection = get_matching_indices(hwnd, LVNI_SELECTED);
				table_model::index_type reference1[] = { 1, 3, };

				assert_equal(reference1, selection);

				// INIT
				lv->select(2, true);

				// ACT
				hwnd = get_window_and_resize(lv, hparent);

				// ASSERT
				selection = get_matching_indices(hwnd, LVNI_SELECTED);
				table_model::index_type reference2[] = { 2, };

				assert_equal(reference2, selection);
			}


			test( SelectingNPosAcquiresNoTrackable )
			{
				// INIT
				shared_ptr<listview> lv(new win32::listview);
				shared_ptr<mocks::listview_model> model(new mocks::listview_model(5, 1));
				vector<table_model::index_type> selections;

				lv->set_columns_model(mocks::columns_model::create(L"Name", 100));
				lv->set_model(model);

				HWND hwnd = get_window_and_resize(lv, hparent);

				::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) & ~LVS_SINGLESEL);

				lv->select(1, true);
				lv->select(3, false);
				model->tracking_requested.clear();

				// ACT
				lv->select(table_model::npos(), false);

				// ASSERT
				assert_is_empty(model->tracking_requested);
				assert_equal(2u, get_matching_indices(hwnd, LVNI_SELECTED).size());

				// ACT
				lv->select(table_model::npos(), true);

				// ASSERT
				assert_is_empty(model->tracking_requested);
			}


			test( IndependtListViewHandlesNativeNotifications )
			{
				// INIT
				window_tracker wt;
				shared_ptr<listview> lv(new win32::listview);
				HWND hlv = get_window_and_resize(lv, hparent);
				vector<table_model::index_type> selections;
				slot_connection c =
					lv->selection_changed += bind(&push_back<table_model::index_type>, ref(selections), _1);

				wt.checkpoint();

				RECT rc;
				column_def columns[] = {
					column_def(L"", 100),
					column_def(L"", 100),
					column_def(L"", 50),
				};
				mocks::columns_model_ptr cm(mocks::columns_model::create(columns, columns_model::npos(), false));
				mocks::model_ptr m(new mocks::listview_model(3, 3));

				::MoveWindow(hlv, 0, 0, 300, 200, TRUE);
				lv->set_columns_model(cm);
				lv->set_model(m);

				// ACT
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

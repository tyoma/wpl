#include <wpl/ui/combobox.h>

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/native_view.h>

#include "TestHelpers.h"

#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windowsx.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				struct list_model : wpl::ui::list_model
				{
					virtual index_type get_count() const throw()
					{	return items.size();	}

					virtual void get_text(index_type index, std::wstring &text) const
					{	text = items[index];	}

					vector<wstring> items;
				};
			}

			namespace
			{
				static wstring get_item_text(HWND hcombobox, unsigned index)
				{
					COMBOBOXINFO cbi = { sizeof(COMBOBOXINFO),  };

					::GetComboBoxInfo(hcombobox, &cbi);
					assert_not_null(cbi.hwndList);
					const size_t length = ::SendMessage(cbi.hwndList, LB_GETTEXTLEN, index, 0);
					vector<TCHAR> buffer(length + 1);
					::SendMessage(cbi.hwndList, LB_GETTEXT, index, reinterpret_cast<LPARAM>(&buffer[0]));
					return wstring(buffer.begin(), buffer.end() - 1);
				}

				static HWND get_window_and_resize(HWND hparent, visual &v, int cx, int cy)
				{
					visual::positioned_native_views nviews;
					v.resize(cx, cy, nviews);
					assert_is_true(1u <= nviews.size());
					return nviews[0].get_view().get_window(hparent);
				}
			}

			begin_test_suite( ComboboxTests )
				WindowManager wmanager;
				HWND parent;

				init( CreateParent )
				{
					parent = wmanager.create_visible_window();
					wmanager.enable_reflection(parent);
				}


				test( ComboboxControlIsANativeView )
				{
					// INIT
					shared_ptr<combobox> cb = create_combobox();
					window_tracker wt;

					// ACT
					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.find_created(WC_COMBOBOX).size());
					assert_equal(wt.find_created(WC_COMBOBOX)[0], hwnd);
					assert_equal(parent, ::GetParent(hwnd));

					// INIT
					wt.created.clear();

					// ACT / ASSERT (happens in get_window_and_resize)
					assert_equal(hwnd, get_window_and_resize(parent, *cb, 19, 7));

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
				}


				test( ComboboxControlHasRequestStyles )
				{
					// INIT
					const unsigned required_style = CBS_HASSTRINGS | CBS_DROPDOWNLIST;
					shared_ptr<combobox> cb = create_combobox();
					window_tracker wt;

					// ACT
					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);

					// ASSERT
					assert_equal(required_style, (required_style | CBS_SORT) & GetWindowStyle(hwnd));
				}


				test( CreatedComboboxIsPopulatedWithStringsFromThePreviouslySetModel )
				{
					// INIT
					wstring items1[] = { L"foo", L"bar" };
					wstring items2[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", };
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb1 = create_combobox();
					shared_ptr<combobox> cb2 = create_combobox();

					// ACT
					m->items = mkvector(items1);
					cb1->set_model(m);
					HWND hwnd = get_window_and_resize(parent, *cb1, 100, 20);

					// ASSERT
					assert_equal(2, ComboBox_GetCount(hwnd));
					assert_equal(items1[0], get_item_text(hwnd, 0));
					assert_equal(items1[1], get_item_text(hwnd, 1));

					// ACT
					m->items = mkvector(items2);
					cb2->set_model(m);
					hwnd = get_window_and_resize(parent, *cb2, 100, 20);

					// ASSERT
					assert_equal(3, ComboBox_GetCount(hwnd));
					assert_equal(items2[0], get_item_text(hwnd, 0));
					assert_equal(items2[1], get_item_text(hwnd, 1));
					assert_equal(items2[2], get_item_text(hwnd, 2));
				}


				test( ComboboxIsPopulatedWhenModelIsSet )
				{
					// INIT
					wstring items1[] = { L"foo", L"bar" };
					wstring items2[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", };
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();

					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);

					// ACT
					m->items = mkvector(items1);
					cb->set_model(m);

					// ASSERT
					assert_equal(2, ComboBox_GetCount(hwnd));
					assert_equal(items1[0], get_item_text(hwnd, 0));
					assert_equal(items1[1], get_item_text(hwnd, 1));

					// ACT
					m->items = mkvector(items2);
					cb->set_model(m);

					// ASSERT
					assert_equal(3, ComboBox_GetCount(hwnd));
					assert_equal(items2[0], get_item_text(hwnd, 0));
					assert_equal(items2[1], get_item_text(hwnd, 1));
					assert_equal(items2[2], get_item_text(hwnd, 2));
				}


				test( ModelInvalidationResetsItemsTexts )
				{
					// INIT
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();

					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);
					cb->set_model(m);
					m->items.resize(2);

					// ACT
					m->items[1] = L"one two";
					m->invalidated();

					// ASSERT
					assert_equal(2, ComboBox_GetCount(hwnd));
					assert_equal(L"", get_item_text(hwnd, 0));
					assert_equal(L"one two", get_item_text(hwnd, 1));

					// INIT
					m->items.resize(5);

					// ACT
					m->items[0] = L"three four five";
					m->items[3] = L"ein";
					m->invalidated();

					// ASSERT
					assert_equal(5, ComboBox_GetCount(hwnd));
					assert_equal(L"three four five", get_item_text(hwnd, 0));
					assert_equal(L"one two", get_item_text(hwnd, 1));
					assert_equal(L"", get_item_text(hwnd, 2));
					assert_equal(L"ein", get_item_text(hwnd, 3));
					assert_equal(L"", get_item_text(hwnd, 4));
				}


				test( ComboboxIsClearedOnModelReset )
				{
					// INIT
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();

					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);
					m->items.resize(2);
					cb->set_model(m);

					// ACT
					cb->set_model(shared_ptr<list_model>());

					// ASSERT
					assert_equal(0, ComboBox_GetCount(hwnd));

					// ACT
					m->invalidated();

					// ASSERT
					assert_equal(0, ComboBox_GetCount(hwnd));
				}


				test( SelectingAnItemChangesNativeSelection )
				{
					// INIT
					wstring items[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", L"one two" };
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();
					HWND hwnd =  get_window_and_resize(parent, *cb, 100, 20);

					m->items = mkvector(items);
					cb->set_model(m);

					// ACT
					cb->select(1);

					// ASSERT
					assert_equal(1, ComboBox_GetCurSel(hwnd));

					// ACT
					cb->select(3);

					// ASSERT
					assert_equal(3, ComboBox_GetCurSel(hwnd));

					// ACT
					cb->select(combobox::npos());

					// ASSERT
					assert_equal(CB_ERR, ComboBox_GetCurSel(hwnd));
				}


				test( SelectingAnItemSetsNativeSelectionOnMaterialize )
				{
					// INIT
					wstring items[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", L"one two" };
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();

					m->items = mkvector(items);
					cb->set_model(m);

					// ACT
					cb->select(3);
					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);

					// ASSERT
					assert_equal(3, ComboBox_GetCurSel(hwnd));

					// INIT
					wmanager.Cleanup();
					parent = wmanager.create_visible_window();

					// ACT
					cb->select(2);
					hwnd = get_window_and_resize(parent, *cb, 100, 20);

					// ASSERT
					assert_equal(2, ComboBox_GetCurSel(hwnd));
				}


				test( ChangingSelectionLeadsToNotification )
				{
					// INIT
					vector<combobox::index_type> selection_log;
					wstring items[] = { L"1", L"2", L"3", L"4", L"5", };
					shared_ptr<mocks::list_model> m(new mocks::list_model);
					shared_ptr<combobox> cb = create_combobox();
					slot_connection c = cb->selection_changed += [&] (combobox::index_type s) {
						selection_log.push_back(s);
					};

					HWND hwnd = get_window_and_resize(parent, *cb, 100, 20);

					m->items = mkvector(items);
					cb->set_model(m);

					// ASSERT
					assert_is_empty(selection_log);

					// ACT
					ComboBox_SetCurSel(hwnd, 3);
					::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

					// ASSERT
					combobox::index_type reference1[] = { 3u, };

					assert_equal(reference1, selection_log);

					// ACT
					ComboBox_SetCurSel(hwnd, 1);
					::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

					// ASSERT
					combobox::index_type reference2[] = { 3u, 1u, };

					assert_equal(reference2, selection_log);

					// ACT
					ComboBox_SetCurSel(hwnd, -1);
					::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

					// ASSERT
					combobox::index_type reference3[] = { 3u, 1u, combobox::npos() };

					assert_equal(reference3, selection_log);
				}

			end_test_suite
		}
	}
}

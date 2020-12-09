#include <wpl/win32/combobox.h>

#include "helpers.h"
#include "helpers-win32.h"

#include <list>
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windowsx.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			struct trackable : wpl::trackable
			{
				virtual index_type index() const override
				{	return index_;	}

				index_type index_;
			};

			struct list_model : wpl::list_model<wstring>
			{
				virtual index_type get_count() const throw() override
				{	return items.size();	}

				virtual void get_value(index_type index, wstring &text) const override
				{	text = items[index];	}

				virtual shared_ptr<const wpl::trackable> track(index_type row) const override
				{
					assert_is_true(0 <= row && row < get_count());

					list<trackable>::iterator i = trackables.insert(trackables.end(), trackable());

					i->index_ = row;
					return shared_ptr<wpl::trackable>(&*i, [i, this] (...) { trackables.erase(i); });
				}

				vector<wstring> items;
				mutable list<trackable> trackables;
			};
		}

		namespace
		{
			static wstring get_item_text(HWND hcombobox, unsigned index)
			{
				const size_t length = ComboBox_GetLBTextLen(hcombobox, index);
				vector<TCHAR> buffer(length + 1);
				ComboBox_GetLBText(hcombobox, index, &buffer[0]);
				return wstring(buffer.begin(), buffer.end() - 1);
			}
		}

		begin_test_suite( ComboboxTests )
			window_manager wmanager;
			HWND parent;

			init( CreateParent )
			{
				parent = wmanager.create_visible_window();
				wmanager.enable_reflection(parent);
			}


			test( ComboboxControlIsANativeView )
			{
				// INIT
				shared_ptr<combobox> cb(new win32::combobox);
				window_tracker wt;

				// ACT
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.find_created(WC_COMBOBOXEX).size());
				assert_equal(wt.find_created(WC_COMBOBOXEX)[0], hwnd);
				assert_equal(parent, ::GetParent(hwnd));

				// INIT
				wt.created.clear();

				// ACT / ASSERT (happens in get_window_and_resize)
				assert_equal(hwnd, get_window_and_resize(cb, parent, 19, 7));

				// ASSERT
				wt.checkpoint();

				assert_is_empty(wt.created);
			}


			test( ComboboxControlHasRequestStyles )
			{
				// INIT
				const unsigned required_style = CBS_DROPDOWNLIST;
				shared_ptr<combobox> cb(new win32::combobox);
				window_tracker wt;

				// ACT
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				assert_equal(required_style, (required_style | CBS_SORT) & GetWindowStyle(hwnd));
			}


			test( CreatedComboboxIsPopulatedWithStringsFromThePreviouslySetModel )
			{
				// INIT
				wstring items1[] = { L"foo", L"bar" };
				wstring items2[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", };
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb1(new win32::combobox);
				shared_ptr<combobox> cb2(new win32::combobox);

				// ACT
				m->items = mkvector(items1);
				cb1->set_model(m);
				HWND hwnd = get_window_and_resize(cb1, parent, 100, 20);

				// ASSERT
				assert_equal(2, ComboBox_GetCount(hwnd));
				assert_equal(items1[0], get_item_text(hwnd, 0));
				assert_equal(items1[1], get_item_text(hwnd, 1));

				// ACT
				m->items = mkvector(items2);
				cb2->set_model(m);
				hwnd = get_window_and_resize(cb2, parent, 100, 20);

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
				shared_ptr<combobox> cb(new win32::combobox);

				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

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
				shared_ptr<combobox> cb(new win32::combobox);

				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);
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
				shared_ptr<combobox> cb(new win32::combobox);

				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);
				m->items.resize(2);
				cb->set_model(m);

				// ACT
				cb->set_model(shared_ptr< list_model<wstring> >());

				// ASSERT
				assert_equal(0, ComboBox_GetCount(hwnd));

				// ACT
				m->invalidated();

				// ASSERT
				assert_equal(0, ComboBox_GetCount(hwnd));
			}


			test( SelectionIsImpossibleIfNoModelIsSet )
			{
				// INIT
				shared_ptr<combobox> cb(new win32::combobox);

				// ACT / ASSERT
				assert_throws(cb->select(0), logic_error);

				// INIT
				HWND hwnd =  get_window_and_resize(cb, parent, 100, 20);

				// ACT / ASSERT
				assert_throws(cb->select(0), logic_error);

				// ASSERT
				assert_equal(CB_ERR, ComboBox_GetCurSel(hwnd));
			}


			test( SelectingAnItemChangesNativeSelection )
			{
				// INIT
				wstring items[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", L"one two" };
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd =  get_window_and_resize(cb, parent, 100, 20);

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
				cb->select(combobox::model_t::npos());

				// ASSERT
				assert_equal(CB_ERR, ComboBox_GetCurSel(hwnd));
			}


			test( SelectingAnItemSetsNativeSelectionOnMaterialize )
			{
				// INIT
				wstring items[] = { L"Lorem ipsum", L"amet dolor", L"sit amet, consectetur adipiscing elit", L"one two" };
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);

				m->items = mkvector(items);
				cb->set_model(m);

				// ACT
				cb->select(3);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				assert_equal(3, ComboBox_GetCurSel(hwnd));

				// INIT
				wmanager.Cleanup();
				parent = wmanager.create_visible_window();

				// ACT
				cb->select(2);
				hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				assert_equal(2, ComboBox_GetCurSel(hwnd));
			}


			test( ChangingSelectionLeadsToNotification )
			{
				// INIT
				vector<combobox::model_t::index_type> selection_log;
				wstring items[] = { L"1", L"2", L"3", L"4", L"5", };
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				slot_connection c = cb->selection_changed += [&] (combobox::model_t::index_type s) {
					selection_log.push_back(s);
				};

				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items = mkvector(items);
				cb->set_model(m);

				// ASSERT
				assert_is_empty(selection_log);

				// ACT
				ComboBox_SetCurSel(hwnd, 3);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ASSERT
				combobox::model_t::index_type reference1[] = { 3u, };

				assert_equal(reference1, selection_log);

				// ACT
				ComboBox_SetCurSel(hwnd, 1);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ASSERT
				combobox::model_t::index_type reference2[] = { 3u, 1u, };

				assert_equal(reference2, selection_log);

				// ACT
				ComboBox_SetCurSel(hwnd, -1);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ASSERT
				combobox::model_t::index_type reference3[] = { 3u, 1u, combobox::model_t::npos() };

				assert_equal(reference3, selection_log);
			}


			test( RecreationOfComboboxPreservesSelectionIfWasChangedByUser )
			{
				// INIT
				vector<combobox::model_t::index_type> selection_log;
				wstring items[] = { L"1", L"2", L"3", L"4", L"5", };
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items = mkvector(items);
				cb->set_model(m);
				ComboBox_SetCurSel(hwnd, 3);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));
				wmanager.Cleanup();
				parent = wmanager.create_visible_window();
				wmanager.enable_reflection(parent);

				// ACT
				hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				assert_equal(3, ComboBox_GetCurSel(hwnd));

				//INIT
				ComboBox_SetCurSel(hwnd, 1);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));
				wmanager.Cleanup();
				parent = wmanager.create_visible_window();

				// ACT
				hwnd = get_window_and_resize(cb, parent, 100, 20);

				// ASSERT
				assert_equal(1, ComboBox_GetCurSel(hwnd));
			}


			test( TrackableIsAcquiredOnSelection )
			{
				// INIT
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);

				get_window_and_resize(cb, parent, 100, 20);
				m->items.resize(5);
				cb->set_model(m);

				// ACT
				cb->select(2);

				// ASSERT
				assert_equal(1u, m->trackables.size());
				assert_equal(2u, m->trackables.begin()->index_);

				// ACT
				cb->select(list_model<wstring>::npos());

				// ASSERT
				assert_is_empty(m->trackables);
			}


			test( TrackableIsAcquiredOnNativeSelection )
			{
				// INIT
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items.resize(5);

				// ACT
				cb->set_model(m);

				// ASSERT
				assert_is_empty(m->trackables);

				// ACT
				ComboBox_SetCurSel(hwnd, 3);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// INIT / ASSERT
				assert_equal(1u, m->trackables.size());
				assert_equal(3u, m->trackables.begin()->index_);
			}


			test( SelectionIsChangingOnTrackablesChange )
			{
				// INIT
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items.resize(5);
				cb->set_model(m);
				ComboBox_SetCurSel(hwnd, 4);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ACT
				m->trackables.begin()->index_ = 1;
				m->invalidated();

				// ASSERT
				assert_equal(1, ComboBox_GetCurSel(hwnd));

				// ACT
				m->trackables.begin()->index_ = 2;
				m->invalidated();

				// ASSERT
				assert_equal(2, ComboBox_GetCurSel(hwnd));
			}


			test( SelectionIsResetAndTrackableIsReleasedOnNposIndex )
			{
				// INIT
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items.resize(5);
				cb->set_model(m);
				ComboBox_SetCurSel(hwnd, 4);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ACT
				m->trackables.begin()->index_ = trackable::npos();
				m->invalidated();

				// ASSERT
				assert_equal(CB_ERR, ComboBox_GetCurSel(hwnd));
				assert_is_empty(m->trackables);
			}


			test( ChangingModelReleasesTrackable )
			{
				// INIT
				shared_ptr<mocks::list_model> m(new mocks::list_model);
				shared_ptr<mocks::list_model> m2(new mocks::list_model);
				shared_ptr<combobox> cb(new win32::combobox);
				HWND hwnd = get_window_and_resize(cb, parent, 100, 20);

				m->items.resize(5);
				m2->items.resize(6);
				cb->set_model(m);
				ComboBox_SetCurSel(hwnd, 4);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ACT
				cb->set_model(m2);

				// ASSERT
				assert_is_empty(m->trackables);
				assert_is_empty(m2->trackables);
				assert_equal(CB_ERR, ComboBox_GetCurSel(hwnd));

				// ACT
				ComboBox_SetCurSel(hwnd, 0);
				::SendMessage(parent, WM_COMMAND, CBN_SELCHANGE << 16, reinterpret_cast<LPARAM>(hwnd));

				// ASSERT
				assert_equal(1u, m2->trackables.size());
				assert_equal(0u, m2->trackables.begin()->index_);

				// ACT
				cb->set_model(shared_ptr< list_model<wstring> >());

				// ASSERT
				assert_is_empty(m2->trackables);
			}

		end_test_suite
	}
}

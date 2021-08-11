#include <wpl/controls.h>

#include <wpl/win32/controls.h>

#include "helpers-win32.h"

#include <functional>
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ut/assert.h>
#include <ut/test.h>

#pragma warning(disable: 4428)

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			void increment(int *value)
			{	++*value;	}

			template <typename T>
			void push_back(vector<T> &container, const T &value)
			{	container.push_back(value);	}

			template <typename T>
			void reset(shared_ptr<T> &ptr)
			{	ptr.reset();	}
		}

		begin_test_suite( ButtonTests )

			window_manager wmanager;
			HWND parent;

			init( CreateParent )
			{
				parent = wmanager.create_visible_window();
			}


			test( ButtonControlIsANativeView )
			{
				// INIT
				shared_ptr<button> b(new win32::button);
				window_tracker wt;

				// ACT / ASSERT
				assert_is_true(provides_tabstoppable_native_view(b));

				// ACT
				HWND hwnd = get_window_and_resize(b, parent, 100, 20);

				// ASSERT
				wt.checkpoint();

				assert_equal(wt.find_created(WC_BUTTON)[0], hwnd);
				assert_equal(parent, ::GetParent(hwnd));

				// ACT / ASSERT (happens in get_window_and_resize)
				assert_equal(hwnd, get_window_and_resize(b, parent, 19, 7));

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.created.size());
			}


			test( ClickingAButtonRaisesClickedSignal )
			{
				// INIT
				int clicks = 0;
				shared_ptr<button> b(new win32::button);
				HWND hbutton = get_window_and_resize(b, parent, 100, 100);
				slot_connection c = b->clicked += bind(&increment, &clicks);
										
				// ACT
				::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

				// ASSERT
				assert_equal(1, clicks);

				// ACT
				::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

				// ASSERT
				assert_equal(2, clicks);
			}


			test( SettingTextChangesButtonText )
			{
				// INIT
				shared_ptr<button> b(new win32::button);
				HWND hbutton = get_window_and_resize(b, parent, 100, 20);

				// ACT
				b->set_text(agge::style_modifier::empty + "Launch!");

				// ASSERT
				assert_equal(L"Launch!", get_window_text(hbutton));

				// ACT
				b->set_text(agge::style_modifier::empty +"Let's go...");

				// ASSERT
				assert_equal(L"Let's go...", get_window_text(hbutton));
			}


			test( TextSetIsPreservedBeforeTheMaterialization )
			{
				// INIT
				shared_ptr<button> b[] = { shared_ptr<button>(new win32::button), shared_ptr<button>(new win32::button), };

				// ACT
				b[0]->set_text(agge::style_modifier::empty +"Launch!");
				b[1]->set_text(agge::style_modifier::empty +"Stop...");
				HWND hbutton1 = get_window_and_resize(b[0], parent, 100, 20);
				HWND hbutton2 = get_window_and_resize(b[1], parent, 100, 20);

				// ASSERT
				assert_equal(L"Launch!", get_window_text(hbutton1));
				assert_equal(L"Stop...", get_window_text(hbutton2));
			}


			test( SelfDestructIsOK )
			{
				// INIT
				shared_ptr<button> b(new win32::button);
				HWND hbutton = get_window_and_resize(b, parent, 100, 20);
				slot_connection c = b->clicked += bind(&reset<button>, ref(b));

				// ACT
				::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

				// ASSERT
				assert_null(b);
			}
		end_test_suite


		begin_test_suite( LinkTests )

			window_manager wmanager;
			HWND parent;

			init( CreateParent )
			{
				parent = wmanager.create_visible_window();
			}


			test( LinkControlIsANativeView )
			{
				// INIT
				shared_ptr<link> b(new win32::link);
				window_tracker wt;

				// ACT / ASSERT
				assert_is_true(provides_tabstoppable_native_view(b));

				// ACT
				HWND hwnd = get_window_and_resize(b, parent, 100, 20);

				// ASSERT
				wt.checkpoint();

				assert_equal(wt.find_created(L"SysLink")[0], hwnd);
				assert_equal(parent, ::GetParent(hwnd));

				// ACT / ASSERT (happens in get_window_and_resize)
				assert_equal(hwnd, get_window_and_resize(b, parent, 19, 7));

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.created.size());
			}


			test( LinkControlAlignmentCanBeChangedBeforeTheWindowIsCreated )
			{
				// INIT
				shared_ptr<link> b(new win32::link);

				// ACT
				b->set_halign(agge::align_near);
				HWND hlink = get_window_and_resize(b, parent, 100, 20);

				// ASSERT
				assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

				// ACT
				b.reset(new win32::link);
				b->set_halign(agge::align_far);
				hlink = get_window_and_resize(b, parent, 100, 20);

				// ASSERT
				assert_equal(LWS_RIGHT, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
			}


			test( LinkControlAlignmentCanBeChanged )
			{
				// INIT
				shared_ptr<link> b(new win32::link);
				window_tracker wt;

				// ACT
				HWND hlink = get_window_and_resize(b, parent, 100, 20);

				// ASSERT
				assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
					
				// ACT
				b->set_halign(agge::align_center);

				// ASSERT
				assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

				// ACT
				b->set_halign(agge::align_far);

				// ASSERT
				assert_equal(LWS_RIGHT, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

				// ACT
				b->set_halign(agge::align_center);

				// ASSERT
				assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

				// ACT
				b->set_halign(agge::align_near);

				// ASSERT
				assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
			}


			test( ClickingALinkRaisesClickedSignal )
			{
				// INIT
				vector<size_t> log_ids;
				vector<string> log_links;
				shared_ptr<link> l(new win32::link);
				HWND hlink = get_window_and_resize(l, parent, 100, 20);
				slot_connection c1 = l->clicked += std::bind(&push_back<size_t>, ref(log_ids), _1);
				slot_connection c2 = l->clicked += std::bind(&push_back<string>, ref(log_links), _2);
				NMLINK nmlink = {};

				nmlink.hdr.code = NM_CLICK;
				nmlink.item.iLink = 0;
				wcscpy(nmlink.item.szUrl, L"Test #1");
										
				// ACT
				::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

				// ASSERT
				size_t reference_ids1[] = { 0u, };
				string reference_links1[] = { "Test #1", };

				assert_equal(reference_ids1, log_ids);
				assert_equal(reference_links1, log_links);

				// INIT
				nmlink.item.iLink = 3;
				wcscpy(nmlink.item.szUrl, L"https://github.com");

				// ACT
				::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

				// ASSERT
				size_t reference_ids2[] = { 0u, 3u, };
				string reference_links2[] = { "Test #1", "https://github.com", };

				assert_equal(reference_ids2, log_ids);
				assert_equal(reference_links2, log_links);
			}


			test( SettingTextChangesButtonText )
			{
				// INIT
				shared_ptr<link> l(new win32::link);
				HWND hlink = get_window_and_resize(l, parent, 100, 20);

				// ACT
				l->set_text(agge::style_modifier::empty +"Launch!");

				// ASSERT
				assert_equal(L"Launch!", get_window_text(hlink));

				// ACT
				l->set_text(agge::style_modifier::empty + "Let's <a href=\"zz\">go</a>...");

				// ASSERT
				assert_equal(L"Let's <a href=\"zz\">go</a>...", get_window_text(hlink));
			}


			test( TextSetIsPreservedBeforeTheMaterialization )
			{
				// INIT
				shared_ptr<link> l[] = { shared_ptr<link>(new win32::link), shared_ptr<link>(new win32::link), };

				// ACT
				l[0]->set_text(agge::style_modifier::empty + "Launch <a>the rocket</a>!");
				l[1]->set_text(agge::style_modifier::empty + "Stop...");
				HWND hlink1 = get_window_and_resize(l[0], parent, 100, 20);
				HWND hlink2 = get_window_and_resize(l[1], parent, 100, 20);

				// ASSERT
				assert_equal(L"Launch <a>the rocket</a>!", get_window_text(hlink1));
				assert_equal(L"Stop...", get_window_text(hlink2));
			}


			test( SelfDestructIsOK )
			{
				// INIT
				shared_ptr<link> l(new win32::link);
				HWND hlink = get_window_and_resize(l, parent, 100, 20);
				slot_connection c = l->clicked += bind(&reset<link>, ref(l));
				NMLINK nmlink = {};

				nmlink.hdr.code = NM_CLICK;
				nmlink.item.iLink = 0;
										
				// ACT
				::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

				// ASSERT
				assert_null(l);
			}
		end_test_suite


		begin_test_suite( AEditBoxTests )

			window_manager wmanager;
			HWND parent;

			init( CreateParent )
			{
				parent = wmanager.create_visible_window();
			}


			test( EditBoxControlIsANativeView )
			{
				// INIT
				shared_ptr<editbox> e(new win32::editbox);
				window_tracker wt;

				// ACT / ASSERT
				assert_is_true(provides_tabstoppable_native_view(e));

				// ACT
				auto hedit = get_window_and_resize(e, parent, 100, 20);

				// ASSERT
				wt.checkpoint();

				assert_equal(wt.find_created(WC_EDITW)[0], hedit);
				assert_equal(parent, ::GetParent(hedit));

				// ACT / ASSERT (happens in get_window_and_resize)
				assert_equal(hedit, get_window_and_resize(e, parent, 19, 7));

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.created.size());
			}


			test( ChangeNotificationIsDeliveredAsASignal )
			{
				// INIT
				auto e = make_shared<win32::editbox>();
				auto hedit = get_window_and_resize(e, parent, 100, 20);
				auto changed = 0;

				auto conn = e->changed += [&] {	changed++;	};

				// ACT
				::SendMessageW(hedit, OCM_COMMAND, EN_CHANGE << 16, 0);

				// ASSERT
				assert_equal(1, changed);

				// ACT
				::SendMessageW(hedit, OCM_COMMAND, EN_CHANGE << 16, 0);
				::SendMessageW(hedit, OCM_COMMAND, EN_CHANGE << 16, 0);

				// ASSERT
				assert_equal(3, changed);

				// ACT
				::SendMessageW(hedit, OCM_COMMAND, EN_UPDATE << 16, 0);
				::SendMessageW(hedit, OCM_COMMAND, EN_ERRSPACE << 16, 0);

				// ASSERT
				assert_equal(3, changed);
			}


			test( SettingTextChangesEditText )
			{
				// INIT
				shared_ptr<editbox> e(new win32::editbox);
				HWND hbutton = get_window_and_resize(e, parent, 100, 20);

				// ACT
				e->set_value("Launch!");

				// ASSERT
				assert_equal(L"Launch!", get_window_text(hbutton));

				// ACT
				e->set_value("\xd0\x9d\xd0\xb5 \xd0\xb2\xd1\x8b\xd1\x85\xd0\xbe\xd0\xb4\xd0\xb8 \xd0\xb8\xd0\xb7 \xd0\xba\xd0\xbe\xd0\xbc\xd0\xbd\xd0\xb0\xd1\x82\xd1\x8b");

				// ASSERT
				assert_equal(L"\u041d\u0435 \u0432\u044b\u0445\u043e\u0434\u0438 \u0438\u0437 \u043a\u043e\u043c\u043d\u0430\u0442\u044b", get_window_text(hbutton));
			}


			test( TextSetIsPreservedBeforeTheMaterialization )
			{
				// INIT
				shared_ptr<editbox> e[] = { shared_ptr<editbox>(new win32::editbox), shared_ptr<editbox>(new win32::editbox), };

				// ACT
				e[0]->set_value("Launch!");
				e[1]->set_value("Stop...");
				HWND hedit1 = get_window_and_resize(e[0], parent, 100, 20);
				HWND hedit2 = get_window_and_resize(e[1], parent, 100, 20);

				// ASSERT
				assert_equal(L"Launch!", get_window_text(hedit1));
				assert_equal(L"Stop...", get_window_text(hedit2));
			}


			test( GettingValueReturnsWindowText )
			{
				// INIT
				auto e = make_shared<win32::editbox>();
				string value;

				e->set_value("...this will be ignored");
				auto hedit = get_window_and_resize(e, parent, 100, 20);

				::SetWindowTextW(hedit, L"And this will be returned...");

				// ACT / ASSERT
				assert_is_true(e->get_value(value));

				// ASSERT
				assert_equal("And this will be returned...", value);

				// INIT
				::SetWindowTextW(hedit, L"\u0432\u044b\u0445\u043e\u0434\u0438");

				// ACT / ASSERT
				assert_is_true(e->get_value(value));

				// ASSERT
				assert_equal("\xd0\xb2\xd1\x8b\xd1\x85\xd0\xbe\xd0\xb4\xd0\xb8", value);
			}


			test( AcceptIsReportedOnEnter )
			{
				// INIT
				auto e = make_shared<win32::editbox>();
				auto hedit = get_window_and_resize(e, parent, 100, 20);
				vector<string> log;
				auto conn = e->accept += [&] (string &v) {	log.push_back(v);	};

				// INIT / ACT
				::SetWindowTextW(hedit, L"one");

				// ASSERT
				assert_is_empty(log);

				// ACT
				::SendMessageW(hedit, WM_KEYDOWN, VK_RETURN, 0);

				// ASSERT
				string reference1[] = {	"one",	};

				assert_equal(reference1, log);

				// INIT
				::SetWindowTextW(hedit, L"a long string");

				// ACT
				::SendMessageW(hedit, WM_KEYDOWN, VK_RETURN, 0);
				::SendMessageW(hedit, WM_KEYDOWN, VK_LEFT, 0);

				// ASSERT
				string reference2[] = {	"one", "a long string",	};

				assert_equal(reference2, log);
				assert_equal(L"a long string", get_window_text(hedit));
			}


			test( WindowTextIsSetToTheModifiedValue )
			{
				// INIT
				auto e = make_shared<win32::editbox>();
				auto hedit = get_window_and_resize(e, parent, 100, 20);
				auto conn = e->accept += [&] (string &v) {	v = "something else";	};

				::SetWindowTextW(hedit, L"one");

				// ACT
				::SendMessageW(hedit, WM_KEYDOWN, VK_RETURN, 0);

				// ASSERT
				assert_equal(L"something else", get_window_text(hedit));

				// INIT
				conn = e->accept += [&] (string &v) {	v = "not quite correct";	};
				::SetWindowTextW(hedit, L"two");

				// ACT
				::SendMessageW(hedit, WM_KEYDOWN, VK_RETURN, 0);

				// ASSERT
				assert_equal(L"not quite correct", get_window_text(hedit));
			}


			test( CharactersAreTranslatedViaSignal )
			{
				// INIT
				auto e = make_shared<win32::editbox>();
				auto hedit = get_window_and_resize(e, parent, 100, 20);
				auto conn = e->translate_char += [] (wchar_t &c) {	c = towupper(c);	};

				// ACT
				::SendMessageW(hedit, WM_CHAR, L's', 0);
				::SendMessageW(hedit, WM_CHAR, L'o', 0);
				::SendMessageW(hedit, WM_CHAR, L'm', 0);
				::SendMessageW(hedit, WM_CHAR, L'e', 0);

				// ASSERT
				assert_equal(L"SOME", get_window_text(hedit));

				// INIT
				conn = e->translate_char += [] (wchar_t &c) {	c = towlower(c);	};

				// ACT
				::SendMessageW(hedit, WM_CHAR, L't', 0);
				::SendMessageW(hedit, WM_CHAR, L'h', 0);
				::SendMessageW(hedit, WM_CHAR, L'I', 0);
				::SendMessageW(hedit, WM_CHAR, L'N', 0);
				::SendMessageW(hedit, WM_CHAR, L'G', 0);

				// ASSERT
				assert_equal(L"SOMEthing", get_window_text(hedit));
			}
		end_test_suite
	}
}

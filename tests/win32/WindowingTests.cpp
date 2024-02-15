#include <wpl/win32/window.h>

#include "helpers-win32.h"

#include <functional>
#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <vector>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			struct handler
			{
				vector<MSG> messages;
				LRESULT myresult;

				handler()
					: myresult(0)
				{	}

				LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam, const win32::window::original_handler_t &previous)
				{
					MSG m = { };

					m.message = message;
					m.wParam = wparam;
					m.lParam = lparam;

					messages.push_back(m);
					if (message == WM_SETTEXT && _tcscmp((LPCTSTR)lparam, _T("disallowed")) == 0)
						return 0;
					return !myresult ? previous(message, wparam, lparam) : myresult;
				}
			};

			WNDPROC original = 0;

			LRESULT CALLBACK replacement_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
			{	return ::CallWindowProc(original, hwnd, message, wparam, lparam);	}

			LRESULT passthrough(UINT message, WPARAM wparam, LPARAM lparam, const win32::window::original_handler_t &previous)
			{	return previous(message, lparam, wparam);	}

			LRESULT reset_on_ncdestroy(shared_ptr<win32::window> &w, UINT message, WPARAM wparam, LPARAM lparam,
				const win32::window::original_handler_t &previous)
			{
				if (WM_NCDESTROY == message)
					w.reset();
				return previous(message, wparam, lparam);
			}

			bool msg_eq(MSG lhs, MSG rhs)
			{
				return lhs.hwnd == rhs.hwnd && lhs.message == rhs.message && lhs.wParam == rhs.wParam
					&& lhs.lParam == rhs.lParam;
			}
		}

		begin_test_suite( WindowingTests )

			window_manager windowManager;

			teardown( Cleanup )
			{
				windowManager.Cleanup();
			}


			test( FailToWrapNonWindow )
			{
				// INIT / ACT / ASSERT
				assert_throws(win32::window::attach((HWND)0x12345678, &passthrough), invalid_argument);
			}


			test( WrapWindowAndReturnWrapper )
			{
				// INIT
				HWND hwnd = windowManager.create_window();

				// ACT (must not throw)
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));

				// ASSERT
				assert_not_null(w);
			}


			test( WrappersForDifferentWindowsAreDifferent )
			{
				// INIT
				HWND hwnd1 = windowManager.create_window(), hwnd2 = windowManager.create_window();

				// ACT
				shared_ptr<win32::window> w1(win32::window::attach(hwnd1, &passthrough));
				shared_ptr<win32::window> w2(win32::window::attach(hwnd2, &passthrough));

				// ASSERT
				assert_not_equal(w1, w2);
			}


			test( WrapperForTheSameWindowIsTheSame )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));

				// ACT / ASSERT
				assert_throws(win32::window::attach(hwnd, &passthrough), logic_error);
			}


			test( SameWindowCanBeReattachedIfPreviousWrapperDestroyed )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));
				weak_ptr<win32::window> ww(w);

				// ACT
				w.reset();

				// ACT / ASSERT (does not throw)
				w = win32::window::attach(hwnd, &passthrough);

				// ASSERT
				assert_not_null(w);
				assert_is_true(ww.expired());
			}


			static LRESULT dummy(shared_ptr<void>)
			{	return 0;	}

			test( UserHandlerIsHeldIfWindowObjectIsAlive )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<bool> p(new bool);
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&dummy, p)));

				// ACT
				::DestroyWindow(hwnd);

				// ACT / ASSERT
				assert_is_false(p.unique());
			}


			test( UserHandlerIsReleasedOnWindowObjectRelease )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<bool> p(new bool);
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&dummy, p)));

				// ACT
				w.reset();

				// ACT / ASSERT
				assert_is_true(p.unique());
			}


			test( MessagesAreDelegatedToOriginalWndProcIfNotIntercepted )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				TCHAR buffer[256] = { 0 };

				win32::window::attach(hwnd, &passthrough);

				// ACT
				::SetWindowText(hwnd, _T("First message..."));
				::GetWindowText(hwnd, buffer, 255);

				// ASSERT
				assert_equal(0, _tcscmp(buffer, _T("First message...")));

				// ACT
				::SetWindowText(hwnd, _T("Message #2"));
				::GetWindowText(hwnd, buffer, 255);

				// ASSERT
				assert_equal(0, _tcscmp(buffer, _T("Message #2")));
			}


			test( MessagesAreDelegatedToUserCBIfIntercepted )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				handler h;
				LPCTSTR s1 = _T("Text #1");
				LPCTSTR s2 = _T("Text #2");
				TCHAR buffer[5] = { 0 };

				// ACT
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));
				::SetWindowText(hwnd, s1);
				::SetWindowText(hwnd, s2);
				::GetWindowText(hwnd, buffer, 3);
				::GetWindowText(hwnd, buffer, 4);

				// ASSERT
				assert_equal(4u, h.messages.size());
				assert_equal(WM_SETTEXT, (int)h.messages[0].message);
				assert_equal(reinterpret_cast<LPCTSTR>(h.messages[0].lParam), s1);
				assert_equal(WM_SETTEXT, (int)h.messages[1].message);
				assert_equal(reinterpret_cast<LPCTSTR>(h.messages[1].lParam), s2);
				assert_equal(WM_GETTEXT, (int)h.messages[2].message);
				assert_equal(3u, h.messages[2].wParam);
				assert_equal(WM_GETTEXT, (int)h.messages[3].message);
				assert_equal(4u, h.messages[3].wParam);
			}


			test( ResultIsProvidedFromInterceptor )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				handler h;
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

				// ACT
				h.myresult = 123;
				int r1 = ::GetWindowTextLength(hwnd);
				h.myresult = 321;
				int r2 = ::GetWindowTextLength(hwnd);

				// ASSERT
				assert_equal(123, r1);
				assert_equal(321, r2);
			}


			test( MessageIsNotPassedToPreviousHandlerIfUserHandlerDoesNotCallIt )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				handler h;
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

				// ACT
				::SetWindowText(hwnd, _T("allowed"));

				// ASSERT
				assert_equal(7, ::GetWindowTextLength(hwnd));

				// ACT
				::SetWindowText(hwnd, _T("disallowed"));

				// ASSERT
				assert_equal(7, ::GetWindowTextLength(hwnd));
			}


			test( InterceptionStopsIfConnectionDestroyed )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				handler h;
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

				::SetWindowText(hwnd, _T("allowed"));
				::SetWindowText(hwnd, _T("disallowed"));

				// preASSERT
				assert_equal(7, ::GetWindowTextLength(hwnd));

				// INIT
				size_t calls = h.messages.size();

				// ACT
				w.reset();
				::SetWindowText(hwnd, _T("disallowed"));

				// ASSERT
				assert_equal(10, ::GetWindowTextLength(hwnd));
				assert_equal(h.messages.size(), calls);
			}


			test( DetachRestoresOriginalWindowProc )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));
				WNDPROC replacement_wndproc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);

				// ACT
				w.reset();

				// ASSERT
				assert_not_equal(replacement_wndproc, (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC));
			}


			test( DetachReleasesWrapperFromWindow )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));
				weak_ptr<win32::window> w_weak(w);

				// ACT
				w.reset();

				// ASSERT
				assert_is_true(w_weak.expired());
			}


			test( WrapperIsNotDestroyedIfSubclassedAfterAttachment )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<win32::window> w(win32::window::attach(hwnd, &passthrough));
				win32::window *p = w.get();
				TCHAR buffer[100] = { 0 };

				original = (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&replacement_proc);

				// ACT
				w.reset();
				(*p)(WM_SETTEXT, 0, reinterpret_cast<LPARAM>(_T("one")));
				(*p)(WM_GETTEXT, sizeof(buffer), reinterpret_cast<LPARAM>(buffer));

				// ASSERT
				assert_equal(0, _tcscmp(buffer, _T("one")));

				// ACT
				(*p)(WM_SETTEXT, 0, reinterpret_cast<LPARAM>(_T("two")));
				(*p)(WM_GETTEXT, sizeof(buffer), reinterpret_cast<LPARAM>(buffer));

				// ASSERT
				assert_equal(0, _tcscmp(buffer, _T("two")));
			}


			test( UserHandlerIsReleasedOnWindowObjectReleaseEvenIfOversubclassed )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				shared_ptr<bool> p(new bool);
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&dummy, p)));

				original = (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&replacement_proc);

				// ACT
				w.reset();

				// ACT / ASSERT
				assert_is_true(p.unique());
			}


			test( NcDestroyIsReceivedOnDestroyWindow )
			{
				// INIT
				HWND hwnd = windowManager.create_window();
				handler h;
				shared_ptr<win32::window> w(win32::window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

				// ACT
				::DestroyWindow(hwnd);

				// ASSERT
				assert_is_false(h.messages.empty());
				assert_equal(WM_NCDESTROY, static_cast<int>(h.messages.back().message));
			}


			test( WindowDetachOnDestroyReleasesWindowObject )
			{
				// INIT
				HWND hwnd[] = { windowManager.create_window(), windowManager.create_window(), };
				shared_ptr<win32::window> w[2];

				w[0] = win32::window::attach(hwnd[0], bind(&reset_on_ncdestroy, ref(w[0]), _1, _2, _3, _4));
				w[1] = win32::window::attach(hwnd[1], bind(&reset_on_ncdestroy, ref(w[1]), _1, _2, _3, _4));

				weak_ptr<win32::window> ww[2] = { w[0], w[1], };

				// ACT / ASSERT (must not crash)
				::DestroyWindow(hwnd[0]);

				// ASSERT
				assert_is_true(ww[0].expired());

				// ACT / ASSERT (must not crash)
				::DestroyWindow(hwnd[1]);

				// ASSERT
				assert_is_true(ww[1].expired());
			}


			test( FocusAndKeyboardMessagesAreForwardedToParentWindow )
			{
				// INIT
				HWND hparent = windowManager.create_window();
				HWND hwnd[] = {
					windowManager.create_window(L"edit", hparent, WS_CHILD | WS_VISIBLE, 0),
					windowManager.create_window(L"button", hparent, WS_CHILD, 0),
				};
				vector<MSG> messages;
				auto w = win32::window::attach(hparent, [&] (UINT m, WPARAM w, LPARAM l, const win32::window& prev) {
					if (win32::window::WM_FORWARDED == m)
						messages.push_back(*reinterpret_cast<const MSG *>(w));
					return prev(m, w, l);
				});
				auto w1 = win32::window::attach(hwnd[0], passthrough);
				auto w2 = win32::window::attach(hwnd[1], passthrough);

				// ACT
				::SendMessage(hwnd[0], WM_KEYDOWN, VK_UP, 10);
				::SendMessage(hwnd[1], WM_KEYUP, VK_RETURN, 3120);
				::SendMessage(hwnd[1], WM_SETFOCUS, (WPARAM)hwnd[0], 0);

				// ASSERT
				MSG reference1[] = {
					{	hwnd[0], WM_KEYDOWN, VK_UP, 10,	},
					{	hwnd[1], WM_KEYUP, VK_RETURN, 3120,	},
					{	hwnd[1], WM_SETFOCUS, (WPARAM)hwnd[0], 0,	},
				};

				assert_equal_pred(reference1, messages, msg_eq);

				// ACT
				::SendMessage(hwnd[0], WM_SETFOCUS, (WPARAM)hwnd[1], 0);

				// ASSERT
				MSG reference2[] = {
					{	hwnd[0], WM_KEYDOWN, VK_UP, 10,	},
					{	hwnd[1], WM_KEYUP, VK_RETURN, 3120,	},
					{	hwnd[1], WM_SETFOCUS, (WPARAM)hwnd[0], 0,	},
					{	hwnd[0], WM_SETFOCUS, (WPARAM)hwnd[1], 0,	},
				};

				assert_equal_pred(reference2, messages, msg_eq);

				// ACT
				::SendMessage(hwnd[0], WM_LBUTTONDOWN, 0, 0);
				::SendMessage(hwnd[0], WM_RBUTTONUP, 0, 0);

				// ASSERT
				assert_equal_pred(reference2, messages, msg_eq);
			}


			test( ForwardedMessagesCanBeInhibitedUponRequest )
			{
				// INIT
				HWND hparent = windowManager.create_window();
				HWND hwnd[] = {
					windowManager.create_window(L"edit", hparent, WS_CHILD | WS_VISIBLE, 0),
					windowManager.create_window(L"button", hparent, WS_CHILD, 0),
				};
				bool inhibit = false;
				vector<HWND> messages;
				auto w = win32::window::attach(hparent, [&] (UINT m, WPARAM w, LPARAM l, const win32::window& prev) {
					if (win32::window::WM_FORWARDED == m)
						*reinterpret_cast<bool*>(l) = inhibit;
					return prev(m, w, l);
				});
				auto w1 = win32::window::attach(hwnd[0], [&] (UINT m, WPARAM w, LPARAM l, const win32::window& prev) {
					messages.push_back(hwnd[0]);
					return prev(m, w, l);
				});
				auto w2 = win32::window::attach(hwnd[1], [&] (UINT m, WPARAM w, LPARAM l, const win32::window& prev) {
					messages.push_back(hwnd[1]);
					return prev(m, w, l);
				});

				// ACT
				::SendMessage(hwnd[0], WM_KEYDOWN, VK_UP, 10);
				inhibit = true;
				::SendMessage(hwnd[1], WM_KEYUP, VK_RETURN, 3120);
				::SendMessage(hwnd[1], WM_SETFOCUS, (WPARAM)hwnd[0], 0);
				inhibit = false;

				// ASSERT
				HWND reference1[] = {	hwnd[0], hwnd[1] /*WM_SETFOCUS cannot be inhibited*/,	};

				assert_equal(reference1, messages);

				// ACT
				inhibit = true;
				::SendMessage(hwnd[0], WM_KEYDOWN, VK_UP, 10);
				inhibit = false;
				::SendMessage(hwnd[1], WM_KEYUP, VK_RETURN, 3120);
				::SendMessage(hwnd[0], WM_SETFOCUS, (WPARAM)hwnd[0], 0);

				// ASSERT
				HWND reference2[] = {	hwnd[0], hwnd[1], hwnd[1], hwnd[0],	};

				assert_equal(reference2, messages);
			}
		end_test_suite
	}
}

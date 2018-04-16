#include <wpl/ui/win32/window.h>

#include "TestHelpers.h"

#include <functional>
#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <vector>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace ui
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

					LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
					{
						MSG m = { 0 };

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

				LRESULT passthrough(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous)
				{	return previous(message, lparam, wparam);	}
			}

			begin_test_suite( WindowingTests )

				WindowManager windowManager;

				init( Init )
				{
					windowManager.Init();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}


				test( FailToWrapNonWindow )
				{
					// INIT / ACT / ASSERT
					assert_throws(window::attach((HWND)0x12345678, &passthrough), invalid_argument);
				}


				test( WrapWindowAndReturnWrapper )
				{
					// INIT
					HWND hwnd = windowManager.create_window();

					// ACT (must not throw)
					shared_ptr<window> w(window::attach(hwnd, &passthrough));

					// ASSERT
					assert_not_null(w);
				}


				test( WrappersForDifferentWindowsAreDifferent )
				{
					// INIT
					HWND hwnd1 = windowManager.create_window(), hwnd2 = windowManager.create_window();

					// ACT
					shared_ptr<window> w1(window::attach(hwnd1, &passthrough));
					shared_ptr<window> w2(window::attach(hwnd2, &passthrough));

					// ASSERT
					assert_not_equal(w1, w2);
				}


				test( WrapperHoldsHWND )
				{
					// INIT
					HWND hwnd1 = windowManager.create_window(), hwnd2 = windowManager.create_window();
					shared_ptr<window> w1(window::attach(hwnd1, &passthrough));
					shared_ptr<window> w2(window::attach(hwnd2, &passthrough));

					// ACT / ASSERT
					assert_equal(hwnd1, w1->hwnd());
					assert_equal(hwnd2, w2->hwnd());
				}


				test( WrapperForTheSameWindowIsTheSame )
				{
					// INIT
					HWND hwnd = windowManager.create_window();
					shared_ptr<window> w(window::attach(hwnd, &passthrough));

					// ACT / ASSERT
					assert_throws(window::attach(hwnd, &passthrough), logic_error);
				}


				test( SameWindowCanBeReattachedIfPreviousWrapperDestroyed )
				{
					// INIT
					HWND hwnd = windowManager.create_window();
					shared_ptr<window> w(window::attach(hwnd, &passthrough));
					weak_ptr<window> ww(w);

					// ACT
					w.reset();

					// ACT / ASSERT (does not throw)
					w = window::attach(hwnd, &passthrough);

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
					shared_ptr<window> w(window::attach(hwnd, bind(&dummy, p)));

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
					shared_ptr<window> w(window::attach(hwnd, bind(&dummy, p)));

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

					window::attach(hwnd, &passthrough);

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
					shared_ptr<window> w(window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));
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
					shared_ptr<window> w(window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

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
					shared_ptr<window> w(window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

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
					shared_ptr<window> w(window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

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
					shared_ptr<window> w(window::attach(hwnd, &passthrough));
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
					shared_ptr<window> w(window::attach(hwnd, &passthrough));
					weak_ptr<window> w_weak(w);

					// ACT
					w.reset();

					// ASSERT
					assert_is_true(w_weak.expired());
				}


				test( WrapperIsNotDestroyedIfSubclassedAfterAttachment )
				{
					// INIT
					HWND hwnd = windowManager.create_window();
					shared_ptr<window> w(window::attach(hwnd, &passthrough));
					window *p = w.get();
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
					shared_ptr<window> w(window::attach(hwnd, bind(&dummy, p)));

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
					shared_ptr<window> w(window::attach(hwnd, bind(&handler::on_message, &h, _1, _2, _3, _4)));

					// ACT
					::DestroyWindow(hwnd);

					// ASSERT
					assert_is_false(h.messages.empty());
					assert_equal(WM_NCDESTROY, static_cast<int>(h.messages.back().message));
				}


				test( WindowHandleIsNulledOnDestroy )
				{
					// INIT
					HWND hwnd = windowManager.create_window();
					shared_ptr<window> w(window::attach(hwnd, &passthrough));

					// ACT
					::DestroyWindow(hwnd);

					// ASSERT
					assert_null(w->hwnd());
				}
			end_test_suite
		}
	}
}

#include <agge.text/text_engine.h>

#include <wpl/win32/native_view.h>

#include "helpers-win32.h"

#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/stylesheet_db.h>
#include <wpl/win32/font_manager.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class windowed_view : public native_view
			{
			public:
				windowed_view(const string &font_style = "text")
					: native_view(font_style)
				{	}

			public:
				vector< pair<HWND /*parent*/, HWND /*created*/> > log;

			private:
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const win32::window::original_handler_t &original)
				{	return original(message, wparam, lparam);	}

				virtual HWND materialize(HWND hparent)
				{
					HWND hwnd = ::CreateWindow(L"static", 0, WS_CHILD, 0, 0, 1, 1, hparent, 0, 0, 0);

					log.push_back(make_pair(hparent, hwnd));
					return hwnd;
				}
			};
		}

		begin_test_suite( NativeViewHelperTests )
			window_manager wmanager;
			HWND parent;
			window_tracker tracker;
			shared_ptr<gcontext::text_engine_type> text_engine;

			init( CreateParent )
			{
				parent = wmanager.create_visible_window();
				tracker.checkpoint();
				tracker.created.clear();
				text_engine = create_text_engine();
			}


			test( NativeViewHelperDoesNotCreateAnythingOnConstruction )
			{
				// INIT / ACT
				mocks::windowed_view wv;

				// ASSERT
				tracker.checkpoint();

				assert_is_empty(tracker.created);
			}


			test( NativeViewHelperRequestsWindowCreationOnGetWindow )
			{
				// INIT
				mocks::windowed_view wv;
				native_view &nv = wv;

				// ACT
				HWND hwnd = nv.get_window(parent);

				// ASSERT
				pair<HWND, HWND> reference[] = { make_pair(parent, hwnd) };

				assert_equal(reference, wv.log);
			}


			test( WindowIsMaterializedOnlyOnceForTheSameParent )
			{
				// INIT
				mocks::windowed_view wv;
				native_view &nv = wv;

				HWND hwnd1 = nv.get_window(parent);

				// ACT
				HWND hwnd2 = nv.get_window(parent);

				// ASSERT
				assert_equal(1u, wv.log.size());
				assert_equal(hwnd1, hwnd2);
			}


			test( WindowIsRecreatedForANewParent )
			{
				// INIT
				mocks::windowed_view wv;
				native_view &nv = wv;
				HWND parent2 = wmanager.create_visible_window();

				HWND hwnd1 = nv.get_window(parent);

				tracker.checkpoint();

				// ACT
				HWND hwnd2 = nv.get_window(parent2);

				// ASSERT
				HWND destroyed[] = { hwnd1, };

				tracker.checkpoint();
				assert_equal(destroyed, tracker.destroyed);
				assert_not_equal(hwnd1, hwnd2);
			}


			test( OwnedWindowIsDestroyedOnViewDestruction )
			{
				// INIT
				shared_ptr<mocks::windowed_view> wv(new mocks::windowed_view);
				native_view &nv = *wv;

				HWND hwnd = nv.get_window(parent);

				tracker.checkpoint();

				// ACT
				wv.reset();

				// ASSERT
				HWND destroyed[] = { hwnd, };

				tracker.checkpoint();
				assert_equal(destroyed, tracker.destroyed);
			}


			test( StyleFontIsSetOnMaterialize )
			{
				// INIT
				stylesheet_db ss;
				win32::font_manager fm;
				shared_ptr<mocks::windowed_view> wv[] = {
					shared_ptr<mocks::windowed_view>(new mocks::windowed_view("text.static")),
					shared_ptr<mocks::windowed_view>(new mocks::windowed_view("text.edit")),
					shared_ptr<mocks::windowed_view>(new mocks::windowed_view("text")),
				};

				ss.set_font("text.static", text_engine->create_font(L"Arial", 10, false, false, agge::font::key::gf_none));
				ss.set_font("text.edit", text_engine->create_font(L"Tahoma", -12, false, false, agge::font::key::gf_none));
				ss.set_font("text", text_engine->create_font(L"Tahoma", 13, true, false, agge::font::key::gf_none));

				// ACT
				wv[0]->apply_styles(ss, fm);
				wv[1]->apply_styles(ss, fm);
				wv[2]->apply_styles(ss, fm);

				static_cast<native_view &>(*wv[0]).get_window(parent);
				static_cast<native_view &>(*wv[1]).get_window(parent);
				static_cast<native_view &>(*wv[2]).get_window(parent);

				// ASSERT
				assert_equal(fm.get_font(agge::font::key(L"Arial", 10)).get(),
					(void *)SendMessage(wv[0]->get_window(), WM_GETFONT, 0, 0));
				assert_equal(fm.get_font(agge::font::key(L"Tahoma", -12)).get(),
					(void *)SendMessage(wv[1]->get_window(), WM_GETFONT, 0, 0));
				assert_equal(fm.get_font(agge::font::key(L"Tahoma", 13, true)).get(),
					(void *)SendMessage(wv[2]->get_window(), WM_GETFONT, 0, 0));
			}


			test( FontIsUpdatedForAnExistedWindow )
			{
				// INIT
				stylesheet_db ss;
				win32::font_manager fm;
				mocks::windowed_view wv("text.static");

				ss.set_font("text.static", text_engine->create_font(L"Arial", 10, false, false, agge::font::key::gf_none));
				wv.apply_styles(ss, fm);
				static_cast<native_view &>(wv).get_window(parent);

				// ACT
				ss.set_font("text.static", text_engine->create_font(L"Tahoma", 18, false, false, agge::font::key::gf_none));

				// ASSERT
				assert_equal(fm.get_font(agge::font::key(L"Arial", 10)).get(),
					(void *)SendMessage(wv.get_window(), WM_GETFONT, 0, 0));

				// ACT
				wv.apply_styles(ss, fm);

				// ASSERT
				assert_equal(fm.get_font(agge::font::key(L"Tahoma", 18)).get(),
					(void *)SendMessage(wv.get_window(), WM_GETFONT, 0, 0));
			}

		end_test_suite
	}
}

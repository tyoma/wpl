#include <crtdbg.h>

#include "../application.h"
#include "../stylesheet.h"

#include <tchar.h>
#include <wpl/factory.h>
#include <wpl/freetype2/font_loader.h>
#include <wpl/win32/cursor_manager.h>
#include <wpl/win32/helpers.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace
	{
		LARGE_INTEGER g_pq_frequency;
		const long long c_pq_frequency = (::QueryPerformanceFrequency(&g_pq_frequency), g_pq_frequency.QuadPart);

		timestamp clock_win32()
		{
			LARGE_INTEGER v;
			::QueryPerformanceCounter(&v);
			return 1000 * v.QuadPart / c_pq_frequency;
		}


		class queue_win32 : noncopyable
		{
		public:
			queue_win32()
				: _hwnd(::CreateWindow(_T("static"), 0, WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0))
			{	::SetWindowLongPtr(_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&window_proc));	}

			bool schedule(const queue_task &task, timespan defer_by)
			{
				unique_ptr<queue_task> t(new queue_task(task));

				return (defer_by
					? ::SetTimer(_hwnd, reinterpret_cast<UINT_PTR>(t.get()), static_cast<UINT>(defer_by), NULL)
					: ::PostMessageA(_hwnd, WM_USER, reinterpret_cast<WPARAM>(t.get()), 0)
				) ? t.release(), true : false;
			}

		private:
			static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM /*lparam*/)
			{
				switch (message)
				{
				case WM_TIMER:
					::KillTimer(hwnd, wparam);

				case WM_USER:
					const unique_ptr<queue_task> task(reinterpret_cast<queue_task *>(wparam));

					(*task)();
				}
				return 0;
			}

		private:
			win32::helpers::window_handle _hwnd;
		};
	}

	class application::impl
	{
	};

	application::application()
	{
		_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		const auto q = make_shared<queue_win32>();

		_queue = [q] (const queue_task &task, timespan defer_by) {	return q->schedule(task, defer_by);	};
	}

	application::~application()
	{	}

	shared_ptr<factory> application::create_default_factory() const
	{
		const auto text_engine = create_text_engine();
		const auto stylesheet_ = create_sample_stylesheet(text_engine);
		factory_context context = {
			shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
			shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(2)),
			text_engine,
			stylesheet_,
			make_shared<win32::cursor_manager>(),
			clock_win32,
			_queue,
		};

		return factory::create_default(context);
	}

	queue application::get_application_queue() const
	{	return _queue;	}

	void application::run()
	{
		MSG msg;

		while (::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	void application::exit()
	{	::PostQuitMessage(0);	}
}

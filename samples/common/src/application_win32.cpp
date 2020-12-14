#include <crtdbg.h>

#include "../application.h"

#include <windows.h>

namespace wpl
{
	class application::impl
	{
	};

	application::application()
	{	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	}

	application::~application()
	{	}

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

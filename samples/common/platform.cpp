#include "platform.h"

#include <wpl/win32/font_loader.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace
	{
		struct text_engine_composite
		{
			text_engine_composite()
				: text_engine(loader)
			{	}

			win32::font_loader loader;
			gcontext::text_engine_type text_engine;
		};
	}

	std::shared_ptr<gcontext::text_engine_type> create_text_engine()
	{
		auto tec = make_shared<text_engine_composite>();

		return shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine);
	}

	void run_message_loop()
	{
		MSG msg;

		while (::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	void exit_message_loop()
	{
		::PostQuitMessage(0);
	}
}

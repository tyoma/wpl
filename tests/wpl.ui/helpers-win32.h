#pragma once

#include <wpl/concepts.h>
#include <wpl/input.h>
#include <wpl/types.h>

#include <iterator>
#include <set>
#include <string>

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace wpl
{
	class container;
	struct widget;

	namespace tests
	{
#ifdef UNICODE
		typedef std::basic_string<wchar_t> tstring;
#else
		typedef std::basic_string<char> tstring;
#endif
		class window_tracker;

		RECT get_window_rect(HWND hwnd);
		RECT rect(int left, int top, int width, int height);
		std::wstring get_window_text(HWND hwnd);
		bool has_style(HWND hwnd, int style);
		bool has_no_style(HWND hwnd, int style);
		unsigned int pack_coordinates(int x, int y);
		void emulate_click(HWND hwnd, int x, int y, mouse_input::mouse_buttons button,
			int /*mouse_buttons | modifier_keys*/ depressed);

		class WindowManager
		{
			std::vector<HWND> _windows;
			std::vector< std::shared_ptr<void> > _connections;

		public:
			HWND create_window();
			HWND create_visible_window();
			HWND create_window(const std::wstring &class_name);
			HWND create_window(const std::wstring &class_name, HWND parent, unsigned int style, unsigned int stylex);
			void enable_reflection(HWND hwnd);

			void destroy_window(HWND hwnd);

		public:
			~WindowManager();

			void Cleanup();
		};

		class window_tracker
		{
			std::set<HWND> _windows;
			std::wstring _allow, _prohibit;

		public:
			window_tracker(const std::wstring &allow = L"", const std::wstring &prohibit = L"MSCTFIME UI");

			void checkpoint();
			std::vector<HWND> find_created(const std::wstring &class_name);

			std::vector<HWND> created;
			std::vector<HWND> destroyed;
		};



		inline unsigned int pack_coordinates(int x, int y)
		{	return (unsigned short)x | ((unsigned int )(unsigned short)y << 16);	}
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);

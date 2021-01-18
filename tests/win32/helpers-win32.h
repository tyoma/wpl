#pragma once

#include <wpl/concepts.h>
#include <wpl/input.h>
#include <wpl/types.h>
#include <wpl/visual.h>

#include <iterator>
#include <set>
#include <string>
#include <vector>

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace wpl
{
	class container;
	struct control;
	struct view_host;
	struct widget;

	namespace tests
	{
		namespace mocks
		{
			class cursor_manager;
		}

#ifdef UNICODE
		typedef std::basic_string<wchar_t> tstring;
#else
		typedef std::basic_string<char> tstring;
#endif
		class window_tracker;

		HWND get_window_and_resize(std::shared_ptr<control> control_, HWND hparent, int cx = 150, int cy = 100);
		bool provides_tabstoppable_native_view(std::shared_ptr<control> control_);
		RECT get_window_rect(HWND hwnd);
		agge::box<int> get_client_size(HWND hwnd);
		agge::rect<int> get_update_rect(HWND hwnd);
		RECT rect(int left, int top, int width, int height);
		std::wstring get_window_text(HWND hwnd);
		bool has_style(HWND hwnd, int style);
		bool has_no_style(HWND hwnd, int style);
		unsigned int pack_coordinates(int x, int y);
		void emulate_click(HWND hwnd, int x, int y, mouse_input::mouse_buttons button,
			int /*mouse_buttons | modifier_keys*/ depressed);
		std::shared_ptr<gcontext::text_engine_type> create_text_engine();

		class window_manager
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
			~window_manager();

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

		unsigned int pack_screen_coordinates(HWND hwnd, int x, int y);

		unsigned int pack_wheel(int delta, int modifiers = 0);
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);
bool operator ==(const agge::rect_i &lhs, const RECT &rhs);

#pragma once

#include <wpl/base/concepts.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace wpl
{
	namespace ui
	{
		class container;
		struct widget;

		namespace tests
		{
			class window_tracker;

			RECT get_window_rect(HWND hwnd);
			RECT rect(int left, int top, int width, int height);
			std::wstring get_window_text(HWND hwnd);
			unsigned int pack_coordinates(short x, short y);


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

				void Init();
				void Cleanup();
			};

			class window_tracker
			{
				std::set<HWND> _windows;
				std::wstring _allow, _prohibit;

			public:
				window_tracker(const std::wstring &allow = L"", const std::wstring &prohibit = L"MSCTFIME UI");

				void checkpoint();

				std::vector<HWND> created;
				std::vector<HWND> destroyed;
			};

			template <typename T, size_t n>
			inline T *begin(T (&container)[n])
			{	return container;	}

			template <typename Container>
			inline typename Container::iterator begin(Container &container)
			{	return container.begin();	}

			template <typename T, size_t n>
			inline T *end(T (&container)[n])
			{	return container + n;	}

			template <typename Container>
			inline typename Container::iterator end(Container &container)
			{	return container.end();	}


			inline unsigned int pack_coordinates(short x, short y)
			{	return (unsigned short)x | ((unsigned int )(unsigned short)y << 16);	}
		}
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);

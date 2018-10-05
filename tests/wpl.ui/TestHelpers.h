#pragma once

#include <agge/types.h>
#include <wpl/base/concepts.h>
#include <wpl/ui/types.h>

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
			inline T *begin(T (&container_)[n])
			{	return container_;	}

			template <typename Container>
			inline typename Container::iterator begin(Container &container_)
			{	return container_.begin();	}

			template <typename T, size_t n>
			inline T *end(T (&container_)[n])
			{	return container_ + n;	}

			template <typename Container>
			inline typename Container::iterator end(Container &container_)
			{	return container_.end();	}


			inline unsigned int pack_coordinates(short x, short y)
			{	return (unsigned short)x | ((unsigned int )(unsigned short)y << 16);	}

			template <typename T>
			inline agge::rect<T> make_rect(T x1, T y1, T x2, T y2)
			{
				agge::rect<T> r = { x1, y1, x2, y2 };
				return r;
			}

			inline view_location make_position(int x, int y, int width, int height)
			{
				view_location p = { x, y, width, height };
				return p;
			}
		}

		inline bool operator ==(const view_location &lhs, const view_location &rhs)
		{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;	}
	}
}

template <typename T>
inline bool operator ==(const agge::rect<T> &lhs, const agge::rect<T> &rhs)
{	return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;	}

bool operator ==(const RECT &lhs, const RECT &rhs);

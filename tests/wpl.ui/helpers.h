#pragma once

#include <wpl/concepts.h>
#include <wpl/input.h>
#include <wpl/types.h>
#include <wpl/visual.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace agge
{
	struct color;
}

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

		template <typename T, size_t n>
		inline std::vector<T> mkvector(T (&data)[n])
		{	return std::vector<T>(data, data + n);	}


		inline unsigned int pack_coordinates(int x, int y)
		{	return (unsigned short)x | ((unsigned int )(unsigned short)y << 16);	}

		template <typename T>
		inline agge::agge_vector<T> make_vector(T dx, T dy)
		{
			agge::agge_vector<T> v = { dx, dy };
			return v;
		}

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

		gcontext::pixel_type make_pixel(const agge::color& color);
	}

	inline bool operator ==(const view_location &lhs, const view_location &rhs)
	{	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;	}
}

namespace agge
{
	bool operator ==(const wpl::gcontext::pixel_type &lhs, const wpl::gcontext::pixel_type &rhs);

	template <typename T>
	inline bool operator ==(const rect<T> &lhs, const rect<T> &rhs)
	{	return lhs.x1 == rhs.x1 && lhs.y1 == rhs.y1 && lhs.x2 == rhs.x2 && lhs.y2 == rhs.y2;	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);

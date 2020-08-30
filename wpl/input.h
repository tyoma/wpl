#pragma once

#include "signals.h"

namespace wpl
{
	struct keyboard_input
	{
		typedef std::pair< int /*tab index*/, std::shared_ptr<keyboard_input> > tabbed_control;

		enum special_keys {
			left, up, right, down, enter, tab, home, end, page_down, page_up,
		};

		enum modifier_keys {
			base = 1,
			shift = base << 0,
			control = base << 1,
			command = base << 2,
			alt = base << 3,
			next = base << 4,
		};

		virtual ~keyboard_input() {	}

		virtual void get_tabbed_controls(std::vector<tabbed_control> &tabbed_controls, bool do_clear = true);

		virtual void key_down(unsigned code, int modifiers);
		virtual void character(wchar_t symbol, unsigned repeats, int modifiers);
		virtual void key_up(unsigned code, int modifiers);

		virtual void got_focus();
		virtual void lost_focus();

		signal<void (const std::shared_ptr<keyboard_input> &view_)> request_focus;
	};

	struct mouse_input
	{
		enum mouse_buttons {
			base = keyboard_input::next,
			left = base << 0,
			middle = base << 1,
			right = base << 2,
		};

		virtual ~mouse_input() {	}

		virtual void mouse_enter();
		virtual void mouse_leave();

		virtual void mouse_move(int depressed, int x, int y);

		virtual void mouse_down(mouse_buttons button_, int depressed, int x, int y);
		virtual void mouse_up(mouse_buttons button_, int depressed, int x, int y);
		virtual void mouse_double_click(mouse_buttons button_, int depressed, int x, int y);

		virtual void lost_capture();

		signal<void(std::shared_ptr<void> &handle)> capture;
	};



	inline void keyboard_input::get_tabbed_controls(std::vector<tabbed_control> &/*tabbed_controls*/, bool /*do_clear*/)
	{	}

	inline void keyboard_input::key_down(unsigned /*code*/, int /*modifiers*/)
	{	}

	inline void keyboard_input::character(wchar_t /*symbol*/, unsigned /*repeats*/, int /*modifiers*/)
	{	}

	inline void keyboard_input::key_up(unsigned /*code*/, int /*modifiers*/)
	{	}

	inline void keyboard_input::got_focus()
	{	}

	inline void keyboard_input::lost_focus()
	{	}


	inline void mouse_input::mouse_enter()
	{	}

	inline void mouse_input::mouse_leave()
	{	}

	inline void mouse_input::mouse_move(int /*buttons*/, int /*x*/, int /*y*/)
	{	}

	inline void mouse_input::mouse_down(mouse_buttons /*button*/, int /*buttons*/, int /*x*/, int /*y*/)
	{	}

	inline void mouse_input::mouse_up(mouse_buttons /*button*/, int /*buttons*/, int /*x*/, int /*y*/)
	{	}

	inline void mouse_input::mouse_double_click(mouse_buttons /*button*/, int /*buttons*/, int /*x*/, int /*y*/)
	{	}

	inline void mouse_input::lost_capture()
	{	}
}

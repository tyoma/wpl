#pragma once

#include "../base/signals.h"

namespace wpl
{
	struct keyboard_input
	{
		enum modifier_keys {
			shift = 1 << 0,
			control = 1 << 1,
			command = 1 << 2,
			alt = 3 << 2,
		};

		virtual ~keyboard_input() {	}

		virtual void got_focus();
		virtual void key_down(unsigned code, int modifiers);
		virtual void character(wchar_t symbol, unsigned repeats, int modifiers);
		virtual void key_up(unsigned code, int modifiers);
		virtual void lost_focus();
	};

	struct mouse_input
	{
		enum mouse_buttons {
			left = 1 << 0,
			middle = 1 << 1,
			right = 1 << 2,
		};

		virtual ~mouse_input() {	}

		virtual void mouse_enter();
		virtual void mouse_leave();

		virtual void mouse_move(int depressed, int x, int y);

		virtual void mouse_down(mouse_buttons button, int depressed, int x, int y);
		virtual void mouse_up(mouse_buttons button, int depressed, int x, int y);
		virtual void mouse_double_click(mouse_buttons button, int depressed, int x, int y);

		virtual void lost_capture();

		signal<void(std::shared_ptr<void> &handle)> capture;
	};



	void inline keyboard_input::got_focus()
	{	}

	void inline keyboard_input::key_down(unsigned /*code*/, int /*modifiers*/)
	{	}

	void inline keyboard_input::character(wchar_t /*symbol*/, unsigned /*repeats*/, int /*modifiers*/)
	{	}

	void inline keyboard_input::key_up(unsigned /*code*/, int /*modifiers*/)
	{	}

	void inline keyboard_input::lost_focus()
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
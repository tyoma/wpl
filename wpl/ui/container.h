#pragma once

#include "view.h"

namespace wpl
{
	namespace ui
	{
		struct layout_manager;

		class container : public view
		{
		public:
			struct positioned_view
			{
				view_location location;
				std::shared_ptr<view> child;
				int tab_order;
				slot_connection invalidate_connection;
				slot_connection force_layout_connection;
				slot_connection capture_connection;
			};

		public:
			container();

			void add_view(const std::shared_ptr<view> &child, int tab_order = 0);
			void set_layout(const std::shared_ptr<layout_manager> &layout);

			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

			virtual void get_tabbed_controls(std::vector<tabbed_control> &tabbed_controls, bool do_clear = true);

			virtual void mouse_leave();
			virtual void mouse_move(int depressed, int x, int y);
			virtual void mouse_down(mouse_buttons button, int depressed, int x, int y);
			virtual void mouse_up(mouse_buttons button, int depressed, int x, int y);
			virtual void mouse_double_click(mouse_buttons button, int depressed, int x, int y);

		private:
			typedef std::vector<positioned_view> views_t;

		private:
			void on_invalidate(unsigned index, const agge::rect_i *area);
			void on_force_layout();
			void on_capture(unsigned index, std::shared_ptr<void> &capture_handle);
			std::shared_ptr<view> child_from_point(int &x, int &y) const;
			std::shared_ptr<view> switch_mouse_over(int &x, int &y);

		private:
			views_t _children;
			std::shared_ptr<mouse_input> _mouse_over;
			unsigned _capture_target;
			std::shared_ptr<layout_manager> _layout;
		};
	}
}

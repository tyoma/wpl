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
				int left, top, width, height;
				std::shared_ptr<view> child;
				slot_connection invalidate_connection;
			};

		public:
			void add_view(const std::shared_ptr<view> &child);
			void set_layout(const std::shared_ptr<layout_manager> &layout);

			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy);

		private:
			typedef std::vector<positioned_view> views_t;

		private:
			void do_layout();
			void on_invalidate(unsigned index, const agge::rect_i *area);

		private:
			views_t _children;
			std::shared_ptr<layout_manager> _layout;
			int _cx, _cy;
		};
	}
}

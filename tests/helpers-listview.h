#pragma once

#include <tests/common/predicates.h>

#include <wpl/controls/listview_core.h>

namespace wpl
{
	namespace tests
	{
		class tracking_listview : public controls::listview_core
		{
		public:
			enum drawing_event_type { item_background = 1, subitem_background = 2, item_self = 4, subitem_self = 8, };

		public:
			struct drawing_event
			{
				template <typename T>
				drawing_event(drawing_event_type type_, gcontext &context_,
						const gcontext::rasterizer_ptr &rasterizer_, const agge::rect<T> &box_, index_type item_,
						unsigned state_, columns_model::index_type subitem_ = 0u, std::wstring text_ = L"")
					: type(type_), context(&context_), rasterizer(rasterizer_.get()), item(item_),
						subitem(subitem_), state(state_), text(text_)
				{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2; }

				drawing_event_type type;
				gcontext *context;
				gcontext::rasterizer_type *rasterizer;
				agge::rect<double> box;
				table_model::index_type item;
				columns_model::index_type subitem;
				unsigned state;
				std::wstring text;
			};

			using controls::listview_core::item_state_flags;

		public:
			tracking_listview()
				: item_height(0), reported_events(item_background | subitem_background | item_self | subitem_self)
			{	}

		public:
			mutable std::vector<drawing_event> events;
			double item_height;
			unsigned reported_events;

		private:
			virtual agge::real_t get_minimal_item_height() const override
			{	return (agge::real_t)item_height;	}

			virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned state) const override
			{
				if (item_background & reported_events)
					events.push_back(drawing_event(item_background, ctx, rasterizer, box, item, state));
			}

			virtual void draw_subitem_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned state, columns_model::index_type subitem) const override
			{
				if (subitem_background & reported_events)
					events.push_back(drawing_event(subitem_background, ctx, rasterizer, box, item, state, subitem));
			}

			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
				const agge::rect_r &box, index_type item, unsigned state) const override
			{
				if (item_self & reported_events)
					events.push_back(drawing_event(item_self, ctx, rasterizer, box, item, state));
			}

			virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
				index_type item, unsigned state, columns_model::index_type subitem, const std::wstring &text) const override
			{
				if (subitem_self & reported_events)
					events.push_back(drawing_event(subitem_self, ctx, rasterizer, box, item, state, subitem, text));
			}
		};


		struct listview_event_eq
		{
			bool operator ()(const tracking_listview::drawing_event &lhs,
				const tracking_listview::drawing_event &rhs) const
			{
				return lhs.type == rhs.type && lhs.context == rhs.context && lhs.rasterizer == rhs.rasterizer
					&& eq()(lhs.box, rhs.box) && lhs.item == rhs.item && lhs.state == rhs.state
					&& lhs.subitem == rhs.subitem && lhs.text == rhs.text;
			}
		};
	}
}

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
						unsigned state_, headers_model::index_type subitem_ = 0u)
					: type(type_), context(&context_), rasterizer(rasterizer_.get()), item(item_),
						subitem(subitem_), state(state_)
				{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2; }

				drawing_event_type type;
				gcontext *context;
				gcontext::rasterizer_type *rasterizer;
				agge::rect<double> box;
				table_model_base::index_type item;
				headers_model::index_type subitem;
				unsigned state;
			};

			using controls::listview_core::item_state_flags;

		public:
			tracking_listview()
				: item_height(0), reported_events(item_background | subitem_background | item_self | subitem_self)
			{	got_focus();	}

			virtual void set_columns_model(std::shared_ptr<headers_model> cmodel) override
			{	controls::listview_core::set_columns_model(cmodel);	}

			virtual void set_model(std::shared_ptr<richtext_table_model> model) override
			{	controls::listview_core::set_model(model);	}

		public:
			mutable std::vector<drawing_event> events;
			double item_height;
			unsigned reported_events;

		private:
			virtual agge::real_t get_minimal_item_height() const override
			{	return (agge::real_t)item_height;	}

			virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
				unsigned layer, index_type row, unsigned state) const override
			{
				if (0u == layer && (item_background & reported_events))
					events.push_back(drawing_event(item_background, ctx, rasterizer, box, row, state));
				if (1u == layer && (item_self & reported_events))
					events.push_back(drawing_event(item_self, ctx, rasterizer, box, row, state));
			}

			virtual void draw_subitem(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
				unsigned layer, index_type row, unsigned state, headers_model::index_type column) const override
			{
				if (0u == layer && (subitem_background & reported_events))
					events.push_back(drawing_event(subitem_background, ctx, rasterizer, box, row, state, column));
				if (1u == layer && (subitem_self & reported_events))
					events.push_back(drawing_event(subitem_self, ctx, rasterizer, box, row, state, column));
			}
		};


		struct listview_event_eq
		{
			bool operator ()(const tracking_listview::drawing_event &lhs,
				const tracking_listview::drawing_event &rhs) const
			{
				return lhs.type == rhs.type && lhs.context == rhs.context && lhs.rasterizer == rhs.rasterizer
					&& eq()(lhs.box, rhs.box) && lhs.item == rhs.item && lhs.state == rhs.state
					&& lhs.subitem == rhs.subitem;
			}
		};
	}
}

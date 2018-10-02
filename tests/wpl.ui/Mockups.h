#pragma once

#include <agge/filling_rules.h>
#include <agge/figures.h>
#include <agge/path.h>
#include <wpl/ui/view.h>
#include <wpl/ui/layout.h>
#include <ut/assert.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				struct mouse_event
				{
					enum event_type { enter, leave, down, up, move, double_click };

					event_type type;
					mouse_input::mouse_buttons button;
					int already_depressed;
					int x, y;
				};


				class blender
				{
				public:
					typedef agge::uint8_t cover_type;

				public:
					blender();

					void operator ()(const gcontext::pixel_type *buffer, int x, int y, agge::count_t count,
						const cover_type *covers) const;

				public:
					mutable int min_x, min_y, max_x, max_y;

				private:
					mutable bool _empty;
				};


				template <typename BaseT>
				class logging_visual : public BaseT
				{
				public:
					std::vector< std::pair<int /*cx*/, int /*cy*/> > resize_log;
					mutable std::vector< std::pair<int /*cx*/, int /*cy*/> > surface_size_log;
					mutable std::vector<agge::rect_i> update_area_log;
					mutable std::vector<gcontext::rasterizer_type *> rasterizers_log;

				private:
					virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
					virtual void resize(unsigned cx, unsigned cy);
				};


				template <typename BaseT>
				class logging_mouse_input : public BaseT
				{
				public:
					logging_mouse_input();

				public:
					std::vector<mouse_event> events_log;
					unsigned capture_lost;

				private:
					virtual void mouse_enter();
					virtual void mouse_leave();
					virtual void mouse_move(int buttons, int x, int y);
					virtual void mouse_down(mouse_input::mouse_buttons button, int buttons, int x, int y);
					virtual void mouse_up(mouse_input::mouse_buttons button, int buttons, int x, int y);
					virtual void mouse_double_click(mouse_input::mouse_buttons button, int buttons, int x, int y);
					virtual void lost_capture();
				};



				class logging_layout_manager : public wpl::ui::layout_manager
				{
				public:
					struct position { int left, top, width, height; };

				public:
					mutable std::vector< std::pair<size_t, size_t> > reposition_log;
					mutable std::vector<container::positioned_view> last_widgets;
					std::vector<position> positions;

				private:
					virtual void layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const;
				};


				class fill_layout : public wpl::ui::layout_manager
				{
				private:
					virtual void layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const;
				};



				inline blender::blender()
					: _empty(true)
				{	}

				inline void blender::operator ()(const gcontext::pixel_type * /*buffer*/, int x, int y, agge::count_t count,
					const cover_type * /*covers*/) const
				{
					if (_empty)
					{
						min_x = x, max_x = x + count, max_y = min_y = y;
						_empty = false;
					}
					else
					{
						min_x = (std::min)(min_x, x);
						min_y = (std::min)(min_y, y);
						max_x = (std::max)(max_x, x + (int)count);
						max_y = (std::max)(max_y, y);
					}
				}


				template <typename BaseT>
				inline void logging_visual<BaseT>::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
				{
					blender b;

					assert_equal(0, rasterizer->width());
					agge::add_path(*rasterizer, agge::rectangle(-1000.0f, -1000.0f, 1000.0f, 1000.0f));
					ctx(rasterizer, b, agge::winding<>());
					surface_size_log.push_back(std::make_pair(b.max_x - b.min_x, b.max_y - b.min_y + 1));
					update_area_log.push_back(ctx.update_area());
					rasterizers_log.push_back(rasterizer.get());
				}

				template <typename BaseT>
				inline void logging_visual<BaseT>::resize(unsigned cx, unsigned cy)
				{	resize_log.push_back(std::make_pair(cx, cy));	}


				inline mouse_event me_down(mouse_input::mouse_buttons button, int already_depressed, int x, int y)
				{
					mouse_event e = { mouse_event::down, button, already_depressed, x, y };
					return e;
				}

				inline mouse_event me_up(mouse_input::mouse_buttons button, int already_depressed, int x, int y)
				{
					mouse_event e = { mouse_event::up, button, already_depressed, x, y };
					return e;
				}

				inline mouse_event me_double_click(mouse_input::mouse_buttons button, int already_depressed, int x, int y)
				{
					mouse_event e = { mouse_event::up, button, already_depressed, x, y };
					return e;
				}

				inline mouse_event me_move(int already_depressed, int x, int y)
				{
					mouse_event e = { mouse_event::move, mouse_input::left, already_depressed, x, y };
					return e;
				}

				inline mouse_event me_enter()
				{
					mouse_event e = { mouse_event::enter, mouse_input::left, 0, 0, 0 };
					return e;
				}

				inline mouse_event me_leave()
				{
					mouse_event e = { mouse_event::leave, mouse_input::left, 0, 0, 0 };
					return e;
				}

				inline bool operator ==(const mouse_event &lhs, const mouse_event &rhs)
				{
					return lhs.type == rhs.type && lhs.button == rhs.button && lhs.already_depressed == rhs.already_depressed
						&& lhs.x == rhs.x && lhs.y == rhs.y;
				}


				template <typename BaseT>
				inline logging_mouse_input<BaseT>::logging_mouse_input()
					: capture_lost(0)
				{	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_enter()
				{	events_log.push_back(me_enter());	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_leave()
				{	events_log.push_back(me_leave());	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_move(int buttons, int x, int y)
				{	events_log.push_back(me_move(buttons, x, y));	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_down(mouse_input::mouse_buttons button, int buttons, int x, int y)
				{	events_log.push_back(me_down(button, buttons, x, y));	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_up(mouse_input::mouse_buttons button, int buttons, int x, int y)
				{	events_log.push_back(me_up(button, buttons, x, y));	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::mouse_double_click(mouse_input::mouse_buttons button, int buttons, int x, int y)
				{	events_log.push_back(me_up(button, buttons, x, y));	}

				template <typename BaseT>
				inline void logging_mouse_input<BaseT>::lost_capture()
				{	++capture_lost;	}
			}
		}
	}
}

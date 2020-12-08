#pragma once

#include "helpers.h"
#include "helpers-visual.h"

#include <agge/color.h>
#include <agge/filling_rules.h>
#include <agge/figures.h>
#include <agge/path.h>
#include <map>
#include <wpl/cursor.h>
#include <wpl/view.h>
#include <wpl/layout.h>
#include <ut/assert.h>

#ifdef far
	#undef far
#endif

#ifdef near
	#undef near
#endif

#include <agge.text/text_engine.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			struct mouse_event
			{
				enum event_type { enter, leave, down, up, move, double_click, scroll };

				event_type type;
				mouse_input::mouse_buttons button;
				int already_depressed;
				int x, y;
				int delta_x, delta_y;
			};


			struct keyboard_event
			{
				enum event_type { focusin, focusout, keydown, keyup, key_char, };

				event_type type;
				int /*keyboard_input::special_keys*/ key_code;
				int /*keyboard_input::modifier_keys*/ modifiers;
			};


			class capture_provider
			{
			public:
				void add_view(view &v)
				{
					_views[&v] = v.capture += [this, &v] (std::shared_ptr<void> &ch) {
						std::vector< std::pair<view *, bool> > &log_ = this->log;
						std::map< view *, std::weak_ptr<void> > &log2_ = this->log2;
						view &v2 = v;

						log_.push_back(std::make_pair(&v, true));
						ch.reset(new bool, [&log_, &log2_, &v2] (bool *p) {
							delete p;
							log_.push_back(std::make_pair(&v2, false));
							log2_.erase(&v2);
						});
						log2_[&v] = ch;
					};
				}

			public:
				std::vector< std::pair<view *, bool> > log;
				std::map<view *, std::weak_ptr<void> > log2;

			private:
				std::map<view *, slot_connection> _views;
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
				mutable bool has_uniform_color;
				mutable gcontext::pixel_type uniform_color;

			private:
				mutable bool _empty, _unicolor_set;
			};


			struct font_loader : gcontext::text_engine_type::loader
			{
				virtual agge::font::accessor_ptr load(const wchar_t *, int, bool, bool, agge::font::key::grid_fit)
				{	return agge::font::accessor_ptr();	}
			};


			class cursor_manager : public wpl::cursor_manager
			{
			public:
				cursor_manager();

				virtual std::shared_ptr<const cursor> get(standard_cursor id) const;
				virtual void set(std::shared_ptr<const cursor> cursor_);
				virtual void push(std::shared_ptr<const cursor> cursor_);
				virtual void pop();

			public:
				std::map< standard_cursor, std::shared_ptr<const cursor> > cursors;
				std::shared_ptr<const cursor> recently_set;
				unsigned attempts, stack_level;
			};


			template <typename BaseT>
			class logging_visual : public BaseT
			{
			public:
				mutable std::vector< std::pair<int /*cx*/, int /*cy*/> > surface_size_log;
				mutable std::vector<agge::rect_i> update_area_log;
				mutable std::vector<gcontext::text_engine_type *> text_engines_log;
				mutable std::vector<gcontext::rasterizer_type *> rasterizers_log;
				mutable std::vector< std::pair<gcontext::pixel_type, bool> > background_color;

				std::vector< std::pair<int /*cx*/, int /*cy*/> > resize_log;

			private:
				virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			};


			template <typename BaseT>
			class filling_visual : public BaseT
			{
			public:
				filling_visual(agge::color color);

			private:
				virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;

			private:
				agge::color _color;
				unsigned _cx, _cy;
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
				virtual void mouse_move(int depressed, int x, int y);
				virtual void mouse_down(mouse_input::mouse_buttons button, int depressed, int x, int y);
				virtual void mouse_up(mouse_input::mouse_buttons button, int depressed, int x, int y);
				virtual void mouse_double_click(mouse_input::mouse_buttons button, int depressed, int x, int y);
				virtual void mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y);
				virtual void lost_capture();
			};


			template <typename BaseT>
			class logging_key_input : public BaseT
			{
			public:
				logging_key_input();

			public:
				std::vector<keyboard_event> events;

			private:
				virtual void key_down(unsigned code, int modifiers);
				virtual void character(wchar_t symbol, unsigned repeats, int modifiers);
				virtual void key_up(unsigned code, int modifiers);
				virtual void got_focus();
				virtual void lost_focus();
			};



			template <typename BaseT>
			inline void logging_visual<BaseT>::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
			{
				blender b;

				assert_equal(0, rasterizer->width());
				agge::add_path(*rasterizer, agge::rectangle(-1000.0f, -1000.0f, 1000.0f, 1000.0f));
				ctx(rasterizer, b, agge::winding<>());
				surface_size_log.push_back(std::make_pair(b.max_x - b.min_x, b.max_y - b.min_y + 1));
				update_area_log.push_back(ctx.update_area());
				text_engines_log.push_back(&ctx.text_engine);
				rasterizers_log.push_back(rasterizer.get());
				background_color.push_back(std::make_pair(b.uniform_color, b.has_uniform_color));
			}


			template <typename BaseT>
			inline filling_visual<BaseT>::filling_visual(agge::color color)
				: _color(color)
			{	}

			template <typename BaseT>
			void filling_visual<BaseT>::draw(gcontext &ctx, gcontext::rasterizer_ptr &/*rasterizer*/) const
			{	rectangle(ctx, _color, 0, 0, _cx, _cy);	}


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
				mouse_event e = { mouse_event::double_click, button, already_depressed, x, y };
				return e;
			}

			inline mouse_event me_scroll(int already_depressed, int x, int y, int delta_x, int delta_y)
			{
				mouse_event e = { mouse_event::scroll, mouse_input::left, already_depressed, x, y, delta_x, delta_y };
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
					&& lhs.x == rhs.x && lhs.y == rhs.y
					&& (lhs.type != mouse_event::scroll || lhs.delta_x == rhs.delta_x && lhs.delta_y == rhs.delta_y);
			}

			inline bool operator ==(const keyboard_event &lhs, const keyboard_event &rhs)
			{	return lhs.type == rhs.type && lhs.key_code == rhs.key_code && lhs.modifiers == rhs.modifiers;	}


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
			inline void logging_mouse_input<BaseT>::mouse_move(int depressed, int x, int y)
			{	events_log.push_back(me_move(depressed, x, y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::mouse_down(mouse_input::mouse_buttons button, int depressed, int x, int y)
			{	events_log.push_back(me_down(button, depressed, x, y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::mouse_up(mouse_input::mouse_buttons button, int depressed, int x, int y)
			{	events_log.push_back(me_up(button, depressed, x, y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::mouse_double_click(mouse_input::mouse_buttons button, int depressed, int x, int y)
			{	events_log.push_back(me_double_click(button, depressed, x, y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::mouse_scroll(int depressed, int x, int y, int delta_x, int delta_y)
			{	events_log.push_back(me_scroll(depressed, x, y, delta_x, delta_y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::lost_capture()
			{	++capture_lost;	}


			template <typename BaseT>
			inline logging_key_input<BaseT>::logging_key_input()
			{	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::key_down(unsigned code, int modifiers)
			{	keyboard_event e = { keyboard_event::keydown, (int)code, modifiers }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::character(wchar_t /*symbol*/, unsigned /*repeats*/, int /*modifiers*/)
			{	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::key_up(unsigned code, int modifiers)
			{	keyboard_event e = { keyboard_event::keyup, (int)code, modifiers }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::got_focus()
			{	keyboard_event e = { keyboard_event::focusin, 0, 0 }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::lost_focus()
			{	keyboard_event e = { keyboard_event::focusout, 0, 0 }; events.push_back(e);	}
		}
	}
}

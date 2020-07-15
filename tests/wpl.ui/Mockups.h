#pragma once

#include "helpers.h"

#include <agge/color.h>
#include <agge/filling_rules.h>
#include <agge/figures.h>
#include <agge/path.h>
#include <map>
#include <wpl/view.h>
#include <wpl/layout.h>
#include <ut/assert.h>

namespace wpl
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


			template <typename BaseT>
			class logging_visual : public BaseT
			{
			public:
				std::vector< std::pair<int /*cx*/, int /*cy*/> > resize_log;
				mutable std::vector< std::pair<int /*cx*/, int /*cy*/> > surface_size_log;
				mutable std::vector<agge::rect_i> update_area_log;
				mutable std::vector<gcontext::rasterizer_type *> rasterizers_log;
				mutable std::vector< std::pair<gcontext::pixel_type, bool> > background_color;

			private:
				virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
				virtual void resize(unsigned cx, unsigned cy, visual::positioned_native_views &nviews);
			};


			class visual_with_native_view : public view
			{
			private:
				virtual void resize(unsigned cx, unsigned cy, positioned_native_views &nviews);
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



			class logging_layout_manager : public wpl::layout_manager
			{
			public:
				mutable std::vector< std::pair<unsigned, unsigned> > reposition_log;
				mutable std::vector<container::positioned_view> last_widgets;
				std::vector<view_location> positions;

			private:
				virtual void layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const;
			};


			class fill_layout : public wpl::layout_manager
			{
			private:
				virtual void layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const;
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
				rasterizers_log.push_back(rasterizer.get());
				background_color.push_back(std::make_pair(b.uniform_color, b.has_uniform_color));
			}

			template <typename BaseT>
			inline void logging_visual<BaseT>::resize(unsigned cx, unsigned cy, visual::positioned_native_views &/*nviews*/)
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
			{	events_log.push_back(me_up(button, depressed, x, y));	}

			template <typename BaseT>
			inline void logging_mouse_input<BaseT>::lost_capture()
			{	++capture_lost;	}


			template <typename BaseT>
			inline logging_key_input<BaseT>::logging_key_input()
			{	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::key_down(unsigned code, int modifiers)
			{	keyboard_event e = { keyboard_event::keydown, code, modifiers }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::character(wchar_t /*symbol*/, unsigned /*repeats*/, int /*modifiers*/)
			{	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::key_up(unsigned code, int modifiers)
			{	keyboard_event e = { keyboard_event::keyup, code, modifiers }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::got_focus()
			{	keyboard_event e = { keyboard_event::focusin, 0, 0 }; events.push_back(e);	}

			template <typename BaseT>
			inline void logging_key_input<BaseT>::lost_focus()
			{	keyboard_event e = { keyboard_event::focusout, 0, 0 }; events.push_back(e);	}
		}
	}
}

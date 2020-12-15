#pragma once

#include <wpl/keyboard_router.h>
#include <wpl/mouse_router.h>
#include <wpl/visual_router.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class keyboard_router_host : public wpl::keyboard_router_host
			{
			public:
				std::function<void (native_view &nview)> on_set_focus;

			private:
				virtual void set_focus(native_view &nview) override;
			};


			class visual_router_host : public wpl::visual_router_host
			{
			public:
				std::function<void (const agge::rect_i &area)> on_invalidate;

			private:
				virtual void invalidate(const agge::rect_i &area) override;
			};


			class mouse_router_host : public wpl::mouse_router_host
			{
			public:
				std::function<void (std::shared_ptr<keyboard_input> input)> on_request_focus;
				std::function<std::shared_ptr<void> ()> on_capture_mouse;

			private:
				virtual void request_focus(std::shared_ptr<keyboard_input> input) override;
				virtual std::shared_ptr<void> capture_mouse() override;
			};



			inline void keyboard_router_host::set_focus(native_view &nview)
			{
				if (on_set_focus)
					on_set_focus(nview);
			}


			inline void visual_router_host::invalidate(const agge::rect_i &area)
			{
				if (on_invalidate)
					on_invalidate(area);
			}


			inline void mouse_router_host::request_focus(std::shared_ptr<keyboard_input> input)
			{
				if (on_request_focus)
					on_request_focus(input);
			}

			inline std::shared_ptr<void> mouse_router_host::capture_mouse()
			{	return on_capture_mouse ? on_capture_mouse() : nullptr;	}
		}
	}
}

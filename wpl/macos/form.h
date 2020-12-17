#pragma once

#include "../control.h"
#include "../factory_context.h"
#include "../form.h"
#include "../keyboard_router.h"
#include "../mouse_router.h"
#include "../visual_router.h"

#include <memory>
#include <vector>

#ifdef __OBJC__
	@class form_macos;
	@class NSView;
	@class window_macos;
#else
	struct form_macos;
	struct NSView;
	struct window_macos;
#endif

namespace wpl
{
	namespace macos
	{
		class routers_host;
	}
}

namespace wpl
{
	namespace macos
	{
		class routers_host : public keyboard_router_host, public mouse_router_host, public visual_router_host
		{
		public:
			routers_host(NSView *native_view);
			
		private:
			virtual void set_focus(native_view &nview) override;
			virtual void request_focus(std::shared_ptr<keyboard_input> input) override;
			virtual std::shared_ptr<void> capture_mouse() override;
			virtual void invalidate(const agge::rect_i &area) override;

		private:
			NSView *_native_view;
		};
			
		class form : public wpl::form, noncopyable
		{
		public:
			form(const wpl::form_context &context);
			~form();
			
		private:
			virtual void set_root(std::shared_ptr<control> root) override;
			virtual view_location get_location() const override;
			virtual void set_location(const view_location &location) override;
			virtual void set_visible(bool value) override;
			virtual void set_caption(const std::wstring &caption) override;
			virtual void set_caption_icon(const gcontext::surface_type &icon) override;
			virtual void set_task_icon(const gcontext::surface_type &icon) override;
			virtual std::shared_ptr<wpl::form> create_child() override;
			virtual void set_features(unsigned /*features*/ features_) override;
			
		private:
			window_macos *_window;
			form_macos *_view;
			std::vector<slot_connection> _connections;
		};
	}
}

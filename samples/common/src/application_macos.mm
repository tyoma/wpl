#include "../application.h"

#include "../stylesheet.h"

#include <wpl/macos/cursor_manager.h>
#include <wpl/factory.h>
#include <wpl/freetype2/font_loader.h>

#include <chrono>
#include <Cocoa/Cocoa.h>
#include <tasker/ui_queue.h>

using namespace std;
using namespace std::chrono;

namespace wpl
{
	class application::impl
	{
	public:
		impl()
			: _application([NSApplication sharedApplication])
		{
			[_application setActivationPolicy:NSApplicationActivationPolicyRegular];
			[_application activateIgnoringOtherApps:YES];
		}

		void run()
		{	[_application run];	}

		void exit()
		{
			[_application stop:nil];
			[_application postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
				location:NSMakePoint(0,0) modifierFlags:0 timestamp:0 windowNumber:0 context:nil
				subtype:0 data1:0 data2:0] atStart:YES];
		}

	private:
		const NSApplication *_application;
	};

	application::application()
		: _impl(new impl), _ui_queue(new tasker::ui_queue([] {
			return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
		})), _queue([this] (queue_task task, timespan defer_by) {
			_ui_queue->schedule(std::move(task), milliseconds(defer_by));
			return true;
		})
	{	}

	application::~application()
	{	}

	shared_ptr<factory> application::create_default_factory() const
	{
		const auto text_engine = create_text_engine();
		const auto stylesheet_ = create_sample_stylesheet(text_engine);
		factory_context context = {
			make_shared<gcontext::surface_type>(1, 1, 16),
			make_shared<gcontext::renderer_type>(2),
			text_engine,
			stylesheet_,
			make_shared<macos::cursor_manager>(),
			[] {	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();	},
			_queue,
		};

		return factory::create_default(context);
	}

	queue application::get_application_queue() const
	{	return _queue;	}

	void application::run()
	{	_impl->run();	}

	void application::exit()
	{	_impl->exit();	}
}

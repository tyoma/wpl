#include "../application.h"

#include "../stylesheet.h"

#include <wpl/factory.h>
#include <wpl/freetype2/font_loader.h>
#include <Cocoa/Cocoa.h>

using namespace std;

namespace wpl
{
	class application::impl
	{
	public:
		impl()
		{
			_pool = [[NSAutoreleasePool alloc] init];
			_application = [NSApplication sharedApplication];

			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			[NSApp activateIgnoringOtherApps:YES];
		}
		
		~impl()
		{	[_pool drain];	}
		
		void run()
		{	[_application run];	}
	
		void exit()
		{	[_application stop:nil];	}

	private:
		NSAutoreleasePool *_pool;
		NSApplication *_application;
	};
	
	application::application()
		: _impl(new impl), _queue([] (const queue_task &, timespan) {	return false;	})
	{	}
	
	application::~application()
	{	}

	shared_ptr<factory> application::create_default_factory() const
	{
		const auto text_engine = create_text_engine();
		const auto stylesheet_ = create_sample_stylesheet(text_engine);
		factory_context context = {
			shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
			shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(2)),
			text_engine,
			stylesheet_,
			nullptr,
			[] {	return timestamp();	},
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

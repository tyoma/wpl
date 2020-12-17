#include "../application.h"

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
		: _impl(new impl)
	{	}
	
	application::~application()
	{	}
	
	void application::run()
	{	_impl->run();	}
	
	void application::exit()
	{	_impl->exit();	}
}

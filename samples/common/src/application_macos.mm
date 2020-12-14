#include "../application.h"

#include <Cocoa/Cocoa.h>

using namespace std;

@interface AppDelegate : NSObject<NSApplicationDelegate>
	{
	}
@end

@implementation AppDelegate
	- (id)init
	{	return [super init];	}

	- (void)dealloc
	{	[super dealloc];	}

	- (void)applicationWillFinishLaunching:(NSNotification *)notification
	{	}

	- (void)applicationDidFinishLaunching:(NSNotification *)notification
	{	}
@end

namespace wpl
{
	class application::impl
	{
	public:
		impl()
		{
			_pool = [[NSAutoreleasePool alloc] init];
			_application = [NSApplication sharedApplication];
			_applicationDelegate =[[[AppDelegate alloc] init] autorelease];

			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			[NSApp activateIgnoringOtherApps:YES];

			[_application setDelegate:_applicationDelegate];
		}
		
		~impl()
		{	[_pool drain];	}
		
		void run()
		{	[_application run];	}
	
		void exit()
		{	[_application stop];	}

	private:
		NSAutoreleasePool *_pool;
		NSApplication *_application;
		AppDelegate *_applicationDelegate;
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

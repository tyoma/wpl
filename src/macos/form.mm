#import <wpl/macos/form.h>

#include <Cocoa/Cocoa.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;
using namespace wpl;

@interface form_macos : NSView
	{
		wpl::form_context _context;
		shared_ptr<wpl::control> _root;
		vector<wpl::placed_view> _views;
		wpl::gcontext::rasterizer_ptr _rasterizer;
		wpl::slot_connection _layout_changed_connection;
		unique_ptr<wpl::macos::routers_host> _routers_host;
		unique_ptr<wpl::keyboard_router> _keyboard_router;
		unique_ptr<wpl::mouse_router> _mouse_router;
		unique_ptr<wpl::visual_router> _visual_router;
	}

	- (void) setFormContext:(wpl::form_context)context;
	- (void) setRoot:(shared_ptr<wpl::control>)root;
	- (void) setFrameSize:(NSSize)newSize;
	- (void) drawRect:(NSRect)dirtyRect;
@end

namespace wpl
{
	namespace macos
	{
		routers_host::routers_host(NSView *native_view)
			: _native_view(native_view)
		{	}
	
		void routers_host::set_focus(native_view &nview)
		{	}
		
		void routers_host::request_focus(shared_ptr<keyboard_input> input)
		{	}
		
		shared_ptr<void> routers_host::capture_mouse()
		{	return nullptr;	}
	
		void routers_host::invalidate(const agge::rect_i &/*area_*/)
		{
//			CGRect area = {
//				{	static_cast<double>(area_.x1), static_cast<double>(area_.y1)	},
//				{	static_cast<double>(wpl::width(area_)), static_cast<double>(wpl::height(area_))	},
//			};
			[_native_view  setNeedsDisplay:true];
		}
	
	
		form::form(const wpl::form_context &context)
		{
			NSRect content = NSMakeRect(0.0f, 0.0f, 600.0f, 400.0f);
			
			_window = [[[NSWindow alloc] autorelease]
				initWithContentRect:content
				styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskResizable)
				backing:NSBackingStoreBuffered
				defer:YES];
			_view = [[[form_macos alloc] autorelease]
				initWithFrame:content];
			[_view setFormContext:context];
			[_window setContentView:_view];
		}
			
		form::~form()
		{
			[_view release];
			[_window release];
		}
			
		void form::set_root(shared_ptr<control> root)
		{	[_view setRoot:root];	}
			
		view_location form::get_location() const
		{	throw 0;	}
			
		void form::set_location(const view_location &location)
		{	}
			
		void form::set_visible(bool value)
		{	[_window setIsVisible:value];	}
			
		void form::set_caption(const wstring &caption)
		{	}
			
		void form::set_caption_icon(const gcontext::surface_type &icon)
		{	}
			
		void form::set_task_icon(const gcontext::surface_type &icon)
		{	}
			
		shared_ptr<wpl::form> form::create_child()
		{	throw 0;	}
			
		void form::set_features(unsigned /*features*/ features_)
		{	}
	}
}

@implementation form_macos
	- (void) setFormContext:(wpl::form_context)context
	{	_context = context;	}

	- (void) setRoot:(shared_ptr<wpl::control>)root
	{
		const auto layout = [self] {
			auto f = [self frame];
			const agge::box<int> b = { 2 * static_cast<int>(f.size.width), 2 * static_cast<int>(f.size.height) };

			[self layout_views:b];
		};
		const auto reload = [self] {
			_visual_router->reload_views();
			_mouse_router->reload_views();
			_keyboard_router->reload_views();
		};

		_root = root;
		layout();
		reload();
		_layout_changed_connection = root ? root->layout_changed += [layout, reload] (bool hierarchy_changed) {
			layout();
			if (hierarchy_changed)
				reload();
		} : slot_connection();
	}
	
	-(void) layout_views:(box<int>)size
	{
		_views.clear();
		if (_root)
			_root->layout([self] (const placed_view &pv) {	_views.emplace_back(pv);	}, size);
		_context.backbuffer->resize(size.w, size.h);
	}

	- (id) initWithFrame:(NSRect)frameRect
	{
		_rasterizer.reset(new gcontext::rasterizer_type);
		_routers_host.reset(new wpl::macos::routers_host(self));
		_keyboard_router.reset(new wpl::keyboard_router(_views, *_routers_host));
		_mouse_router.reset(new wpl::mouse_router(_views, *_routers_host));
		_visual_router.reset(new wpl::visual_router(_views, *_routers_host));
	
		self = [super initWithFrame:frameRect];
		[self scaleUnitSquareToSize:NSMakeSize(0.5, 0.5)];
		return self;
	}

	- (void) setFrameSize:(NSSize)newSize
	{
		const agge::box<int> b = { 2 * static_cast<int>(newSize.width), 2 * static_cast<int>(newSize.height) };

		[self layout_views:b];
		[super setFrameSize:newSize];
	}

	- (void) drawRect:(NSRect)dirtyRect
	{
		const vector_i offset = {};
		gcontext ctx(*_context.backbuffer, *_context.renderer, *_context.text_engine, offset);
		
		_visual_router->draw(ctx, _rasterizer);
		_context.backbuffer->blit([[NSGraphicsContext currentContext]CGContext], 0, 0,
			_context.backbuffer->width(), _context.backbuffer->height());
	}
@end

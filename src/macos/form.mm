//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#import <wpl/macos/form.h>

#include <Cocoa/Cocoa.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;
using namespace wpl;

#if TARGET_RT_BIG_ENDIAN
	const NSStringEncoding kEncoding_wchar_t = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32BE);
#else
	const NSStringEncoding kEncoding_wchar_t = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
#endif

@interface window_macos : NSWindow
	{
		@public wpl::signal<void ()> close_;
	}
	
	- (void) close;
@end

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
			[_native_view  setNeedsDisplay:YES];
		}
	
	
		form::form(const wpl::form_context &context)
		{
			NSRect content = NSMakeRect(0.0f, 0.0f, 600.0f, 400.0f);
			
			_window = [[[window_macos alloc] autorelease]
				initWithContentRect:content
				styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable)
				backing:NSBackingStoreBuffered
				defer:YES];
			_view = [[[form_macos alloc] autorelease]
				initWithFrame:content];
			[_view setFormContext:context];
			[_window setContentView:_view];
			_connections.push_back(_window->close_ += [this] {	close();	});
		}
			
		form::~form()
		{
			[_window setContentView:nil];
			[_view setRoot:nullptr];
			_connections.clear();
		}
			
		void form::set_root(shared_ptr<control> root)
		{	[_view setRoot:root];	}
			
		wpl::rect_i form::get_location() const
		{	throw 0;	}
			
		void form::set_location(const wpl::rect_i &location)
		{	}
			
		void form::set_visible(bool value)
		{	[_window setIsVisible:value];	}
			
		void form::set_caption(const wstring &caption)
		{
			[_window setTitle:[[NSString alloc]
				initWithBytes:caption.c_str() length:caption.size() encoding:kEncoding_wchar_t]];
		}
			
		void form::set_caption_icon(const gcontext::surface_type &icon)
		{	}
			
		void form::set_task_icon(const gcontext::surface_type &icon)
		{	}
			
		shared_ptr<wpl::form> form::create_child()
		{	throw 0;	}
			
		void form::set_features(unsigned /*features*/ features_)
		{
			auto styleMask = [_window styleMask];
			
			styleMask |= (form::resizeable & features_) ? NSWindowStyleMaskResizable : 0;
			styleMask &= !(form::resizeable & features_) ? ~NSWindowStyleMaskResizable : 0;
			styleMask |= (form::minimizable & features_) ? NSWindowStyleMaskMiniaturizable : 0;
			styleMask &= !(form::minimizable & features_) ? ~NSWindowStyleMaskMiniaturizable : 0;
			[_window setStyleMask:styleMask];
		}
	}
}

@implementation window_macos
	- (void) close
	{	close_();	}
@end

@implementation form_macos
	- (void) setFormContext:(wpl::form_context)context
	{
		_context = context;
		_context.backbuffer->resize(800, 700);
	}

	- (void) setRoot:(shared_ptr<wpl::control>)root
	{
		const auto layout = [self] {
			[self layout_views:[self frame].size];
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
	
	-(void) layout_views:(NSSize)size_
	{
		const agge::box<int> size = { static_cast<int>(size_.width), static_cast<int>(size_.height) };
		
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
	
		const auto options = NSTrackingActiveAlways | NSTrackingInVisibleRect |NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved;
		auto ta = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];

		[super initWithFrame:frameRect];
		[self addTrackingArea:ta];
	
		return [super initWithFrame:frameRect];
	}

	- (void) setFrameSize:(NSSize)newSize
	{
		[self layout_views:newSize];
		[super setFrameSize:newSize];
	}

	- (void) drawRect:(NSRect)dirtyRect
	{
		const vector_i offset = {};
		gcontext ctx(*_context.backbuffer, *_context.renderer, *_context.text_engine, offset);
		const auto context = [[NSGraphicsContext currentContext] CGContext];
		
		_visual_router->draw(ctx, _rasterizer);
		_context.backbuffer->blit(context, 0, 0, _context.backbuffer->width(), _context.backbuffer->height());
	}
	
	- (void) mouseEvent:(NSEvent *)event
	{
		const auto p = [event locationInWindow];
		const agge::point<int> point = {	static_cast<int>(p.x), static_cast<int>([self frame].size.height - p.y)	};
		void (mouse_input::*method)(mouse_input::mouse_buttons, int, int, int) = nullptr;
		mouse_input::mouse_buttons button;
		
		switch (event.type)
		{
		case NSEventTypeLeftMouseDown: case NSEventTypeRightMouseDown:
			method = &mouse_input::mouse_down;
			break;
			
		case NSEventTypeLeftMouseUp: case NSEventTypeRightMouseUp:
			method = event.clickCount == 2 ? &mouse_input::mouse_double_click: &mouse_input::mouse_up;
			break;
			
		case NSEventTypeMouseMoved: case NSEventTypeLeftMouseDragged: case NSEventTypeRightMouseDragged:
			_mouse_router->mouse_move(0, point);
			
		default:
			return;
		}
		switch (event.type)
		{
		case NSEventTypeLeftMouseDown: case NSEventTypeLeftMouseUp:
			button = mouse_input::left;
			break;
			
		case NSEventTypeRightMouseDown: case NSEventTypeRightMouseUp:
			button = mouse_input::right;
			break;
			
		default:
			break;
		}
	
		if (method)
			_mouse_router->mouse_click(method, button, 0, point);
	}
	
	- (void) mouseDown:(NSEvent *)event
	{	[self mouseEvent:event];	}

	- (void) mouseUp:(NSEvent *)event
	{	[self mouseEvent:event];	}

	- (void) mouseMoved:(NSEvent *)event
	{	[self mouseEvent:event];	}

	- (void) mouseDragged:(NSEvent *)event
	{	[self mouseEvent:event];	}

	- (void) mouseExited:(NSEvent *)event
	{	_mouse_router->mouse_leave();	}
@end

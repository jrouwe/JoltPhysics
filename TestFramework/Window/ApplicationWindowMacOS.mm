// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowMacOS.h>

#import <MetalKit/MetalKit.h>

// This class implements a metal view
@interface MetalView : MTKView <MTKViewDelegate>
@end

@implementation MetalView
{
	ApplicationWindowMacOS *mWindow;
}

- (MetalView *)init:(ApplicationWindowMacOS *)window
{
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame: NSMakeRect(0, 0, window->GetWindowWidth(), window->GetWindowHeight()) device: device];
	[device release];

	mWindow = window;
	
	self.delegate = self;
	
	return self;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)canBecomeKeyView
{
	return YES;
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)mouseMoved:(NSEvent *)event
{
	NSPoint location_in_view = [self convertPoint: event.locationInWindow fromView: nil];
	NSPoint location_in_backing = [self convertPointToBacking: location_in_view];
	mWindow->OnMouseMoved(location_in_backing.x, -location_in_backing.y);
}

- (void)drawInMTKView:(MTKView *)view
{
	@autoreleasepool
	{
		mWindow->RenderCallback();
	}
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
	mWindow->OnWindowResized(size.width, size.height);
}

@end

// This is the main application delegate that tells us if we're starting / stopping
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
	// Add the Quit button to the first menu item on the toolbar
	NSMenu *app_menu = [[NSApp mainMenu] itemAtIndex: 0].submenu;
	NSMenuItem *quit_item = [[NSMenuItem alloc] initWithTitle: @"Quit" action: @selector(terminate:) keyEquivalent: @"q"];
	[app_menu addItem: quit_item];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	// Close the app when the window is closed
	return YES;
}

@end

ApplicationWindowMacOS::~ApplicationWindowMacOS()
{
	[mMetalView release];
}

void ApplicationWindowMacOS::Initialize(const char *inTitle)
{
	// Create metal view
	MetalView *view = [[MetalView alloc] init: this];
	view.clearColor = MTLClearColorMake(0.098f, 0.098f, 0.439f, 1.000f);
	view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
	view.clearDepth = 0.0f;
	mMetalView = view;

	// Create window
	NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, mWindowWidth, mWindowHeight)
												   styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
													 backing: NSBackingStoreBuffered
													   defer: NO];
	window.contentView = view;
	[window setAcceptsMouseMovedEvents: YES];
	[window setTitle: [NSString stringWithCString: inTitle encoding: NSUTF8StringEncoding]];
	[window makeKeyAndOrderFront: nil];
}

void ApplicationWindowMacOS::MainLoop(ApplicationWindow::RenderCallback inRenderCallback)
{
	mRenderCallback = inRenderCallback;
	
	@autoreleasepool
	{
		NSApplication *app = [NSApplication sharedApplication];
		AppDelegate *delegate = [[AppDelegate alloc] init];
		[app setDelegate: delegate];
		[app run];
		[delegate release];
	}
}

CAMetalLayer *ApplicationWindowMacOS::GetMetalLayer() const
{
	return (CAMetalLayer *)mMetalView.layer;
}

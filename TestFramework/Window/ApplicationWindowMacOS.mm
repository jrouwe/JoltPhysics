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
	[super initWithFrame: NSMakeRect(0, 0, window->GetWindowWidth(), window->GetWindowHeight()) device: MTLCreateSystemDefaultDevice()];
	
	mWindow = window;
	
	self.delegate = self;
	
	return self;
}

- (bool)acceptsFirstResponder
{
	return YES;
}

- (bool)canBecomeKeyView
{
	return YES;
}

- (BOOL)isFlipped {
    return YES;
}

- (void)mouseMoved:(NSEvent *)event
{
    NSPoint locationInView = [self convertPoint:event.locationInWindow fromView:nil];
    NSPoint locationInBacking = [self convertPointToBacking:locationInView];
	mWindow->OnMouseMoved(locationInBacking.x, -locationInBacking.y);
}

- (void)drawInMTKView:(MTKView *)view
{
	mWindow->RenderCallback();
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
	NSMenuItem *quit_item = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
	[app_menu addItem:quit_item];
}

-(bool)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	// Close the app when the window is closed
	return YES;
}

@end

void ApplicationWindowMacOS::Initialize()
{
	// Create metal view
	MetalView *view = [[MetalView alloc] init: this];
	mMetalView = view;

	// Create window
	NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, mWindowWidth, mWindowHeight)
												   styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
													 backing: NSBackingStoreBuffered
													   defer: NO];
	window.contentView = view;
	[window setAcceptsMouseMovedEvents: YES];
	[window setTitle: @"TestFramework"];
	[window makeKeyAndOrderFront: nil];
}

void ApplicationWindowMacOS::MainLoop(ApplicationWindow::RenderCallback inRenderCallback)
{
	mRenderCallback = inRenderCallback;
	
	@autoreleasepool
	{
		NSApplication *app = [NSApplication sharedApplication];
		AppDelegate *delegate = [[AppDelegate alloc] init];
		[app setDelegate:delegate];
		[app run];
	}
}

CAMetalLayer *ApplicationWindowMacOS::GetMetalLayer() const
{
	return (CAMetalLayer *)mMetalView.layer;
}

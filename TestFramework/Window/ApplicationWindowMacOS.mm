// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowMacOS.h>

#import <MetalKit/MetalKit.h>

// This class receives updates from MTKView and is used to trigger rendering a frame
@interface MetalDelegate : NSObject <MTKViewDelegate>
@end

@implementation MetalDelegate
{
	ApplicationWindowMacOS *mWindow;
}

- (MetalDelegate *)init:(ApplicationWindowMacOS *)window
{
	mWindow = window;
	return self;
}

- (void)drawInMTKView:(MTKView *)view
{
	mWindow->RenderCallback();
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
}

@end

// This is the main application delegate that tells us if we're starting / stopping
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

-(bool)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	// Close the app when the window is closed
	return YES;
}

@end

void ApplicationWindowMacOS::Initialize()
{
	// Create metal view
	CGRect rect = NSMakeRect(0, 0, mWindowWidth, mWindowHeight);
	MTKView *view = [[MTKView alloc] initWithFrame: rect device: MTLCreateSystemDefaultDevice()];
	view.delegate = [[MetalDelegate new] init: this];
	mMetalLayer = (CAMetalLayer *)view.layer;

	// Create window
	NSWindow *window = [[NSWindow alloc] initWithContentRect: rect
												   styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
													 backing: NSBackingStoreBuffered
														defer: NO];
	window.contentView = view;
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

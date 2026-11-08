// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowMacOS.h>

#import <MetalKit/MetalKit.h>
#import <GameController/GameController.h>

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
	if (mKeyDownMonitor != nullptr)
		[NSEvent removeMonitor: (id)mKeyDownMonitor];

	if (mMouseConnectObserver != nullptr)
	{
		[NSNotificationCenter.defaultCenter removeObserver: (id)mMouseConnectObserver];

		// Remove the handlers as they reference this window
		for (GCMouse *mouse in GCMouse.mice)
		{
			GCMouseInput *mouse_input = mouse.mouseInput;
			mouse_input.mouseMovedHandler = nil;
			mouse_input.leftButton.pressedChangedHandler = nil;
			mouse_input.rightButton.pressedChangedHandler = nil;
			mouse_input.middleButton.pressedChangedHandler = nil;
		}
	}
}

void ApplicationWindowMacOS::Initialize(const char *inTitle)
{
	// Create metal view
	MetalView *view = [[MetalView alloc] init: this];
	SetMetalView(view);

	// Create window
	NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, mWindowWidth, mWindowHeight)
												   styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
													 backing: NSBackingStoreBuffered
													   defer: NO];
	window.contentView = view;
	[window setAcceptsMouseMovedEvents: YES];
	[window setTitle: [NSString stringWithCString: inTitle encoding: NSUTF8StringEncoding]];
	[window makeKeyAndOrderFront: nil];

	// Key presses are received through the GameController framework, swallow the key down events to avoid the system beeping
	mKeyDownMonitor = [NSEvent addLocalMonitorForEventsMatchingMask: NSEventMaskKeyDown handler: ^NSEvent *(NSEvent *event) {
		// Ignore all keystrokes except Command-Q (Quit)
		if ((event.modifierFlags & NSEventModifierFlagCommand) && [event.charactersIgnoringModifiers isEqual: @"q"])
			return event;
		else
			return nil;
	}];

	// Listen for keyboards
	ListenForKeyboards();

	// Listen for mice being connected to receive relative mouse movement (will work also when mouse is outside the window or at the edge of the screen) and button presses
	ApplicationWindowMacOS *window_macos = this;
	mMouseConnectObserver = [NSNotificationCenter.defaultCenter addObserverForName: GCMouseDidConnectNotification object: nil queue: nil usingBlock: ^(NSNotification *notification) {
		GCMouseInput *mouse_input = ((GCMouse *)notification.object).mouseInput;
		if (mouse_input == nil)
			return;

		mouse_input.mouseMovedHandler = ^(GCMouseInput *, float inDeltaX, float inDeltaY) { window_macos->OnMouseDelta(int(inDeltaX), int(-inDeltaY)); };
		mouse_input.leftButton.pressedChangedHandler = ^(GCControllerButtonInput *, float, BOOL inPressed) { window_macos->OnMouseButton(MouseMacOS::EMouseButton::Left, inPressed); };
		mouse_input.rightButton.pressedChangedHandler = ^(GCControllerButtonInput *, float, BOOL inPressed) { window_macos->OnMouseButton(MouseMacOS::EMouseButton::Right, inPressed); };
		mouse_input.middleButton.pressedChangedHandler = ^(GCControllerButtonInput *, float, BOOL inPressed) { window_macos->OnMouseButton(MouseMacOS::EMouseButton::Middle, inPressed); };
	}];
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

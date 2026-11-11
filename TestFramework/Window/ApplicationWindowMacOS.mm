// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowMacOS.h>

#import <MetalKit/MetalKit.h>
#import <GameController/GameController.h>

static EKey sToKey(GCKeyCode inValue)
{
	if (inValue == GCKeyCodeKeyA) return EKey::A;
	if (inValue == GCKeyCodeKeyB) return EKey::B;
	if (inValue == GCKeyCodeKeyC) return EKey::C;
	if (inValue == GCKeyCodeKeyD) return EKey::D;
	if (inValue == GCKeyCodeKeyE) return EKey::E;
	if (inValue == GCKeyCodeKeyF) return EKey::F;
	if (inValue == GCKeyCodeKeyG) return EKey::G;
	if (inValue == GCKeyCodeKeyH) return EKey::H;
	if (inValue == GCKeyCodeKeyI) return EKey::I;
	if (inValue == GCKeyCodeKeyJ) return EKey::J;
	if (inValue == GCKeyCodeKeyK) return EKey::K;
	if (inValue == GCKeyCodeKeyL) return EKey::L;
	if (inValue == GCKeyCodeKeyM) return EKey::M;
	if (inValue == GCKeyCodeKeyN) return EKey::N;
	if (inValue == GCKeyCodeKeyO) return EKey::O;
	if (inValue == GCKeyCodeKeyP) return EKey::P;
	if (inValue == GCKeyCodeKeyQ) return EKey::Q;
	if (inValue == GCKeyCodeKeyR) return EKey::R;
	if (inValue == GCKeyCodeKeyS) return EKey::S;
	if (inValue == GCKeyCodeKeyT) return EKey::T;
	if (inValue == GCKeyCodeKeyU) return EKey::U;
	if (inValue == GCKeyCodeKeyV) return EKey::V;
	if (inValue == GCKeyCodeKeyW) return EKey::W;
	if (inValue == GCKeyCodeKeyX) return EKey::X;
	if (inValue == GCKeyCodeKeyY) return EKey::Y;
	if (inValue == GCKeyCodeKeyZ) return EKey::Z;
	if (inValue == GCKeyCodeZero) return EKey::Num0;
	if (inValue == GCKeyCodeOne) return EKey::Num1;
	if (inValue == GCKeyCodeTwo) return EKey::Num2;
	if (inValue == GCKeyCodeThree) return EKey::Num3;
	if (inValue == GCKeyCodeFour) return EKey::Num4;
	if (inValue == GCKeyCodeFive) return EKey::Num5;
	if (inValue == GCKeyCodeSix) return EKey::Num6;
	if (inValue == GCKeyCodeSeven) return EKey::Num7;
	if (inValue == GCKeyCodeEight) return EKey::Num8;
	if (inValue == GCKeyCodeNine) return EKey::Num9;
	if (inValue == GCKeyCodeSpacebar) return EKey::Space;
	if (inValue == GCKeyCodeComma) return EKey::Comma;
	if (inValue == GCKeyCodePeriod) return EKey::Period;
	if (inValue == GCKeyCodeEscape) return EKey::Escape;
	if (inValue == GCKeyCodeLeftShift) return EKey::LShift;
	if (inValue == GCKeyCodeRightShift) return EKey::RShift;
	if (inValue == GCKeyCodeLeftControl) return EKey::LControl;
	if (inValue == GCKeyCodeRightControl) return EKey::RControl;
	if (inValue == GCKeyCodeLeftAlt) return EKey::LAlt;
	if (inValue == GCKeyCodeRightAlt) return EKey::RAlt;
	if (inValue == GCKeyCodeLeftArrow) return EKey::Left;
	if (inValue == GCKeyCodeRightArrow) return EKey::Right;
	if (inValue == GCKeyCodeUpArrow) return EKey::Up;
	if (inValue == GCKeyCodeDownArrow) return EKey::Down;
	if (inValue == GCKeyCodeReturnOrEnter) return EKey::Return;
	return EKey::Unknown;
}

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

	if (mKeyboardConnectObserver != nullptr)
	{
		[NSNotificationCenter.defaultCenter removeObserver: (id)mKeyboardConnectObserver];

		// Remove the handler as it references this window
		GCKeyboard.coalescedKeyboard.keyboardInput.keyChangedHandler = nil;
	}

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

	ApplicationWindowMacOS *window_macos = this;

	// Listen for keyboards being connected, key presses will be reported through OnKey
	mKeyboardConnectObserver = [NSNotificationCenter.defaultCenter addObserverForName: GCKeyboardDidConnectNotification object: nil queue: nil usingBlock: ^(NSNotification *notification) {
		GCKeyboardInput *keyboard_input = ((GCKeyboard *)notification.object).keyboardInput;
		if (keyboard_input == nil)
			return;

		keyboard_input.keyChangedHandler = ^(GCKeyboardInput *, GCControllerButtonInput *, GCKeyCode inKeyCode, BOOL inPressed) {
			EKey key = sToKey(inKeyCode);
			if (key != EKey::Invalid)
				window_macos->OnKey(key, inPressed);
		};
	}];

	// Listen for mice being connected to receive relative mouse movement (will work also when mouse is outside the window or at the edge of the screen) and button presses
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

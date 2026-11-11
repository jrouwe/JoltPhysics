// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowIOS.h>

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>
#include <crt_externs.h>

ApplicationWindowIOS *ApplicationWindowIOS::sInstance = nullptr;

// Touch gesture tuning parameters (in points)
static constexpr CGFloat cLookSensitivity = 0.5; // Mouse delta units per point that the finger moves
static constexpr CGFloat cTapMaxMovement = 15.0;
static constexpr NSTimeInterval cTapMaxDuration = 0.3;
static constexpr NSTimeInterval cPressDelay = 0.1;
static constexpr NSTimeInterval cLongPressDuration = 0.5;

// Convert a UIKit key code to our own key code
static EKey sToKey(UIKeyboardHIDUsage inKeyCode)
{
	if (inKeyCode >= UIKeyboardHIDUsageKeyboardA && inKeyCode <= UIKeyboardHIDUsageKeyboardZ)
		return EKey(int(EKey::A) + int(inKeyCode - UIKeyboardHIDUsageKeyboardA));
	if (inKeyCode >= UIKeyboardHIDUsageKeyboard1 && inKeyCode <= UIKeyboardHIDUsageKeyboard9)
		return EKey(int(EKey::Num1) + int(inKeyCode - UIKeyboardHIDUsageKeyboard1));

	switch (inKeyCode)
	{
	case UIKeyboardHIDUsageKeyboard0:				return EKey::Num0;
	case UIKeyboardHIDUsageKeyboardSpacebar:		return EKey::Space;
	case UIKeyboardHIDUsageKeyboardComma:			return EKey::Comma;
	case UIKeyboardHIDUsageKeyboardPeriod:			return EKey::Period;
	case UIKeyboardHIDUsageKeyboardEscape:			return EKey::Escape;
	case UIKeyboardHIDUsageKeyboardLeftShift:		return EKey::LShift;
	case UIKeyboardHIDUsageKeyboardRightShift:		return EKey::RShift;
	case UIKeyboardHIDUsageKeyboardLeftControl:		return EKey::LControl;
	case UIKeyboardHIDUsageKeyboardRightControl:	return EKey::RControl;
	case UIKeyboardHIDUsageKeyboardLeftAlt:			return EKey::LAlt;
	case UIKeyboardHIDUsageKeyboardRightAlt:		return EKey::RAlt;
	case UIKeyboardHIDUsageKeyboardLeftArrow:		return EKey::Left;
	case UIKeyboardHIDUsageKeyboardRightArrow:		return EKey::Right;
	case UIKeyboardHIDUsageKeyboardUpArrow:			return EKey::Up;
	case UIKeyboardHIDUsageKeyboardDownArrow:		return EKey::Down;
	case UIKeyboardHIDUsageKeyboardReturnOrEnter:	return EKey::Return;
	case UIKeyboardHIDUsageKeypadEnter:				return EKey::Return;
	default:										return EKey::Unknown;
	}
}

// Type of touch gesture that is in progress
enum class EGesture
{
	None,
	OneFinger,
	MultiFinger
};

// This class implements a metal view that handles touches
@interface MetalView : MTKView <MTKViewDelegate>
@end

@implementation MetalView
{
	ApplicationWindowIOS *mWindow;
	bool				mInDraw;			// If we're currently rendering a frame

	// Gesture state
	EGesture			mGesture;
	NSTimeInterval		mGestureStartTime;
	NSUInteger			mGestureMaxTouches;
	bool				mGestureMoved;
	bool				mPressPending;		// Left button press has not been sent yet (we wait a bit to see if more fingers touch the screen)
	CGPoint				mGestureStartPos;
	CGPoint				mLastPos;
	CGPoint				mDeltaRemainder;	// Fractional part of the mouse delta that has not been sent yet
	bool				mLongPress;			// If the Space key is held because of a long press
}

- (MetalView *)init:(ApplicationWindowIOS *)window
{
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame: CGRectMake(0, 0, window->GetWindowWidth(), window->GetWindowHeight()) device: device];
	[device release];

	mWindow = window;
	mGesture = EGesture::None;
	mLongPress = false;
	mInDraw = false;

	self.delegate = self;
	self.multipleTouchEnabled = YES;

	return self;
}

- (void)drawInMTKView:(MTKView *)view
{
	// A fatal error alert runs a nested run loop from within the frame, don't start a new frame from there
	if (mInDraw)
		return;
	mInDraw = true;

	@autoreleasepool
	{
		// Send the delayed press if the finger stayed down long enough
		NSTimeInterval gesture_duration = [NSProcessInfo processInfo].systemUptime - mGestureStartTime;
		if (mPressPending && gesture_duration > cPressDelay)
			[self sendPress];

		// A long press with one finger holds the Space key (drags the object under the cursor)
		if (mGesture == EGesture::OneFinger && !mGestureMoved && !mLongPress && gesture_duration > cLongPressDuration)
		{
			mLongPress = true;
			mWindow->OnKey(EKey::Space, true);
		}

		mWindow->RenderCallback();
	}

	mInDraw = false;
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
	mWindow->OnWindowResized(int(size.width), int(size.height));
}

// Get all touches that are still on the screen
- (NSArray<UITouch *> *)activeTouches:(UIEvent *)event
{
	NSMutableArray<UITouch *> *touches = [NSMutableArray array];
	for (UITouch *t in [event touchesForView: self])
		if (t.phase != UITouchPhaseEnded && t.phase != UITouchPhaseCancelled)
			[touches addObject: t];
	return touches;
}

// Get the average position of a set of touches
- (CGPoint)centroid:(NSArray<UITouch *> *)touches
{
	CGPoint c = CGPointMake(0, 0);
	for (UITouch *t in touches)
	{
		CGPoint p = [t locationInView: self];
		c.x += p.x;
		c.y += p.y;
	}
	if (touches.count > 0)
	{
		c.x /= touches.count;
		c.y /= touches.count;
	}
	return c;
}

// Report a position in points to the window as a mouse position in pixels
- (void)sendMousePosition:(CGPoint)pos
{
	CGFloat scale = self.contentScaleFactor;
	mWindow->OnMouseMoved(int(pos.x * scale), int(pos.y * scale));
}

- (void)sendPress
{
	mPressPending = false;
	mWindow->OnMouseButton(MouseMacOS::EMouseButton::Left, true);
}

- (void)tapKey:(EKey)key
{
	mWindow->OnKey(key, true);
	mWindow->OnKey(key, false);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	NSArray<UITouch *> *active = [self activeTouches: event];
	if (active.count == 0)
		return;

	if (mGesture == EGesture::None)
	{
		// Start of a new gesture
		mGestureStartTime = event.timestamp;
		mGestureMaxTouches = 0;
		mGestureMoved = false;
	}

	mGestureMaxTouches = std::max(mGestureMaxTouches, active.count);

	if (active.count == 1 && mGesture == EGesture::None)
	{
		// One finger: Acts as a mouse with the left button pressed
		mGesture = EGesture::OneFinger;
		mLastPos = mGestureStartPos = [active[0] locationInView: self];
		mDeltaRemainder = CGPointMake(0, 0);
		[self sendMousePosition: mLastPos];
		mPressPending = true;
	}
	else if (active.count > 1)
	{
		// Multiple fingers: Cancel the mouse press and use the center of the fingers to pan the camera
		if (mGesture == EGesture::OneFinger)
		{
			if (!mPressPending)
			{
				// The press was already sent, move the cursor off screen before releasing so that the release doesn't click the UI element under the first finger
				mWindow->OnMouseMoved(-1, -1);
				mWindow->OnMouseButton(MouseMacOS::EMouseButton::Left, false);
			}
			mPressPending = false;
			if (mLongPress)
			{
				mLongPress = false;
				mWindow->OnKey(EKey::Space, false);
			}
		}
		mGesture = EGesture::MultiFinger;
		mGestureStartPos = mLastPos = [self centroid: active];
	}
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	NSArray<UITouch *> *active = [self activeTouches: event];
	if (active.count == 0)
		return;

	switch (mGesture)
	{
	case EGesture::OneFinger:
		{
			CGPoint pos = [active[0] locationInView: self];
			if (hypot(pos.x - mGestureStartPos.x, pos.y - mGestureStartPos.y) > cTapMaxMovement)
			{
				mGestureMoved = true;
				if (mPressPending)
					[self sendPress];
			}

			// Moving the finger drags the scene (so the camera rotates in the opposite direction)
			CGFloat dx = cLookSensitivity * (mLastPos.x - pos.x) + mDeltaRemainder.x;
			CGFloat dy = cLookSensitivity * (mLastPos.y - pos.y) + mDeltaRemainder.y;
			int idx = int(dx), idy = int(dy);
			mDeltaRemainder = CGPointMake(dx - idx, dy - idy);
			mWindow->OnMouseDelta(idx, idy);
			[self sendMousePosition: pos];
			mLastPos = pos;
		}
		break;

	case EGesture::MultiFinger:
		{
			// Pan the camera by the distance that the center of the fingers moved
			CGPoint pos = [self centroid: active];
			if (hypot(pos.x - mGestureStartPos.x, pos.y - mGestureStartPos.y) > cTapMaxMovement)
				mGestureMoved = true;
			mWindow->OnPan(float(pos.x - mLastPos.x), float(pos.y - mLastPos.y));
			mLastPos = pos;
		}
		break;

	case EGesture::None:
		break;
	}
}

- (void)endTouches:(UIEvent *)event cancelled:(bool)cancelled
{
	NSArray<UITouch *> *active = [self activeTouches: event];

	switch (mGesture)
	{
	case EGesture::OneFinger:
		if (active.count == 0)
		{
			// Ensure that a short tap results in a click
			if (mPressPending && !cancelled)
				[self sendPress];
			if (!mPressPending)
				mWindow->OnMouseButton(MouseMacOS::EMouseButton::Left, false);
			mPressPending = false;
			if (mLongPress)
			{
				mLongPress = false;
				mWindow->OnKey(EKey::Space, false);
			}
			mGesture = EGesture::None;
		}
		break;

	case EGesture::MultiFinger:
		if (active.count == 0)
		{
			// A two finger tap presses Escape
			if (!cancelled && !mGestureMoved && mGestureMaxTouches == 2 && event.timestamp - mGestureStartTime < cTapMaxDuration)
				[self tapKey: EKey::Escape];

			mGesture = EGesture::None;
		}
		else
		{
			// Number of fingers changed, the center of the fingers jumps so start panning from the new center
			mGestureStartPos = mLastPos = [self centroid: active];
		}
		break;

	case EGesture::None:
		break;
	}
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self endTouches: event cancelled: false];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	[self endTouches: event cancelled: true];
}

// Receive key presses from a hardware keyboard (this also works for the Mac keyboard in the simulator)
- (BOOL)canBecomeFirstResponder
{
	return YES;
}

- (void)didMoveToWindow
{
	[super didMoveToWindow];

	if (self.window != nil)
		[self becomeFirstResponder];
}

// Report key presses / releases to the window, returns the presses that were not handled
- (NSSet<UIPress *> *)handlePresses:(NSSet<UIPress *> *)presses pressed:(bool)pressed
{
	NSMutableSet<UIPress *> *unhandled = [NSMutableSet set];
	for (UIPress *press in presses)
	{
		EKey key = press.key != nil? sToKey(press.key.keyCode) : EKey::Unknown;
		if (key != EKey::Unknown)
			mWindow->OnKey(key, pressed);
		else
			[unhandled addObject: press];
	}
	return unhandled;
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
	NSSet<UIPress *> *unhandled = [self handlePresses: presses pressed: true];
	if (unhandled.count > 0)
		[super pressesBegan: unhandled withEvent: event];
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
	NSSet<UIPress *> *unhandled = [self handlePresses: presses pressed: false];
	if (unhandled.count > 0)
		[super pressesEnded: unhandled withEvent: event];
}

- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
	NSSet<UIPress *> *unhandled = [self handlePresses: presses pressed: false];
	if (unhandled.count > 0)
		[super pressesCancelled: unhandled withEvent: event];
}

@end

// View controller that hosts the metal view
@interface MetalViewController : UIViewController
@end

@implementation MetalViewController

- (void)loadView
{
	// Create a black root view
	UIView *root = [[UIView alloc] init];
	root.backgroundColor = UIColor.blackColor;
	self.view = root;
	[root release];

	// Place the metal view in the safe area so that it is not obscured by e.g. the camera cutout
	MTKView *metal_view = ApplicationWindowIOS::sGetInstance()->GetMetalView();
	metal_view.translatesAutoresizingMaskIntoConstraints = NO;
	[root addSubview: metal_view];
	UILayoutGuide *safe_area = root.safeAreaLayoutGuide;
	[NSLayoutConstraint activateConstraints: @[
		[metal_view.leadingAnchor constraintEqualToAnchor: safe_area.leadingAnchor],
		[metal_view.trailingAnchor constraintEqualToAnchor: safe_area.trailingAnchor],
		[metal_view.topAnchor constraintEqualToAnchor: safe_area.topAnchor],
		[metal_view.bottomAnchor constraintEqualToAnchor: safe_area.bottomAnchor]
	]];
}

- (BOOL)prefersStatusBarHidden
{
	return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden
{
	return YES;
}

- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures
{
	return UIRectEdgeAll;
}

@end

// Scene delegate that creates the window
@interface SceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation SceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions
{
	UIWindowScene *window_scene = (UIWindowScene *)scene;
	UIWindow *window = [[UIWindow alloc] initWithWindowScene: window_scene];
	MetalViewController *view_controller = [[MetalViewController alloc] init];
	window.rootViewController = view_controller;
	[view_controller release];
	self.window = window;
	[window makeKeyAndVisible];
	[window release];
}

@end

// Application delegate that configures the scene
@interface AppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation AppDelegate

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options
{
	UISceneConfiguration *config = [UISceneConfiguration configurationWithName: @"Default" sessionRole: connectingSceneSession.role];
	config.delegateClass = [SceneDelegate class];
	return config;
}

@end

ApplicationWindowIOS::~ApplicationWindowIOS()
{
	sInstance = nullptr;
}

void ApplicationWindowIOS::Initialize(const char *inTitle)
{
	JPH_ASSERT(sInstance == nullptr);
	sInstance = this;

	// Create metal view (it will be attached to a window when the application starts)
	MetalView *view = [[MetalView alloc] init: this];
	SetMetalView(view);	
}

void ApplicationWindowIOS::MainLoop(ApplicationWindow::RenderCallback inRenderCallback)
{
	mRenderCallback = inRenderCallback;

	// Note that this function never returns
	@autoreleasepool
	{
		UIApplicationMain(*_NSGetArgc(), *_NSGetArgv(), nil, NSStringFromClass([AppDelegate class]));
	}
}

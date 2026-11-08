// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowIOS.h>

#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>
#include <crt_externs.h>

ApplicationWindowIOS *ApplicationWindowIOS::sInstance = nullptr;

// Resolution scale used to render. Using less than the native scale of the screen makes the UI elements bigger which makes them easier to touch.
static constexpr CGFloat cMaxContentScale = 1.5;

// Touch gesture tuning parameters (in points)
static constexpr CGFloat cLookSensitivity = 0.5; // Mouse delta units per point that the finger moves
static constexpr CGFloat cJoystickDeadZone = 20.0;
static constexpr CGFloat cTapMaxMovement = 15.0;
static constexpr NSTimeInterval cTapMaxDuration = 0.3;
static constexpr NSTimeInterval cPressDelay = 0.1;
static constexpr NSTimeInterval cLongPressDuration = 0.5;

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
	bool				mVirtualKeys[4];	// W, A, S, D
	bool				mLongPress;			// If the Space key is held because of a long press
}

- (MetalView *)init:(ApplicationWindowIOS *)window
{
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame: CGRectMake(0, 0, window->GetWindowWidth(), window->GetWindowHeight()) device: device];
	[device release];

	mWindow = window;
	mGesture = EGesture::None;
	for (bool &k : mVirtualKeys)
		k = false;
	mLongPress = false;
	mInDraw = false;

	self.delegate = self;
	self.multipleTouchEnabled = YES;

	return self;
}

- (void)didMoveToWindow
{
	[super didMoveToWindow];

	// Determine the scale at which we render
	if (self.window != nil)
		self.contentScaleFactor = std::min(self.window.windowScene.screen.scale, cMaxContentScale);
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

- (void)setVirtualKey:(int)index key:(EKey)key pressed:(bool)pressed
{
	if (mVirtualKeys[index] != pressed)
	{
		mVirtualKeys[index] = pressed;
		mWindow->OnKey(key, pressed);
	}
}

- (void)releaseVirtualKeys
{
	[self setVirtualKey: 0 key: EKey::W pressed: false];
	[self setVirtualKey: 1 key: EKey::A pressed: false];
	[self setVirtualKey: 2 key: EKey::S pressed: false];
	[self setVirtualKey: 3 key: EKey::D pressed: false];
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
		// Multiple fingers: Cancel the mouse press and use the center of the fingers as virtual joystick
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
		mGestureStartPos = [self centroid: active];
		[self releaseVirtualKeys];
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
			// Virtual joystick
			CGPoint pos = [self centroid: active];
			CGFloat dx = pos.x - mGestureStartPos.x;
			CGFloat dy = pos.y - mGestureStartPos.y;
			if (hypot(dx, dy) > cTapMaxMovement)
				mGestureMoved = true;
			[self setVirtualKey: 0 key: EKey::W pressed: dy < -cJoystickDeadZone];
			[self setVirtualKey: 1 key: EKey::A pressed: dx < -cJoystickDeadZone];
			[self setVirtualKey: 2 key: EKey::S pressed: dy > cJoystickDeadZone];
			[self setVirtualKey: 3 key: EKey::D pressed: dx > cJoystickDeadZone];
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
			[self releaseVirtualKeys];

			// A two finger tap presses Escape
			if (!cancelled && !mGestureMoved && mGestureMaxTouches == 2 && event.timestamp - mGestureStartTime < cTapMaxDuration)
				[self tapKey: EKey::Escape];

			mGesture = EGesture::None;
		}
		else
		{
			// Number of fingers changed, recenter the joystick
			mGestureStartPos = [self centroid: active];
			[self releaseVirtualKeys];
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

	// Listen for hardware keyboards
	ListenForKeyboards();
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

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>
#include <Input/Keyboard.h>
#include <Jolt/Core/StaticArray.h>

struct android_app;
struct ANativeWindow;
struct AInputEvent;

/// Responsible for opening the main window on Android.
///
/// Touch input is translated into mouse and keyboard input:
/// - One finger: Moves the mouse cursor and holds the left mouse button (rotates the camera / interacts with the menu)
/// - One finger long press: Holds Space (drags the object under the cursor)
/// - Two finger drag: Moves the camera, the scene follows the fingers (reported through Mouse::GetPanDX / GetPanDY)
/// - Two finger tap: Presses Escape (toggles the menu / goes back one menu level)
/// - Back button / gesture: Presses Escape
///
/// The window size (GetWindowWidth / GetWindowHeight) is the size of the area that is not obscured by display cutouts and system bars,
/// the surface that we render to can be bigger (see GetSurfaceWidth / GetSurfaceHeight / GetSafeAreaX / GetSafeAreaY).
class ApplicationWindowAndroid : public ApplicationWindow
{
public:
	/// Destructor
	virtual							~ApplicationWindowAndroid() override;

	/// Set the android application, must be called before the window is initialized
	/// Android can create a new activity (with its own android_main thread) while the previous one is still running (e.g. after the app was updated).
	/// Only one application can run at a time, so android_main must call sAcquireApp before creating the application. This asks the running application
	/// to quit and waits until it has done so. Returns false if the activity was destroyed while waiting, in which case android_main should return.
	static bool						sAcquireApp(android_app *inApp);

	/// Must be called by android_main after the application has been destroyed. Finishes the activity if needed and waits until it is destroyed.
	static void						sReleaseApp(android_app *inApp);

	/// Get the command line, this is the program name followed by the 'args' string extra of the intent that started the activity
	/// (e.g. adb shell am start -n com.joltphysics.samples/android.app.NativeActivity -e args "HairTest")
	static String					sGetCommandLine();

	/// Initialize the window (waits until Android has created the native window)
	virtual void					Initialize(const char *inTitle) override;

	/// Enter the main loop and keep rendering frames until the application is terminated
	virtual void					MainLoop(ApplicationWindow::RenderCallback inRenderCallback) override;

	/// Access to the native window, can be null when the application is in the background
	ANativeWindow *					GetNativeWindow() const;

	/// Size of the surface that we render to (in pixels)
	int								GetSurfaceWidth() const					{ return mSurfaceWidth; }
	int								GetSurfaceHeight() const				{ return mSurfaceHeight; }

	/// Offset of the safe area in the surface (in pixels), the size of the safe area is the window size
	int								GetSafeAreaX() const					{ return mSafeAreaX; }
	int								GetSafeAreaY() const					{ return mSafeAreaY; }

	/// Subscribe to surface callbacks, the native window can be destroyed and recreated when the application goes to the background
	using SurfaceCallback = function<void(bool)>;							///< Called with false before the native window is destroyed, with true after it is recreated
	void							SetSurfaceCallback(SurfaceCallback inCallback) { mSurfaceCallback = inCallback; }

	/// Subscribe to mouse callbacks
	using MouseMovedCallback = function<void(int, int)>;					///< Absolute position in window coordinates (in pixels)
	using MouseDeltaCallback = function<void(int, int)>;					///< Relative movement
	using MouseButtonCallback = function<void(bool)>;						///< Left button pressed / released
	using PanCallback = function<void(float, float)>;						///< Two finger drag (in density independent pixels)
	void							SetMouseMovedCallback(MouseMovedCallback inCallback) { mMouseMovedCallback = inCallback; }
	void							SetMouseDeltaCallback(MouseDeltaCallback inCallback) { mMouseDeltaCallback = inCallback; }
	void							SetMouseButtonCallback(MouseButtonCallback inCallback) { mMouseButtonCallback = inCallback; }
	void							SetPanCallback(PanCallback inCallback)	{ mPanCallback = inCallback; }

	/// Subscribe to key callbacks
	using KeyCallback = function<void(EKey, bool)>;							///< Key pressed / released
	void							SetKeyCallback(KeyCallback inCallback)	{ mKeyCallback = inCallback; }

private:
	/// Position of a touch (in density independent pixels)
	struct Point
	{
		float						mX = 0.0f;
		float						mY = 0.0f;
	};

	using Points = StaticArray<Point, 16>;

	/// Type of touch gesture that is in progress
	enum class EGesture
	{
		None,
		OneFinger,
		MultiFinger
	};

	/// Callbacks from the native app glue
	static void						sHandleCommand(android_app *inApp, int32_t inCommand);
	static int32_t					sHandleInput(android_app *inApp, AInputEvent *inEvent);

	/// Process all pending events, blocks until an event arrives if inBlock is true
	void							ProcessEvents(bool inBlock);

	/// Handle a command from the native app glue
	void							HandleCommand(int32_t inCommand);

	/// Handle input events
	bool							HandleKeyEvent(const AInputEvent *inEvent);
	bool							HandleMotionEvent(const AInputEvent *inEvent);

	/// Determine the size of the surface and the safe area, calls the resize callback if it changed
	void							UpdateWindowSize(bool inNotify);

	/// Query the insets of the display cutouts and system bars (in pixels)
	void							GetSafeInsets(int &outLeft, int &outTop, int &outRight, int &outBottom) const;

	/// Report events to the listeners
	void							OnMouseMoved(int inX, int inY)			{ if (mMouseMovedCallback) mMouseMovedCallback(inX, inY); }
	void							OnMouseDelta(int inDX, int inDY)		{ if (mMouseDeltaCallback) mMouseDeltaCallback(inDX, inDY); }
	void							OnMouseButton(bool inPressed)			{ if (mMouseButtonCallback) mMouseButtonCallback(inPressed); }
	void							OnPan(float inDX, float inDY)			{ if (mPanCallback) mPanCallback(inDX, inDY); }
	void							OnKey(EKey inKey, bool inPressed)		{ if (mKeyCallback) mKeyCallback(inKey, inPressed); }

	/// Touch handling
	void							UpdateGesture(double inTime);
	void							TouchesBegan(const Points &inActive, double inTime);
	void							TouchesMoved(const Points &inActive);
	void							TouchesEnded(const Points &inActive, double inTime, bool inCancelled);
	void							CancelGesture();
	void							SendMousePosition(const Point &inPos);
	void							SendPress();
	void							TapKey(EKey inKey);
	static Point					sCentroid(const Points &inTouches);

	static android_app *			sApp;

	// State of the application
	bool							mResumed = false;
	bool							mQuitRequested = false;

	// Surface size
	float							mPixelsPerDP = 1.0f;					///< Conversion factor from density independent pixels to pixels of the native window
	int								mSurfaceWidth = 0;
	int								mSurfaceHeight = 0;
	int								mSafeAreaX = 0;
	int								mSafeAreaY = 0;

	// Callbacks
	ApplicationWindow::RenderCallback mRenderCallback;
	SurfaceCallback					mSurfaceCallback;
	MouseMovedCallback				mMouseMovedCallback;
	MouseDeltaCallback				mMouseDeltaCallback;
	MouseButtonCallback				mMouseButtonCallback;
	PanCallback						mPanCallback;
	KeyCallback						mKeyCallback;

	// Gesture state
	EGesture						mGesture = EGesture::None;
	double							mGestureStartTime = 0.0;
	uint							mGestureMaxTouches = 0;
	bool							mGestureMoved = false;
	bool							mPressPending = false;					///< Left button press has not been sent yet (we wait a bit to see if more fingers touch the screen)
	Point							mGestureStartPos;
	Point							mLastPos;
	float							mDeltaRemainderX = 0.0f;				///< Fractional part of the mouse delta that has not been sent yet
	float							mDeltaRemainderY = 0.0f;
	bool							mLongPress = false;						///< If the Space key is held because of a long press
};

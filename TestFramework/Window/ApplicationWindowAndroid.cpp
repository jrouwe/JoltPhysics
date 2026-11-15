// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowAndroid.h>
#include <Utils/Log.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/configuration.h>
#include <android/api-level.h>
#include <jni.h>
#include <time.h>
#include <mutex>
#include <atomic>
JPH_SUPPRESS_WARNINGS_STD_END

android_app *ApplicationWindowAndroid::sApp = nullptr;

// Ownership of the application, see sAcquireApp
static std::mutex sOwnerMutex;
static android_app *sOwner = nullptr;						///< The android app that is currently running the application
static std::atomic<bool> sReleaseRequested { false };		///< Set when another activity wants to run the application

// Process the events of an android app without running the application (so that the activity callbacks, which wait for the events to be processed, don't block)
static void sProcessEventsWithoutApplication(android_app *inApp, int inTimeoutMs)
{
	int timeout = inTimeoutMs;
	for (;;)
	{
		int events;
		android_poll_source *source = nullptr;
		int ident = ALooper_pollOnce(timeout, nullptr, &events, reinterpret_cast<void **>(&source));
		if (ident == ALOOPER_POLL_TIMEOUT || ident == ALOOPER_POLL_ERROR)
			break;
		if (ident >= 0 && source != nullptr)
			source->process(inApp, source);
		if (inApp->destroyRequested)
			break;
		timeout = 0;
	}
}

bool ApplicationWindowAndroid::sAcquireApp(android_app *inApp)
{
	for (;;)
	{
		{
			std::lock_guard lock(sOwnerMutex);
			if (sOwner == nullptr)
			{
				// We can run the application
				sOwner = inApp;
				sReleaseRequested = false;
				sApp = inApp;
				return true;
			}

			// Ask the running application to quit and wake up its thread in case it is waiting for events
			if (!sReleaseRequested)
				Trace("Another activity is still running, asking it to quit");
			sReleaseRequested = true;
			ALooper_wake(sOwner->looper);
		}

		// Keep processing our own events while we wait
		sProcessEventsWithoutApplication(inApp, 10);
		if (inApp->destroyRequested)
			return false;
	}
}

void ApplicationWindowAndroid::sReleaseApp(android_app *inApp)
{
	{
		std::lock_guard lock(sOwnerMutex);
		JPH_ASSERT(sOwner == inApp);
		sOwner = nullptr;
		sApp = nullptr;
	}

	// android_main is not allowed to return before the activity is destroyed, so finish the activity and wait for it to be destroyed
	if (!inApp->destroyRequested)
		ANativeActivity_finish(inApp->activity);
	while (!inApp->destroyRequested)
		sProcessEventsWithoutApplication(inApp, -1);
}

// Touch gesture tuning parameters (in density independent pixels)
static constexpr float cLookSensitivity = 0.5f; // Mouse delta units per density independent pixel that the finger moves
static constexpr float cTapMaxMovement = 15.0f;
static constexpr double cTapMaxDuration = 0.3;
static constexpr double cPressDelay = 0.1;
static constexpr double cLongPressDuration = 0.5;

// Get the current time in seconds (same time base as the motion events)
static double sGetTime()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return double(ts.tv_sec) + 1.0e-9 * double(ts.tv_nsec);
}

// Convert an Android key code to our own key code
static EKey sToKey(int32_t inKeyCode)
{
	if (inKeyCode >= AKEYCODE_A && inKeyCode <= AKEYCODE_Z)
		return EKey(int(EKey::A) + inKeyCode - AKEYCODE_A);
	if (inKeyCode >= AKEYCODE_0 && inKeyCode <= AKEYCODE_9)
		return EKey(int(EKey::Num0) + inKeyCode - AKEYCODE_0);

	switch (inKeyCode)
	{
	case AKEYCODE_SPACE:			return EKey::Space;
	case AKEYCODE_COMMA:			return EKey::Comma;
	case AKEYCODE_PERIOD:			return EKey::Period;
	case AKEYCODE_ESCAPE:			return EKey::Escape;
	case AKEYCODE_BACK:				return EKey::Escape; // The back button / gesture toggles the menu / goes back one menu level
	case AKEYCODE_SHIFT_LEFT:		return EKey::LShift;
	case AKEYCODE_SHIFT_RIGHT:		return EKey::RShift;
	case AKEYCODE_CTRL_LEFT:		return EKey::LControl;
	case AKEYCODE_CTRL_RIGHT:		return EKey::RControl;
	case AKEYCODE_ALT_LEFT:			return EKey::LAlt;
	case AKEYCODE_ALT_RIGHT:		return EKey::RAlt;
	case AKEYCODE_DPAD_LEFT:		return EKey::Left;
	case AKEYCODE_DPAD_RIGHT:		return EKey::Right;
	case AKEYCODE_DPAD_UP:			return EKey::Up;
	case AKEYCODE_DPAD_DOWN:		return EKey::Down;
	case AKEYCODE_ENTER:			return EKey::Return;
	case AKEYCODE_NUMPAD_ENTER:		return EKey::Return;
	default:						return EKey::Unknown;
	}
}

ApplicationWindowAndroid::~ApplicationWindowAndroid()
{
	if (sApp != nullptr)
	{
		// Unregister from the native app glue
		sApp->userData = nullptr;
		sApp->onAppCmd = nullptr;
		sApp->onInputEvent = nullptr;

		// We may have attached this thread to the Java VM when querying the insets
		sApp->activity->vm->DetachCurrentThread();
	}
}

void ApplicationWindowAndroid::Initialize(const char *inTitle)
{
	JPH_ASSERT(sApp != nullptr, "Call sAcquireApp first");

	// Register with the native app glue
	sApp->userData = this;
	sApp->onAppCmd = sHandleCommand;
	sApp->onInputEvent = sHandleInput;

	// Wait until the native window has been created
	while (sApp->window == nullptr)
	{
		ProcessEvents(true);
		if (sApp->destroyRequested)
			exit(0);
	}

	// Determine initial size
	UpdateWindowSize(false);
}

String ApplicationWindowAndroid::sGetCommandLine()
{
	String command_line = "Samples";

	JNIEnv *env = nullptr;
	if (sApp->activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
		return command_line;
	if (env->PushLocalFrame(16) != JNI_OK)
		return command_line;

	// Get the arguments: activity.getIntent().getStringExtra("args")
	jobject activity = sApp->activity->clazz;
	jobject intent = env->CallObjectMethod(activity, env->GetMethodID(env->GetObjectClass(activity), "getIntent", "()Landroid/content/Intent;"));
	jstring args = intent != nullptr? static_cast<jstring>(env->CallObjectMethod(intent, env->GetMethodID(env->GetObjectClass(intent), "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;"), env->NewStringUTF("args"))) : nullptr;
	if (args != nullptr && !env->ExceptionCheck())
	{
		const char *chars = env->GetStringUTFChars(args, nullptr);
		if (chars != nullptr)
		{
			command_line += " ";
			command_line += chars;
			env->ReleaseStringUTFChars(args, chars);
		}
	}

	// Ignore any java exceptions
	if (env->ExceptionCheck())
		env->ExceptionClear();

	env->PopLocalFrame(nullptr);

	return command_line;
}

ANativeWindow *ApplicationWindowAndroid::GetNativeWindow() const
{
	return sApp->window;
}

void ApplicationWindowAndroid::MainLoop(ApplicationWindow::RenderCallback inRenderCallback)
{
	mRenderCallback = inRenderCallback;

	while (!sApp->destroyRequested && !sReleaseRequested)
	{
		// Process events, block if we're not rendering
		ProcessEvents(mQuitRequested || !mResumed || sApp->window == nullptr);
		if (sApp->destroyRequested || sReleaseRequested)
			break;
		if (mQuitRequested || !mResumed || sApp->window == nullptr)
			continue;

		// Update touch gestures that depend on time
		UpdateGesture(sGetTime());

		// Render a frame
		if (!mRenderCallback())
		{
			// Application wants to quit, finish the activity and wait until we get the destroy request
			mQuitRequested = true;
			ANativeActivity_finish(sApp->activity);
		}
	}

	mRenderCallback = {};
}

void ApplicationWindowAndroid::ProcessEvents(bool inBlock)
{
	int timeout = inBlock? -1 : 0;
	for (;;)
	{
		int events;
		android_poll_source *source = nullptr;
		int ident = ALooper_pollOnce(timeout, nullptr, &events, reinterpret_cast<void **>(&source));
		if (ident == ALOOPER_POLL_TIMEOUT || ident == ALOOPER_POLL_ERROR)
			break;

		if (ident >= 0 && source != nullptr)
			source->process(sApp, source);

		if (sApp->destroyRequested)
			break;

		// Process remaining events without blocking
		timeout = 0;
	}
}

void ApplicationWindowAndroid::sHandleCommand(android_app *inApp, int32_t inCommand)
{
	ApplicationWindowAndroid *window = static_cast<ApplicationWindowAndroid *>(inApp->userData);
	if (window != nullptr)
		window->HandleCommand(inCommand);
}

int32_t ApplicationWindowAndroid::sHandleInput(android_app *inApp, AInputEvent *inEvent)
{
	ApplicationWindowAndroid *window = static_cast<ApplicationWindowAndroid *>(inApp->userData);
	if (window == nullptr)
		return 0;

	switch (AInputEvent_getType(inEvent))
	{
	case AINPUT_EVENT_TYPE_KEY:
		return window->HandleKeyEvent(inEvent)? 1 : 0;

	case AINPUT_EVENT_TYPE_MOTION:
		return window->HandleMotionEvent(inEvent)? 1 : 0;

	default:
		return 0;
	}
}

void ApplicationWindowAndroid::HandleCommand(int32_t inCommand)
{
	switch (inCommand)
	{
	case APP_CMD_INIT_WINDOW:
		// Native window was (re)created
		UpdateWindowSize(false);
		if (mSurfaceCallback)
			mSurfaceCallback(true);
		break;

	case APP_CMD_TERM_WINDOW:
		// Native window is about to be destroyed
		CancelGesture();
		if (mSurfaceCallback)
			mSurfaceCallback(false);
		break;

	case APP_CMD_WINDOW_RESIZED:
	case APP_CMD_CONFIG_CHANGED:
	case APP_CMD_CONTENT_RECT_CHANGED:
		UpdateWindowSize(true);
		break;

	case APP_CMD_RESUME:
		mResumed = true;
		break;

	case APP_CMD_PAUSE:
		mResumed = false;
		CancelGesture();
		break;

	case APP_CMD_LOST_FOCUS:
		CancelGesture();
		break;

	default:
		break;
	}
}

void ApplicationWindowAndroid::UpdateWindowSize(bool inNotify)
{
	ANativeWindow *window = sApp->window;
	if (window == nullptr)
		return;

	int native_width = ANativeWindow_getWidth(window);
	int native_height = ANativeWindow_getHeight(window);
	if (native_width <= 0 || native_height <= 0)
		return;

	// Determine the size of a density independent pixel
	int32_t density = AConfiguration_getDensity(sApp->config);
	if (density <= 0 || density == ACONFIGURATION_DENSITY_ANY || density == ACONFIGURATION_DENSITY_NONE)
		density = ACONFIGURATION_DENSITY_MEDIUM;
	mPixelsPerDP = float(density) / float(ACONFIGURATION_DENSITY_MEDIUM);

	// Get the area that is not obscured by display cutouts and system bars
	int left, top, right, bottom;
	GetSafeInsets(left, top, right, bottom);
	if (left + right >= native_width || top + bottom >= native_height)
		left = top = right = bottom = 0;
	int safe_width = native_width - left - right;
	int safe_height = native_height - top - bottom;

	// Check if something changed
	if (native_width == mSurfaceWidth && native_height == mSurfaceHeight
		&& left == mSafeAreaX && top == mSafeAreaY
		&& safe_width == mWindowWidth && safe_height == mWindowHeight)
		return;

	Trace("Window size: %dx%d, safe area: (%d, %d) - %dx%d", native_width, native_height, left, top, safe_width, safe_height);

	mSurfaceWidth = native_width;
	mSurfaceHeight = native_height;
	mSafeAreaX = left;
	mSafeAreaY = top;
	if (inNotify)
	{
		OnWindowResized(safe_width, safe_height);
	}
	else
	{
		mWindowWidth = safe_width;
		mWindowHeight = safe_height;
	}
}

void ApplicationWindowAndroid::GetSafeInsets(int &outLeft, int &outTop, int &outRight, int &outBottom) const
{
	outLeft = outTop = outRight = outBottom = 0;

	// Attach this thread to the Java VM (this is a no-op if we're already attached)
	JNIEnv *env = nullptr;
	if (sApp->activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
		return;
	if (env->PushLocalFrame(16) != JNI_OK)
		return;

	// Get the insets: activity.getWindow().getDecorView().getRootWindowInsets()
	jobject activity = sApp->activity->clazz;
	jobject window = env->CallObjectMethod(activity, env->GetMethodID(env->GetObjectClass(activity), "getWindow", "()Landroid/view/Window;"));
	jobject decor_view = window != nullptr? env->CallObjectMethod(window, env->GetMethodID(env->GetObjectClass(window), "getDecorView", "()Landroid/view/View;")) : nullptr;
	jobject insets = decor_view != nullptr? env->CallObjectMethod(decor_view, env->GetMethodID(env->GetObjectClass(decor_view), "getRootWindowInsets", "()Landroid/view/WindowInsets;")) : nullptr;
	if (insets != nullptr && !env->ExceptionCheck())
	{
		jclass insets_class = env->GetObjectClass(insets);
		if (android_get_device_api_level() >= 30)
		{
			// Get the insets of the system bars and display cutouts: insets.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout())
			jclass type_class = env->FindClass("android/view/WindowInsets$Type");
			jint types = env->CallStaticIntMethod(type_class, env->GetStaticMethodID(type_class, "systemBars", "()I"))
				| env->CallStaticIntMethod(type_class, env->GetStaticMethodID(type_class, "displayCutout", "()I"));
			jobject rect = env->CallObjectMethod(insets, env->GetMethodID(insets_class, "getInsets", "(I)Landroid/graphics/Insets;"), types);
			if (rect != nullptr)
			{
				jclass rect_class = env->GetObjectClass(rect);
				outLeft = env->GetIntField(rect, env->GetFieldID(rect_class, "left", "I"));
				outTop = env->GetIntField(rect, env->GetFieldID(rect_class, "top", "I"));
				outRight = env->GetIntField(rect, env->GetFieldID(rect_class, "right", "I"));
				outBottom = env->GetIntField(rect, env->GetFieldID(rect_class, "bottom", "I"));
			}
		}
		else
		{
			// Get the insets of the system bars
			outLeft = env->CallIntMethod(insets, env->GetMethodID(insets_class, "getSystemWindowInsetLeft", "()I"));
			outTop = env->CallIntMethod(insets, env->GetMethodID(insets_class, "getSystemWindowInsetTop", "()I"));
			outRight = env->CallIntMethod(insets, env->GetMethodID(insets_class, "getSystemWindowInsetRight", "()I"));
			outBottom = env->CallIntMethod(insets, env->GetMethodID(insets_class, "getSystemWindowInsetBottom", "()I"));
		}
	}

	// If anything failed, we assume there are no insets
	if (env->ExceptionCheck())
	{
		env->ExceptionClear();
		outLeft = outTop = outRight = outBottom = 0;
	}

	env->PopLocalFrame(nullptr);

	outLeft = max(0, outLeft);
	outTop = max(0, outTop);
	outRight = max(0, outRight);
	outBottom = max(0, outBottom);
}

bool ApplicationWindowAndroid::HandleKeyEvent(const AInputEvent *inEvent)
{
	// Let the system handle keys that we don't use (e.g. volume keys)
	EKey key = sToKey(AKeyEvent_getKeyCode(inEvent));
	if (key == EKey::Unknown)
		return false;

	switch (AKeyEvent_getAction(inEvent))
	{
	case AKEY_EVENT_ACTION_DOWN:
		// Ignore key repeats
		if (AKeyEvent_getRepeatCount(inEvent) == 0)
			OnKey(key, true);
		break;

	case AKEY_EVENT_ACTION_UP:
		OnKey(key, false);
		break;

	default:
		break;
	}

	return true;
}

bool ApplicationWindowAndroid::HandleMotionEvent(const AInputEvent *inEvent)
{
	int32_t action = AMotionEvent_getAction(inEvent);
	int32_t masked_action = action & AMOTION_EVENT_ACTION_MASK;
	size_t action_index = size_t((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
	double time = 1.0e-9 * double(AMotionEvent_getEventTime(inEvent));

	// A hardware mouse acts as a single finger (left button only)
	if ((AInputEvent_getSource(inEvent) & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)
	{
		if (masked_action == AMOTION_EVENT_ACTION_HOVER_MOVE)
		{
			// Update the cursor position when no button is pressed
			if (mGesture == EGesture::None)
				SendMousePosition({ AMotionEvent_getX(inEvent, 0) / mPixelsPerDP, AMotionEvent_getY(inEvent, 0) / mPixelsPerDP });
			return true;
		}

		if (masked_action == AMOTION_EVENT_ACTION_DOWN && (AMotionEvent_getButtonState(inEvent) & AMOTION_EVENT_BUTTON_PRIMARY) == 0)
			return true;
	}

	// Get all touches that are still on the screen (in density independent pixels)
	Points active;
	if (masked_action != AMOTION_EVENT_ACTION_CANCEL)
		for (size_t i = 0, n = AMotionEvent_getPointerCount(inEvent); i < n && active.size() < active.capacity(); ++i)
			if ((masked_action != AMOTION_EVENT_ACTION_UP && masked_action != AMOTION_EVENT_ACTION_POINTER_UP) || i != action_index)
				active.push_back({ AMotionEvent_getX(inEvent, i) / mPixelsPerDP, AMotionEvent_getY(inEvent, i) / mPixelsPerDP });

	switch (masked_action)
	{
	case AMOTION_EVENT_ACTION_DOWN:
	case AMOTION_EVENT_ACTION_POINTER_DOWN:
		TouchesBegan(active, time);
		break;

	case AMOTION_EVENT_ACTION_MOVE:
		TouchesMoved(active);
		break;

	case AMOTION_EVENT_ACTION_UP:
	case AMOTION_EVENT_ACTION_POINTER_UP:
		TouchesEnded(active, time, false);
		break;

	case AMOTION_EVENT_ACTION_CANCEL:
		TouchesEnded(active, time, true);
		break;

	default:
		break;
	}

	return true;
}

ApplicationWindowAndroid::Point ApplicationWindowAndroid::sCentroid(const Points &inTouches)
{
	Point c;
	for (const Point &p : inTouches)
	{
		c.mX += p.mX;
		c.mY += p.mY;
	}
	if (!inTouches.empty())
	{
		c.mX /= float(inTouches.size());
		c.mY /= float(inTouches.size());
	}
	return c;
}

void ApplicationWindowAndroid::SendMousePosition(const Point &inPos)
{
	// Convert from density independent pixels to pixels relative to the safe area
	OnMouseMoved(int(inPos.mX * mPixelsPerDP) - mSafeAreaX, int(inPos.mY * mPixelsPerDP) - mSafeAreaY);
}

void ApplicationWindowAndroid::SendPress()
{
	mPressPending = false;
	OnMouseButton(true);
}

void ApplicationWindowAndroid::TapKey(EKey inKey)
{
	OnKey(inKey, true);
	OnKey(inKey, false);
}

void ApplicationWindowAndroid::UpdateGesture(double inTime)
{
	// Send the delayed press if the finger stayed down long enough
	double gesture_duration = inTime - mGestureStartTime;
	if (mPressPending && gesture_duration > cPressDelay)
		SendPress();

	// A long press with one finger holds the Space key (drags the object under the cursor)
	if (mGesture == EGesture::OneFinger && !mGestureMoved && !mLongPress && gesture_duration > cLongPressDuration)
	{
		mLongPress = true;
		OnKey(EKey::Space, true);
	}
}

void ApplicationWindowAndroid::TouchesBegan(const Points &inActive, double inTime)
{
	if (inActive.empty())
		return;

	if (mGesture == EGesture::None)
	{
		// Start of a new gesture
		mGestureStartTime = inTime;
		mGestureMaxTouches = 0;
		mGestureMoved = false;
	}

	mGestureMaxTouches = max(mGestureMaxTouches, uint(inActive.size()));

	if (inActive.size() == 1 && mGesture == EGesture::None)
	{
		// One finger: Acts as a mouse with the left button pressed
		mGesture = EGesture::OneFinger;
		mLastPos = mGestureStartPos = inActive[0];
		mDeltaRemainderX = mDeltaRemainderY = 0.0f;
		SendMousePosition(mLastPos);
		mPressPending = true;
	}
	else if (inActive.size() > 1)
	{
		// Multiple fingers: Cancel the mouse press and use the center of the fingers to pan the camera
		if (mGesture == EGesture::OneFinger)
		{
			if (!mPressPending)
			{
				// The press was already sent, move the cursor off screen before releasing so that the release doesn't click the UI element under the first finger
				OnMouseMoved(-1, -1);
				OnMouseButton(false);
			}
			mPressPending = false;
			if (mLongPress)
			{
				mLongPress = false;
				OnKey(EKey::Space, false);
			}
		}
		mGesture = EGesture::MultiFinger;
		mGestureStartPos = mLastPos = sCentroid(inActive);
	}
}

void ApplicationWindowAndroid::TouchesMoved(const Points &inActive)
{
	if (inActive.empty())
		return;

	switch (mGesture)
	{
	case EGesture::OneFinger:
		{
			const Point &pos = inActive[0];
			if (sqrt(Square(pos.mX - mGestureStartPos.mX) + Square(pos.mY - mGestureStartPos.mY)) > cTapMaxMovement)
			{
				mGestureMoved = true;
				if (mPressPending)
					SendPress();
			}

			// Moving the finger drags the scene (so the camera rotates in the opposite direction)
			float dx = cLookSensitivity * (mLastPos.mX - pos.mX) + mDeltaRemainderX;
			float dy = cLookSensitivity * (mLastPos.mY - pos.mY) + mDeltaRemainderY;
			int idx = int(dx), idy = int(dy);
			mDeltaRemainderX = dx - float(idx);
			mDeltaRemainderY = dy - float(idy);
			OnMouseDelta(idx, idy);
			SendMousePosition(pos);
			mLastPos = pos;
		}
		break;

	case EGesture::MultiFinger:
		{
			// Pan the camera by the distance that the center of the fingers moved
			Point pos = sCentroid(inActive);
			if (sqrt(Square(pos.mX - mGestureStartPos.mX) + Square(pos.mY - mGestureStartPos.mY)) > cTapMaxMovement)
				mGestureMoved = true;
			OnPan(pos.mX - mLastPos.mX, pos.mY - mLastPos.mY);
			mLastPos = pos;
		}
		break;

	case EGesture::None:
		break;
	}
}

void ApplicationWindowAndroid::TouchesEnded(const Points &inActive, double inTime, bool inCancelled)
{
	switch (mGesture)
	{
	case EGesture::OneFinger:
		if (inActive.empty())
		{
			// Ensure that a short tap results in a click
			if (mPressPending && !inCancelled)
				SendPress();
			if (!mPressPending)
				OnMouseButton(false);
			mPressPending = false;
			if (mLongPress)
			{
				mLongPress = false;
				OnKey(EKey::Space, false);
			}
			mGesture = EGesture::None;
		}
		break;

	case EGesture::MultiFinger:
		if (inActive.empty())
		{
			// A two finger tap presses Escape
			if (!inCancelled && !mGestureMoved && mGestureMaxTouches == 2 && inTime - mGestureStartTime < cTapMaxDuration)
				TapKey(EKey::Escape);

			mGesture = EGesture::None;
		}
		else
		{
			// Number of fingers changed, the center of the fingers jumps so start panning from the new center
			mGestureStartPos = mLastPos = sCentroid(inActive);
		}
		break;

	case EGesture::None:
		break;
	}
}

void ApplicationWindowAndroid::CancelGesture()
{
	TouchesEnded(Points(), sGetTime(), true);
}

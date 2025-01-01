// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <Utils/Log.h>

#ifdef JPH_PLATFORM_MACOS
#include <Utils/MacOSImpl.h>
#endif

static Renderer *sRenderer = nullptr;

#ifdef JPH_PLATFORM_WINDOWS

#include <shellscalingapi.h>

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		if (sRenderer != nullptr)
			sRenderer->OnWindowResize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

#endif // JPH_PLATFORM_WINDOWS

void Renderer::Initialize()
{
#ifdef JPH_PLATFORM_WINDOWS
	// Prevent this window from auto scaling
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = TEXT("TestFrameworkClass");
	wcex.hIconSm = nullptr;
	if (!RegisterClassEx(&wcex))
		FatalError("Failed to register window class");

	// Create window
	RECT rc = { 0, 0, mWindowWidth, mWindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	mhWnd = CreateWindow(TEXT("TestFrameworkClass"), TEXT("TestFramework"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);
	if (!mhWnd)
		FatalError("Failed to create window");

	// Show window
	ShowWindow(mhWnd, SW_SHOW);
#elif defined(JPH_PLATFORM_LINUX)
	// Open connection to X server
	mDisplay = XOpenDisplay(nullptr);
	if (!mDisplay)
		FatalError("Failed to open X display");

	// Create a simple window
	int screen = DefaultScreen(mDisplay);
	mWindow = XCreateSimpleWindow(mDisplay, RootWindow(mDisplay, screen), 0, 0, mWindowWidth, mWindowHeight, 1, BlackPixel(mDisplay, screen), WhitePixel(mDisplay, screen));

	// Select input events
	XSelectInput(mDisplay, mWindow, ExposureMask | StructureNotifyMask | KeyPressMask);

	// Set window title
	XStoreName(mDisplay, mWindow, "TestFramework");

	// Register WM_DELETE_WINDOW to handle the close button
	mWmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(mDisplay, mWindow, &mWmDeleteWindow, 1);

	// Map the window (make it visible)
	XMapWindow(mDisplay, mWindow);

	// Flush the display to ensure commands are executed
	XFlush(mDisplay);
#elif defined(JPH_PLATFORM_MACOS)
	MacOSInit(mWindowWidth, mWindowHeight);
#else
	#error Unsupported platform
#endif

	// Store global renderer now that we're done initializing
	sRenderer = this;
}

bool Renderer::WindowUpdate()
{
#ifdef JPH_PLATFORM_WINDOWS
	// Main message loop
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			// Handle quit events
			return false;
		}
	}
#elif defined(JPH_PLATFORM_LINUX)
	while (XPending(mDisplay) > 0)
	{
		XEvent event;
		XNextEvent(mDisplay, &event);

		if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == mWmDeleteWindow)
		{
			// Handle quit events
			return false;
		}
		else if (event.type == ConfigureNotify)
		{
			// Handle window resize events
			XConfigureEvent xce = event.xconfigure;
			if (xce.width != mWindowWidth || xce.height != mWindowHeight)
			{
				mWindowWidth = xce.width;
				mWindowHeight = xce.height;
				OnWindowResize();
			}
		}
		else
			mEventListener(event);
	}
#elif defined(JPH_PLATFORM_MACOS)
	if (!MacOSUpdate())
		return false;
#else
	#error Unsupported platform
#endif

	// Application should continue
	return true;
}

void Renderer::OnWindowResize()
{
	JPH_ASSERT(!mInFrame);

#ifdef JPH_PLATFORM_WINDOWS
	// Get new window size
	RECT rc;
	GetClientRect(mhWnd, &rc);
	mWindowWidth = max<LONG>(rc.right - rc.left, 8);
	mWindowHeight = max<LONG>(rc.bottom - rc.top, 8);
#endif
}

static Mat44 sPerspectiveInfiniteReverseZ(float inFovY, float inAspect, float inNear, float inYSign)
{
	float height = 1.0f / Tan(0.5f * inFovY);
	float width = height / inAspect;

	return Mat44(Vec4(width, 0.0f, 0.0f, 0.0f), Vec4(0.0f, inYSign * height, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, -1.0f), Vec4(0.0f, 0.0f, inNear, 0.0f));
}

void Renderer::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	// Mark that we're in the frame
	JPH_ASSERT(!mInFrame);
	mInFrame = true;

	// Store state
	mCameraState = inCamera;

	// Light properties
	Vec3 light_pos = inWorldScale * Vec3(250, 250, 250);
	Vec3 light_tgt = Vec3::sZero();
	Vec3 light_up = Vec3(0, 1, 0);
	Vec3 light_fwd = (light_tgt - light_pos).Normalized();
	float light_fov = DegreesToRadians(20.0f);
	float light_near = 1.0f;

	// Camera properties
	Vec3 cam_pos = Vec3(inCamera.mPos - mBaseOffset);
	float camera_fovy = inCamera.mFOVY;
	float camera_aspect = static_cast<float>(GetWindowWidth()) / GetWindowHeight();
	float camera_fovx = 2.0f * ATan(camera_aspect * Tan(0.5f * camera_fovy));
	float camera_near = 0.01f * inWorldScale;

	// Calculate camera frustum
	mCameraFrustum = Frustum(cam_pos, inCamera.mForward, inCamera.mUp, camera_fovx, camera_fovy, camera_near);

	// Calculate light frustum
	mLightFrustum = Frustum(light_pos, light_fwd, light_up, light_fov, light_fov, light_near);

	// Camera projection and view
	mVSBuffer.mProjection = sPerspectiveInfiniteReverseZ(camera_fovy, camera_aspect, camera_near, mPerspectiveYSign);
	Vec3 tgt = cam_pos + inCamera.mForward;
	mVSBuffer.mView = Mat44::sLookAt(cam_pos, tgt, inCamera.mUp);

	// Light projection and view
	mVSBuffer.mLightProjection = sPerspectiveInfiniteReverseZ(light_fov, 1.0f, light_near, mPerspectiveYSign);
	mVSBuffer.mLightView = Mat44::sLookAt(light_pos, light_tgt, light_up);

	// Camera ortho projection and view
	mVSBufferOrtho.mProjection = Mat44(Vec4(2.0f / mWindowWidth, 0.0f, 0.0f, 0.0f), Vec4(0.0f, -mPerspectiveYSign * 2.0f / mWindowHeight, 0.0f, 0.0f), Vec4(0.0f, 0.0f, -1.0f, 0.0f), Vec4(-1.0f, mPerspectiveYSign * 1.0f, 0.0f, 1.0f));
	mVSBufferOrtho.mView = Mat44::sIdentity();

	// Light projection and view are unused in ortho mode
	mVSBufferOrtho.mLightView = Mat44::sIdentity();
	mVSBufferOrtho.mLightProjection = Mat44::sIdentity();

	// Set constants for pixel shader
	mPSBuffer.mCameraPos = Vec4(cam_pos, 0);
	mPSBuffer.mLightPos = Vec4(light_pos, 0);
}

void Renderer::EndFrame()
{
	// Mark that we're no longer in the frame
	JPH_ASSERT(mInFrame);
	mInFrame = false;
}

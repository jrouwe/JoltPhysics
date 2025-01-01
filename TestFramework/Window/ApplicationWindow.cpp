// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindow.h>
#include <Utils/Log.h>

#ifdef JPH_PLATFORM_MACOS
#include <Utils/MacOSImpl.h>
#endif

#ifdef JPH_PLATFORM_WINDOWS

ApplicationWindow *sWindow = nullptr;

#include <shellscalingapi.h>

// Called every time the application receives a message
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
		if (sWindow != nullptr)
			sWindow->OnWindowResize();
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

void ApplicationWindow::Initialize()
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

	// Store the window pointer for the message loop
	sWindow = this;
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
}

bool ApplicationWindow::WindowUpdate()
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

void ApplicationWindow::OnWindowResize()
{
#ifdef JPH_PLATFORM_WINDOWS
	// Get new window size
	RECT rc;
	GetClientRect(mhWnd, &rc);
	mWindowWidth = max<LONG>(rc.right - rc.left, 8);
	mWindowHeight = max<LONG>(rc.bottom - rc.top, 8);
#endif

	// Call callback
	mWindowResizeListener();
}

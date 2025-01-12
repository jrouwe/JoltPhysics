// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowWin.h>
#include <Utils/Log.h>
#include <shellscalingapi.h>

static ApplicationWindowWin *sWindow = nullptr;

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
		{
			// Get new window size
			RECT rc;
			GetClientRect(hWnd, &rc);
			int width = max<int>(rc.right - rc.left, 8);
			int height = max<int>(rc.bottom - rc.top, 8);
			sWindow->OnWindowResized(width, height);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ApplicationWindowWin::Initialize(const char *inTitle)
{
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
	mhWnd = CreateWindow(TEXT("TestFrameworkClass"), inTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);
	if (!mhWnd)
		FatalError("Failed to create window");

	// Show window
	ShowWindow(mhWnd, SW_SHOW);

	// Store the window pointer for the message loop
	sWindow = this;
}

void ApplicationWindowWin::MainLoop(RenderCallback inRenderCallback)
{
	for (;;)
	{
		// Main message loop
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				// Handle quit events
				return;
			}
		}

		// Call the render callback
		if (!inRenderCallback())
			return;
	}
}

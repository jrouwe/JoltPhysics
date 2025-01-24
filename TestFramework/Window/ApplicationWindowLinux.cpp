// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowLinux.h>
#include <Utils/Log.h>

ApplicationWindowLinux::~ApplicationWindowLinux()
{
	if (mDisplay)
	{
		XDestroyWindow(mDisplay, mWindow);
		XCloseDisplay(mDisplay);
	}
}

void ApplicationWindowLinux::Initialize(const char *inTitle)
{
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
	XStoreName(mDisplay, mWindow, inTitle);

	// Register WM_DELETE_WINDOW to handle the close button
	mWmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(mDisplay, mWindow, &mWmDeleteWindow, 1);

	// Map the window (make it visible)
	XMapWindow(mDisplay, mWindow);

	// Flush the display to ensure commands are executed
	XFlush(mDisplay);
}

void ApplicationWindowLinux::MainLoop(RenderCallback inRenderCallback)
{
	for (;;)
	{
		while (XPending(mDisplay) > 0)
		{
			XEvent event;
			XNextEvent(mDisplay, &event);

			if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == mWmDeleteWindow)
			{
				// Handle quit events
				return;
			}
			else if (event.type == ConfigureNotify)
			{
				// Handle window resize events
				XConfigureEvent xce = event.xconfigure;
				if (xce.width != mWindowWidth || xce.height != mWindowHeight)
					OnWindowResized(xce.width, xce.height);
			}
			else
				mEventListener(event);
		}

		// Call the render callback
		if (!inRenderCallback())
			return;
	}
}

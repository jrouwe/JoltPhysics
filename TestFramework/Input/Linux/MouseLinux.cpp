// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Linux/MouseLinux.h>
#include <Window/ApplicationWindowLinux.h>

MouseLinux::MouseLinux()
{
	Reset();
}

MouseLinux::~MouseLinux()
{
	Shutdown();
}

bool MouseLinux::Initialize(ApplicationWindow *inWindow)
{
	ApplicationWindowLinux *window = static_cast<ApplicationWindowLinux *>(inWindow);
	mDisplay = window->GetDisplay();
	mWindow = window->GetWindow();

	// Poll once and reset the deltas
	Poll();
	mDX = 0;
	mDY = 0;

	return true;
}

void MouseLinux::Shutdown()
{
	mWindow = 0;
	mDisplay = nullptr;
}

void MouseLinux::Reset()
{
	mX = 0;
	mY = 0;
	mDX = 0;
	mDY = 0;
	mLeftPressed = false;
	mRightPressed = false;
	mMiddlePressed = false;
}

void MouseLinux::Poll()
{
	Window root_return, child_return;
	int root_x, root_y, win_x, win_y;
	unsigned int mask;
	if (XQueryPointer(mDisplay, mWindow, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask))
	{
		mDX = win_x - mX;
		mDY = win_y - mY;
		mX = win_x;
		mY = win_y;
		mLeftPressed = mask & Button1Mask;
		mRightPressed = mask & Button3Mask;
		mMiddlePressed = mask & Button2Mask;
	}
	else
		Reset();
}

void MouseLinux::HideCursor()
{
}

void MouseLinux::ShowCursor()
{
}

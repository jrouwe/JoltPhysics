// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/MouseMacOS.h>
#include <Window/ApplicationWindowApple.h>

bool MouseMacOS::Initialize(ApplicationWindow *inWindow)
{
	mWindow = static_cast<ApplicationWindowApple *>(inWindow);

	// Install listeners for mouse callbacks
	mWindow->SetMouseMovedCallback([this](int inX, int inY) { mX = inX; mY = inY; });
	mWindow->SetMouseDeltaCallback([this](int inDX, int inDY) { mDeltaXAcc += inDX; mDeltaYAcc += inDY; });
	mWindow->SetMouseButtonCallback([this](EMouseButton inButton, bool inPressed) {
		int index = int(inButton);
		mButtonDown[index] = inPressed;
		mButtonPressedLatched[index] |= inPressed;
	});

	return true;
}

void MouseMacOS::Shutdown()
{
	if (mWindow != nullptr)
	{
		mWindow->SetMouseMovedCallback({});
		mWindow->SetMouseDeltaCallback({});
		mWindow->SetMouseButtonCallback({});
		mWindow = nullptr;
	}
}

void MouseMacOS::Poll()
{
	mDeltaX = mDeltaXAcc;
	mDeltaY = mDeltaYAcc;

	mDeltaXAcc = 0;
	mDeltaYAcc = 0;

	// A button counts as pressed if it is currently pressed or if it was pressed since the last poll
	for (int i = 0; i < cNumButtons; ++i)
	{
		mButtonPressed[i] = mButtonDown[i] || mButtonPressedLatched[i];
		mButtonPressedLatched[i] = false;
	}
}

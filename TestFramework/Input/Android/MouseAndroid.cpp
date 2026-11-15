// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Android/MouseAndroid.h>
#include <Window/ApplicationWindowAndroid.h>

bool MouseAndroid::Initialize(ApplicationWindow *inWindow)
{
	mWindow = static_cast<ApplicationWindowAndroid *>(inWindow);

	// Install listeners for mouse callbacks
	mWindow->SetMouseMovedCallback([this](int inX, int inY) { mX = inX; mY = inY; });
	mWindow->SetMouseDeltaCallback([this](int inDX, int inDY) { mDeltaXAcc += inDX; mDeltaYAcc += inDY; });
	mWindow->SetPanCallback([this](float inDX, float inDY) { mPanXAcc += inDX; mPanYAcc += inDY; });
	mWindow->SetMouseButtonCallback([this](bool inPressed) {
		mLeftDown = inPressed;
		mLeftPressedLatched |= inPressed;
	});

	return true;
}

void MouseAndroid::Shutdown()
{
	if (mWindow != nullptr)
	{
		mWindow->SetMouseMovedCallback({});
		mWindow->SetMouseDeltaCallback({});
		mWindow->SetPanCallback({});
		mWindow->SetMouseButtonCallback({});
		mWindow = nullptr;
	}
}

void MouseAndroid::Poll()
{
	mDeltaX = mDeltaXAcc;
	mDeltaY = mDeltaYAcc;

	mDeltaXAcc = 0;
	mDeltaYAcc = 0;

	mPanX = mPanXAcc;
	mPanY = mPanYAcc;
	mPanXAcc = 0.0f;
	mPanYAcc = 0.0f;

	// The button counts as pressed if it is currently pressed or if it was pressed since the last poll
	mLeftPressed = mLeftDown || mLeftPressedLatched;
	mLeftPressedLatched = false;
}

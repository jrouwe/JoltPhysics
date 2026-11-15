// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/KeyboardMacOS.h>
#include <Window/ApplicationWindowApple.h>

bool KeyboardMacOS::Initialize(ApplicationWindow *inWindow)
{
	mWindow = static_cast<ApplicationWindowApple *>(inWindow);

	// Install listener for key callbacks
	mWindow->SetKeyCallback([this](EKey inKey, bool inPressed) { OnKeyPressed(inKey, inPressed); });

	return true;
}

void KeyboardMacOS::Shutdown()
{
	if (mWindow != nullptr)
	{
		mWindow->SetKeyCallback({});
		mWindow = nullptr;
	}
}

void KeyboardMacOS::Poll()
{
	// A key counts as pressed if it is currently pressed or if it was pressed since the last poll
	for (int i = 0; i < (int)EKey::NumKeys; ++i)
	{
		mKeyPressed[i] = mKeyDown[i] || mKeyPressedLatched[i];
		mKeyPressedLatched[i] = false;
	}

	// Make the pending buffer the active buffer
	mKeyBuffer = mPendingKeyBuffer;
	mPendingKeyBuffer.clear();
}

EKey KeyboardMacOS::GetFirstKey()
{
	mCurrentKey = 0;
	return GetNextKey();
}

EKey KeyboardMacOS::GetNextKey()
{
	if (mCurrentKey < mKeyBuffer.size())
		return mKeyBuffer[mCurrentKey++];
	return EKey::Invalid;
}

void KeyboardMacOS::OnKeyPressed(EKey inKey, bool inPressed)
{
	if (inPressed && mPendingKeyBuffer.size() < mPendingKeyBuffer.capacity())
		mPendingKeyBuffer.push_back(inKey);

	mKeyDown[(int)inKey] = inPressed;
	mKeyPressedLatched[(int)inKey] |= inPressed;
}

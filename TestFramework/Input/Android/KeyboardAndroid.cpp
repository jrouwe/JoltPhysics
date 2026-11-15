// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Android/KeyboardAndroid.h>
#include <Window/ApplicationWindowAndroid.h>

bool KeyboardAndroid::Initialize(ApplicationWindow *inWindow)
{
	mWindow = static_cast<ApplicationWindowAndroid *>(inWindow);

	// Install listener for key callbacks
	mWindow->SetKeyCallback([this](EKey inKey, bool inPressed) { OnKeyPressed(inKey, inPressed); });

	return true;
}

void KeyboardAndroid::Shutdown()
{
	if (mWindow != nullptr)
	{
		mWindow->SetKeyCallback({});
		mWindow = nullptr;
	}
}

void KeyboardAndroid::Poll()
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

EKey KeyboardAndroid::GetFirstKey()
{
	mCurrentKey = 0;
	return GetNextKey();
}

EKey KeyboardAndroid::GetNextKey()
{
	if (mCurrentKey < mKeyBuffer.size())
		return mKeyBuffer[mCurrentKey++];
	return EKey::Invalid;
}

void KeyboardAndroid::OnKeyPressed(EKey inKey, bool inPressed)
{
	if (inPressed && mPendingKeyBuffer.size() < mPendingKeyBuffer.capacity())
		mPendingKeyBuffer.push_back(inKey);

	mKeyDown[(int)inKey] = inPressed;
	mKeyPressedLatched[(int)inKey] |= inPressed;
}

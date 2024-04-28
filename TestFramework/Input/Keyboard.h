// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// We're using DX8's DirectInput API
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

class Renderer;

/// Keyboard interface class which keeps track on the status of all keys and keeps track of the list of keys pressed.
class Keyboard
{
public:
	/// Constructor
									Keyboard();
									~Keyboard();

	/// Initialization / shutdown
	bool							Initialize(Renderer *inRenderer);
	void							Shutdown();

	/// Update the keyboard state
	void							Poll();

	/// Checks if a key is pressed or not, use one of the DIK_* constants
	bool							IsKeyPressed(int inKey) const		{ return mKeyPressed[inKey] != 0; }
	bool							IsKeyDoubleClicked(int inKey) const	{ return mKeyDoubleClicked[inKey] != 0; }

	/// Checks if a key is pressed and was not pressed the last time this function was called (state is stored in ioPrevState)
	bool							IsKeyPressedAndTriggered(int inKey, bool &ioPrevState) const
	{
		bool prev_state = ioPrevState;
		ioPrevState = IsKeyPressed(inKey);
		return ioPrevState && !prev_state;
	}

	/// Buffered keyboard input, returns 0 for none or one of the DIK_* constants
	int								GetFirstKey();
	int								GetNextKey();
	uint							GetVKValue();						///< Get VK_* constant value for last key returned by GetFirstKey or GetNextKey
	char							GetASCIIValue();					///< Get ASCII value for last key returned by GetFirstKey or GetNextKey

private:
	void							Reset();
	void							ResetKeyboard();

	enum
	{
		BUFFERSIZE					= 64,								///< Number of keys cached
		DCLICKTIME					= 300								///< Minimum time between key release and key down to make it a double click
	};

	// DirectInput part
	ComPtr<IDirectInput8>			mDI;
	ComPtr<IDirectInputDevice8>		mKeyboard;
	char							mKeyPressed[256];
	int								mKeyDoubleClicked[256];
	int								mTimeKeyLastReleased[256];
	DIDEVICEOBJECTDATA				mDOD[BUFFERSIZE];
	DWORD							mDODLength;
	DWORD							mCurrentPosition;

	// Windows User Interface part for translating DIK_* constants into VK_* constants and ASCII characters
	HKL								mKeyboardLayout;
	BYTE							mPreviousWUIState[256];
	BYTE							mCurrentWUIState[256];
	BYTE							mTrackedWUIState[256];
};

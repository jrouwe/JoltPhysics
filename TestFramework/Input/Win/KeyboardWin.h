// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Keyboard.h>

// We're using DX8's DirectInput API
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

/// Keyboard interface class which keeps track on the status of all keys and keeps track of the list of keys pressed.
class KeyboardWin : public Keyboard
{
public:
	/// Constructor
									KeyboardWin();
	virtual							~KeyboardWin() override;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the keyboard state
	virtual void					Poll() override;

	/// Checks if a key is pressed or not
	virtual bool					IsKeyPressed(EKey inKey) const override { return mKeyPressed[FromKey(inKey)] != 0; }

	/// Buffered keyboard input, returns EKey::Invalid for none
	virtual EKey					GetFirstKey() override;
	virtual EKey					GetNextKey() override;

private:
	void							Reset();
	void							ResetKeyboard();
	EKey							ToKey(int inKey) const;
	int								FromKey(EKey inKey) const;

	enum
	{
		BUFFERSIZE					= 64,								///< Number of keys cached
	};

	// DirectInput part
	ComPtr<IDirectInput8>			mDI;
	ComPtr<IDirectInputDevice8>		mKeyboard;
	char							mKeyPressed[256];
	DIDEVICEOBJECTDATA				mDOD[BUFFERSIZE];
	DWORD							mDODLength;
	DWORD							mCurrentPosition;
};

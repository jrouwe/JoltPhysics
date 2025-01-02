// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Keyboard.h>
#include <Jolt/Core/StaticArray.h>

class ApplicationWindowLinux;

/// Keyboard interface class which keeps track on the status of all keys and keeps track of the list of keys pressed.
class KeyboardLinux : public Keyboard
{
public:
	/// Destructor
	virtual							~KeyboardLinux() override;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the keyboard state
	virtual void					Poll() override;

	/// Checks if a key is pressed or not
	virtual bool					IsKeyPressed(EKey inKey) const override		{ return mKeysPressed[(int)inKey]; }

	/// Buffered keyboard input, returns EKey::Invalid for none
	virtual EKey					GetFirstKey() override;
	virtual EKey					GetNextKey() override;

private:
	void							HandleEvent(const XEvent &inEvent);
	EKey							ToKey(int inKey) const;

	ApplicationWindowLinux *		mWindow = nullptr;
	bool							mKeysPressed[(int)EKey::NumKeys] = { };
	StaticArray<EKey, 128>			mPendingKeyBuffer;
	StaticArray<EKey, 128>			mKeyBuffer;
	uint							mCurrentKey = 0;
};

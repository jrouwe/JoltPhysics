// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// We're using DX8's DirectInput API
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

class Renderer;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class Mouse
{
public:
	/// Constructor
									Mouse();
									~Mouse();

	/// Initialization / shutdown
	bool							Initialize(Renderer *inWindow);
	void							Shutdown();

	/// Update the mouse state
	void							Poll();

	int								GetX() const						{ return mMousePos.x; }
	int								GetY() const						{ return mMousePos.y; }
	int								GetDX() const						{ return mMouseState.lX; }
	int								GetDY() const						{ return mMouseState.lY; }

	bool							IsLeftPressed() const				{ return (mMouseState.rgbButtons[0] & 0x80) != 0; }
	bool							IsRightPressed() const				{ return (mMouseState.rgbButtons[1] & 0x80) != 0; }
	bool							IsMiddlePressed() const				{ return (mMouseState.rgbButtons[2] & 0x80) != 0; }

	bool							IsLeftDoubleClicked() const			{ return mLeftButtonDoubleClicked; }

	void							HideCursor();
	void							ShowCursor();

	void							SetExclusive(bool inExclusive = true);

private:
	void							DetectParsecRunning();
	void							Reset();
	void							ResetMouse();

	enum
	{
		BUFFERSIZE					= 64,								///< Number of keys cached
		DCLICKTIME					= 300								///< Minimum time between key release and key down to make it a double click
	};

	Renderer *						mRenderer;
	ComPtr<IDirectInput8>			mDI;
	ComPtr<IDirectInputDevice8>		mMouse;
	bool							mIsParsecRunning;					///< If the Parsec remote desktop solution is running, if so we can't trust the mouse movement information from DX and it will make the mouse too sensitive
	DIMOUSESTATE					mMouseState;
	bool							mMousePosInitialized = false;
	POINT							mMousePos;
	DIDEVICEOBJECTDATA				mDOD[BUFFERSIZE];
	DWORD							mDODLength;
	int								mTimeLeftButtonLastReleased;
	bool							mLeftButtonDoubleClicked;
};

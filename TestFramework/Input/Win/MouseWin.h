// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Mouse.h>

// We're using DX8's DirectInput API
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

class ApplicationWindowWin;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class MouseWin : public Mouse
{
public:
	/// Constructor
									MouseWin();
	virtual							~MouseWin() override;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the mouse state
	virtual void					Poll() override;

	virtual int						GetX() const override				{ return mMousePos.x; }
	virtual int						GetY() const override				{ return mMousePos.y; }
	virtual int						GetDX() const override				{ return mMouseState.lX; }
	virtual int						GetDY() const override				{ return mMouseState.lY; }

	virtual bool					IsLeftPressed() const override		{ return (mMouseState.rgbButtons[0] & 0x80) != 0; }
	virtual bool					IsRightPressed() const override		{ return (mMouseState.rgbButtons[1] & 0x80) != 0; }
	virtual bool					IsMiddlePressed() const override	{ return (mMouseState.rgbButtons[2] & 0x80) != 0; }

	virtual void					HideCursor() override;
	virtual void					ShowCursor() override;

private:
	void							DetectParsecRunning();
	void							Reset();
	void							ResetMouse();

	enum
	{
		BUFFERSIZE					= 64,								///< Number of keys cached
	};

	ApplicationWindowWin *			mWindow;
	ComPtr<IDirectInput8>			mDI;
	ComPtr<IDirectInputDevice8>		mMouse;
	bool							mIsParsecRunning;					///< If the Parsec remote desktop solution is running, if so we can't trust the mouse movement information from DX and it will make the mouse too sensitive
	DIMOUSESTATE					mMouseState;
	bool							mMousePosInitialized = false;
	POINT							mMousePos;
};

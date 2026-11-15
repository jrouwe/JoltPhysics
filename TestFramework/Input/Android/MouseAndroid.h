// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Mouse.h>

class ApplicationWindowAndroid;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
/// Receives input from the touch screen emulation in ApplicationWindowAndroid (a hardware mouse acts as a single finger).
class MouseAndroid : public Mouse
{
public:
	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the mouse state
	virtual void					Poll() override;

	virtual int						GetX() const override				{ return mX; }
	virtual int						GetY() const override				{ return mY; }
	virtual int						GetDX() const override				{ return mDeltaX; }
	virtual int						GetDY() const override				{ return mDeltaY; }

	virtual bool					IsLeftPressed() const override		{ return mLeftPressed; }
	virtual bool					IsRightPressed() const override		{ return false; }
	virtual bool					IsMiddlePressed() const override	{ return false; }

	virtual float					GetPanDX() const override			{ return mPanX; }
	virtual float					GetPanDY() const override			{ return mPanY; }

	virtual void					HideCursor() override				{ }
	virtual void					ShowCursor() override				{ }

private:
	ApplicationWindowAndroid *		mWindow = nullptr;

	int								mX = 0;
	int								mY = 0;
	int								mDeltaX = 0;
	int								mDeltaY = 0;
	int								mDeltaXAcc = 0;
	int								mDeltaYAcc = 0;

	float							mPanX = 0.0f;
	float							mPanY = 0.0f;
	float							mPanXAcc = 0.0f;
	float							mPanYAcc = 0.0f;

	bool							mLeftPressed = false;
	bool							mLeftDown = false;					///< Current state of the left button
	bool							mLeftPressedLatched = false;		///< If the left button was pressed since the last Poll (so that short taps are not lost)
};

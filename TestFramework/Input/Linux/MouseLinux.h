// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Mouse.h>

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class MouseLinux : public Mouse
{
public:
	/// Constructor
									MouseLinux();
	virtual							~MouseLinux() override;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the mouse state
	virtual void					Poll() override;

	virtual int						GetX() const override				{ return mX; }
	virtual int						GetY() const override				{ return mY; }
	virtual int						GetDX() const override				{ return mDX; }
	virtual int						GetDY() const override				{ return mDY; }

	virtual bool					IsLeftPressed() const override		{ return mLeftPressed; }
	virtual bool					IsRightPressed() const override		{ return mRightPressed; }
	virtual bool					IsMiddlePressed() const override	{ return mMiddlePressed; }

	virtual void					HideCursor() override;
	virtual void					ShowCursor() override;

private:
	void							Reset();

	Display *						mDisplay;
	Window							mWindow;

	int								mX;
	int								mY;
	int								mDX;
	int								mDY;
	bool							mLeftPressed;
	bool							mRightPressed;
	bool							mMiddlePressed;
};

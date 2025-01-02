// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Mouse.h>

class ApplicationWindowMacOS;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class MouseMacOS : public Mouse
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
	virtual bool					IsRightPressed() const override		{ return mRightPressed; }
	virtual bool					IsMiddlePressed() const override	{ return mMiddlePressed; }

	virtual void					HideCursor() override				{ }
	virtual void					ShowCursor() override				{ }

	/// Internal callbacks
	void							OnMouseMoved(int inX, int inY)		{ mX = inX; mY = inY; }
	void							OnMouseDelta(int inDX, int inDY)	{ mDeltaXAcc += inDX; mDeltaYAcc += inDY; }
	void							SetLeftPressed(bool inPressed)		{ mLeftPressed = inPressed; }
	void							SetRightPressed(bool inPressed)		{ mRightPressed = inPressed; }
	void							SetMiddlePressed(bool inPressed)	{ mMiddlePressed = inPressed; }

private:
	ApplicationWindowMacOS *		mWindow = nullptr;
	
	int								mX = 0;
	int								mY = 0;
	int								mDeltaX = 0;
	int								mDeltaY = 0;
	int								mDeltaXAcc = 0;
	int								mDeltaYAcc = 0;
	
	bool							mLeftPressed = false;
	bool							mRightPressed = false;
	bool							mMiddlePressed = false;
};

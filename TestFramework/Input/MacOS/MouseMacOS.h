// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Input/Mouse.h>

class ApplicationWindowApple;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class MouseMacOS : public Mouse
{
public:
	/// Mouse buttons
	enum class EMouseButton
	{
		Left = 0,
		Right = 1,
		Middle = 2
	};

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the mouse state
	virtual void					Poll() override;

	virtual int						GetX() const override				{ return mX; }
	virtual int						GetY() const override				{ return mY; }
	virtual int						GetDX() const override				{ return mDeltaX; }
	virtual int						GetDY() const override				{ return mDeltaY; }

	virtual bool					IsLeftPressed() const override		{ return mButtonPressed[int(EMouseButton::Left)]; }
	virtual bool					IsRightPressed() const override		{ return mButtonPressed[int(EMouseButton::Right)]; }
	virtual bool					IsMiddlePressed() const override	{ return mButtonPressed[int(EMouseButton::Middle)]; }

	virtual void					HideCursor() override				{ }
	virtual void					ShowCursor() override				{ }

private:
	static constexpr int			cNumButtons = 3;					///< Left, right, middle (indexed by EMouseButton)

	ApplicationWindowApple *		mWindow = nullptr;

	int								mX = 0;
	int								mY = 0;
	int								mDeltaX = 0;
	int								mDeltaY = 0;
	int								mDeltaXAcc = 0;
	int								mDeltaYAcc = 0;

	bool							mButtonPressed[cNumButtons] = { };
	bool							mButtonDown[cNumButtons] = { };				///< Current state of the buttons
	bool							mButtonPressedLatched[cNumButtons] = { };	///< If a button was pressed since the last Poll (so that short clicks are not lost)
};

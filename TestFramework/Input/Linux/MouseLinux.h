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
	virtual bool					Initialize(Renderer *inWindow) override;
	virtual void					Shutdown() override;

	/// Update the mouse state
	virtual void					Poll() override;

	virtual int						GetX() const override				{ return 0; }
	virtual int						GetY() const override				{ return 0; }
	virtual int						GetDX() const override				{ return 0; }
	virtual int						GetDY() const override				{ return 0; }

	virtual bool					IsLeftPressed() const override		{ return false; }
	virtual bool					IsRightPressed() const override		{ return false; }
	virtual bool					IsMiddlePressed() const override	{ return false; }

	virtual bool					IsLeftDoubleClicked() const override{ return false; }

	virtual void					HideCursor() override;
	virtual void					ShowCursor() override;

	virtual void					SetExclusive(bool inExclusive = true) override;
};

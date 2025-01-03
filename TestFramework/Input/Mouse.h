// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class ApplicationWindow;

/// Mouse interface class, keeps track of the mouse button state and of the absolute and relative movements of the mouse.
class Mouse
{
public:
	/// Constructor
									Mouse() = default;
	virtual							~Mouse() = default;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) = 0;
	virtual void					Shutdown() = 0;

	/// Update the mouse state
	virtual void					Poll() = 0;

	virtual int						GetX() const = 0;
	virtual int						GetY() const = 0;
	virtual int						GetDX() const = 0;
	virtual int						GetDY() const = 0;

	virtual bool					IsLeftPressed() const = 0;
	virtual bool					IsRightPressed() const = 0;
	virtual bool					IsMiddlePressed() const = 0;

	virtual void					HideCursor() = 0;
	virtual void					ShowCursor() = 0;
};

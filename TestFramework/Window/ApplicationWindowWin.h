// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

// Responsible for opening the main window
class ApplicationWindowWin : public ApplicationWindow
{
public:
	/// Initialize the window
	virtual void					Initialize(const char *inTitle) override;

	/// Access to the window handle
	HWND							GetWindowHandle() const				{ return mhWnd; }

	/// Enter the main loop and keep rendering frames until the window is closed
	virtual void					MainLoop(RenderCallback inRenderCallback) override;

protected:
	HWND							mhWnd;
};

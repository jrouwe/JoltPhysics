// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindowApple.h>

/// Responsible for opening the main window on iOS.
///
/// Touch input is translated into mouse and keyboard input:
/// - One finger: Moves the mouse cursor and holds the left mouse button (rotates the camera / interacts with the menu)
/// - One finger long press: Holds Space (drags the object under the cursor)
/// - Two finger drag: Acts as a virtual joystick that presses W, A, S, D (moves the camera)
/// - Two finger tap: Presses Escape (toggles the menu / goes back one menu level)
class ApplicationWindowIOS : public ApplicationWindowApple
{
public:
	/// Destructor
	virtual							~ApplicationWindowIOS() override;

	/// Initialize the window
	virtual void					Initialize(const char *inTitle) override;

	/// Enter the main loop and keep rendering frames until the application is terminated
	virtual void					MainLoop(ApplicationWindow::RenderCallback inRenderCallback) override;

	/// Access to the singleton instance (needed by the UIKit delegates)
	static ApplicationWindowIOS *	sGetInstance()							{ return sInstance; }

protected:
	static ApplicationWindowIOS *	sInstance;
};

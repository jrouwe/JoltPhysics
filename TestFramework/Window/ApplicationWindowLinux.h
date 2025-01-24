// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

// Responsible for opening the main window
class ApplicationWindowLinux : public ApplicationWindow
{
public:
	/// Destructor
									~ApplicationWindowLinux();

	/// Initialize the window
	virtual void					Initialize(const char *inTitle) override;

	/// Access to the window handle
	Display *						GetDisplay() const					{ return mDisplay; }
	Window							GetWindow() const					{ return mWindow; }

	/// Event listener for the keyboard handler
	using EventListener = std::function<void(const XEvent &)>;
	void							SetEventListener(const EventListener &inListener) { mEventListener = inListener; }

	/// Enter the main loop and keep rendering frames until the window is closed
	void							MainLoop(RenderCallback inRenderCallback) override;

protected:
	Display *						mDisplay;
	Window							mWindow;
	Atom							mWmDeleteWindow;
	EventListener					mEventListener;
};

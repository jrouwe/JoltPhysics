// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

// Responsible for opening the main window
class ApplicationWindow
{
public:
	/// Destructor
	virtual							~ApplicationWindow() = default;

	/// Initialize the window
	virtual void					Initialize(const char *inTitle) = 0;

	/// Get window size
	int								GetWindowWidth()					{ return mWindowWidth; }
	int								GetWindowHeight()					{ return mWindowHeight; }

	/// Set callback when the window resizes
	using WindowResizeListener = std::function<void()>;
	void							SetWindowResizeListener(const WindowResizeListener &inListener) { mWindowResizeListener = inListener; }

	/// Enter the main loop and keep rendering frames until the window is closed
	using RenderCallback = std::function<bool()>;
	virtual void					MainLoop(RenderCallback inRenderCallback) = 0;
	
	/// Function that will trigger the callback
	void							OnWindowResized(int inWidth, int inHeight) { mWindowWidth = inWidth; mWindowHeight = inHeight; if (mWindowResizeListener) { mWindowResizeListener(); } }

protected:
	int								mWindowWidth = 1920;
	int								mWindowHeight = 1080;
	WindowResizeListener			mWindowResizeListener;
};

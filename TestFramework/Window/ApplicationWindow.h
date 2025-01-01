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
	virtual void					Initialize();

	/// Get window size
	int								GetWindowWidth()					{ return mWindowWidth; }
	int								GetWindowHeight()					{ return mWindowHeight; }

	/// Set callback when the window resizes
	using WindowResizeListener = std::function<void()>;
	void							SetWindowResizeListener(const WindowResizeListener &inListener) { mWindowResizeListener = inListener; }

#ifdef JPH_PLATFORM_WINDOWS
	/// Access to the window handle
	HWND							GetWindowHandle() const				{ return mhWnd; }
#elif defined(JPH_PLATFORM_LINUX)
	/// Access to the window handle
	Display *						GetDisplay() const					{ return mDisplay; }
	Window							GetWindow() const					{ return mWindow; }
	using EventListener = std::function<void(const XEvent &)>;
	void							SetEventListener(const EventListener &inListener) { mEventListener = inListener; }
#endif // JPH_PLATFORM_WINDOWS

	/// Enter the main loop and keep rendering frames until the window is closed
	using RenderCallback = std::function<bool()>;
	void							MainLoop(RenderCallback inRenderCallback);

	/// Callback when the window resizes
	void							OnWindowResize();

protected:
	int								mWindowWidth = 1920;
	int								mWindowHeight = 1080;
	WindowResizeListener			mWindowResizeListener;

#ifdef JPH_PLATFORM_WINDOWS
	HWND							mhWnd;
#elif defined(JPH_PLATFORM_LINUX)
	Display *						mDisplay;
	Window							mWindow;
	Atom							mWmDeleteWindow;
	EventListener					mEventListener;
#endif
};

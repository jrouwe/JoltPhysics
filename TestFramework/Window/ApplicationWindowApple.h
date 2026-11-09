// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>
#include <Input/MacOS/MouseMacOS.h>
#include <Input/Keyboard.h>

#ifdef __OBJC__
@class MTKView;
@class CAMetalLayer;
#else
typedef void MTKView;
typedef void CAMetalLayer;
#endif

/// Base class for the macOS and iOS windows, manages the metal view
class ApplicationWindowApple : public ApplicationWindow
{
public:
	/// Destructor
	virtual							~ApplicationWindowApple() override;

	/// Access to the metal objects
	MTKView *						GetMetalView() const					{ return mMetalView; }
	CAMetalLayer *					GetMetalLayer() const;

	/// Call the render callback
	bool							RenderCallback()						{ return mRenderCallback && mRenderCallback(); }

	/// Subscribe to mouse callbacks
	using MouseMovedCallback = function<void(int, int)>;					///< Absolute position in window coordinates (in pixels)
	using MouseDeltaCallback = function<void(int, int)>;					///< Relative movement (also works when the mouse is at the edge of the screen)
	using MouseButtonCallback = function<void(MouseMacOS::EMouseButton, bool)>;			///< Button pressed / released
	using PanCallback = function<void(float, float)>;						///< Two finger drag on a touch screen (in points)
	void							SetMouseMovedCallback(MouseMovedCallback inCallback) { mMouseMovedCallback = inCallback; }
	void							SetMouseDeltaCallback(MouseDeltaCallback inCallback) { mMouseDeltaCallback = inCallback; }
	void							SetMouseButtonCallback(MouseButtonCallback inCallback) { mMouseButtonCallback = inCallback; }
	void							SetPanCallback(PanCallback inCallback)	{ mPanCallback = inCallback; }

	/// Subscribe to key callbacks
	using KeyCallback = function<void(EKey, bool)>;							///< Key pressed / released
	void							SetKeyCallback(KeyCallback inCallback)	{ mKeyCallback = inCallback; }

	/// Internal callbacks
	void							OnMouseMoved(int inX, int inY)			{ if (mMouseMovedCallback) mMouseMovedCallback(inX, inY); }
	void							OnMouseDelta(int inDX, int inDY)		{ if (mMouseDeltaCallback) mMouseDeltaCallback(inDX, inDY); }
	void							OnMouseButton(MouseMacOS::EMouseButton inButton, bool inPressed) { if (mMouseButtonCallback) mMouseButtonCallback(inButton, inPressed); }
	void							OnPan(float inDX, float inDY)			{ if (mPanCallback) mPanCallback(inDX, inDY); }
	void							OnKey(EKey inKey, bool inPressed)		{ if (mKeyCallback) mKeyCallback(inKey, inPressed); }

protected:
	/// Take ownership of the metal view and configure it
	void							SetMetalView(MTKView *inView);

	/// Start listening for (hardware) keyboards, key presses will be reported through OnKey
	void							ListenForKeyboards();

	MTKView *						mMetalView = nullptr;
	ApplicationWindow::RenderCallback mRenderCallback;
	MouseMovedCallback				mMouseMovedCallback;
	MouseDeltaCallback				mMouseDeltaCallback;
	MouseButtonCallback				mMouseButtonCallback;
	PanCallback						mPanCallback;
	KeyCallback						mKeyCallback;

private:
	void *							mKeyboardConnectObserver = nullptr;		///< Observer for GCKeyboardDidConnectNotification
};

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

#ifdef __OBJC__
@class CAMetalLayer;
#else
typedef void CAMetalLayer;
#endif

// Responsible for opening the main window
class ApplicationWindowMacOS : public ApplicationWindow
{
public:
	/// Initialize the window
	virtual void					Initialize() override;

	/// Access to the metal layer
	CAMetalLayer *					GetMetalLayer() const					{ return mMetalLayer; }

	/// Enter the main loop and keep rendering frames until the window is closed
	virtual void					MainLoop(RenderCallback inRenderCallback) override;
	
	/// Call the render callback
	bool							RenderCallback()						{ return mRenderCallback(); }
	
	/// Subscribe to mouse move callbacks that supply window coordinates
	using MouseMovedCallback = function<void(int, int)>;
	void							SetMouseMovedCallback(MouseMovedCallback inCallback) { mMouseMovedCallback = inCallback; }
	void							OnMouseMoved(int inX, int inY)			{ mMouseMovedCallback(inX, inY); }

protected:
	CAMetalLayer *					mMetalLayer = nullptr;
	ApplicationWindow::RenderCallback mRenderCallback;
	MouseMovedCallback				mMouseMovedCallback;
};
	

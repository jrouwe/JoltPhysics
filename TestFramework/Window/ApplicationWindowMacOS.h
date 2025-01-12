// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

#ifdef __OBJC__
@class MTKView;
@class CAMetalLayer;
#else
typedef void MTKView;
typedef void CAMetalLayer;
#endif

// Responsible for opening the main window
class ApplicationWindowMacOS : public ApplicationWindow
{
public:
	/// Destructor
	virtual							~ApplicationWindowMacOS() override;

	/// Initialize the window
	virtual void					Initialize(const char *inTitle) override;

	/// Access to the metal objects
	MTKView *						GetMetalView() const					{ return mMetalView; }
	CAMetalLayer *					GetMetalLayer() const;

	/// Enter the main loop and keep rendering frames until the window is closed
	virtual void					MainLoop(RenderCallback inRenderCallback) override;
	
	/// Call the render callback
	bool							RenderCallback()						{ return mRenderCallback && mRenderCallback(); }

	/// Subscribe to mouse move callbacks that supply window coordinates
	using MouseMovedCallback = function<void(int, int)>;
	void							SetMouseMovedCallback(MouseMovedCallback inCallback) { mMouseMovedCallback = inCallback; }
	void							OnMouseMoved(int inX, int inY)			{ mMouseMovedCallback(inX, inY); }

protected:
	MTKView *						mMetalView = nullptr;
	ApplicationWindow::RenderCallback mRenderCallback;
	MouseMovedCallback				mMouseMovedCallback;
};
	

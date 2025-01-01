// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

#ifdef __OBJC__
@class CAMetalLayer;
@class NSWindow;
#else
typedef void CAMetalLayer;
typedef void NSWindow;
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

protected:
	NSWindow *						mWindow = nullptr;
	CAMetalLayer *					mMetalLayer = nullptr;
};
	

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindow.h>

struct CAMetalLayer;

// Responsible for opening the main window
class ApplicationWindowMacOS : public ApplicationWindow
{
public:
	/// Initialize the window
	virtual void					Initialize() override;

	/// Access to the metal layer
	CAMetalLayer *					GetMetalLayer() const;

	/// Enter the main loop and keep rendering frames until the window is closed
	virtual void					MainLoop(RenderCallback inRenderCallback) override;

protected:
	CAMetalLayer *					mMetalLayer = nullptr;
};

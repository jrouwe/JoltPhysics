// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Window/ApplicationWindowApple.h>

// Responsible for opening the main window
class ApplicationWindowMacOS : public ApplicationWindowApple
{
public:
	/// Destructor
	virtual							~ApplicationWindowMacOS() override;

	/// Initialize the window
	virtual void					Initialize(const char *inTitle) override;

	/// Enter the main loop and keep rendering frames until the window is closed
	virtual void					MainLoop(ApplicationWindow::RenderCallback inRenderCallback) override;

private:
	void *							mKeyboardConnectObserver = nullptr;		///< Observer for GCKeyboardDidConnectNotification
	void *							mMouseConnectObserver = nullptr;		///< Observer for GCMouseDidConnectNotification
	void *							mKeyDownMonitor = nullptr;				///< Monitor that swallows key down events
};


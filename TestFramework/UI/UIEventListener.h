// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

enum EUIEvent
{
	EVENT_BUTTON_DOWN,
	EVENT_MENU_DEACTIVATED,
};

class UIElement;

/// Callback class for handling events from UI elements
class UIEventListener
{
public:
	/// Destructor
	virtual				~UIEventListener() = default;

	/// Handle an UI event, function should return true if event was handled
	virtual bool		HandleUIEvent(EUIEvent inEvent, UIElement *inSender) = 0;
};

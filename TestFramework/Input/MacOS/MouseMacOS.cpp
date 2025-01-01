// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/MouseMacOS.h>

MouseMacOS::MouseMacOS()
{
}

MouseMacOS::~MouseMacOS()
{
	Shutdown();
}

bool MouseMacOS::Initialize(ApplicationWindow *inWindow)
{
	return true;
}

void MouseMacOS::Shutdown()
{
}

void MouseMacOS::Poll()
{
}

void MouseMacOS::HideCursor()
{
}

void MouseMacOS::ShowCursor()
{
}

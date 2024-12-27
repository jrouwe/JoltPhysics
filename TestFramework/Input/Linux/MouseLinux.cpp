// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Linux/MouseLinux.h>
#include <Renderer/Renderer.h>

MouseLinux::MouseLinux()
{
}

MouseLinux::~MouseLinux()
{
	Shutdown();
}

bool MouseLinux::Initialize(Renderer *inRenderer)
{
	return true;
}

void MouseLinux::Shutdown()
{
}

void MouseLinux::Poll()
{
}

void MouseLinux::HideCursor()
{
}

void MouseLinux::ShowCursor()
{
}

void MouseLinux::SetExclusive(bool inExclusive)
{
}

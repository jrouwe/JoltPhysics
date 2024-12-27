// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Linux/KeyboardLinux.h>
#include <Renderer/Renderer.h>

KeyboardLinux::KeyboardLinux()
{
}

KeyboardLinux::~KeyboardLinux()
{
	Shutdown();
}

bool KeyboardLinux::Initialize(Renderer *inRenderer)
{
	return true;
}

void KeyboardLinux::Shutdown()
{
}

void KeyboardLinux::Poll()
{
}

EKey KeyboardLinux::GetFirstKey()
{
	return GetNextKey();
}

EKey KeyboardLinux::GetNextKey()
{
	return EKey::Invalid;
}

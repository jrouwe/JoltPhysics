// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/KeyboardMacOS.h>

KeyboardMacOS::~KeyboardMacOS()
{
	Shutdown();
}

bool KeyboardMacOS::Initialize(Renderer *inRenderer)
{
	return true;
}

void KeyboardMacOS::Shutdown()
{
}

void KeyboardMacOS::Poll()
{
}

EKey KeyboardMacOS::GetFirstKey()
{
	return GetNextKey();
}

EKey KeyboardMacOS::GetNextKey()
{
	return EKey::Invalid;
}

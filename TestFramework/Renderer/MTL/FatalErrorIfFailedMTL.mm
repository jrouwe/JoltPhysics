// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/MTL/FatalErrorIfFailedMTL.h>
#include <Utils/Log.h>

void FatalErrorIfFailed(NSError *inResult)
{
	if (inResult != nullptr)
		FatalError("Metal error returned: %s", [[inResult localizedDescription] cStringUsingEncoding: NSUTF8StringEncoding]);
}

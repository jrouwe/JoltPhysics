// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/FatalErrorIfFailedVK.h>
#include <Utils/Log.h>

void FatalErrorIfFailed(VkResult inVkResult)
{
	if (inVkResult != VK_SUCCESS)
		FatalError("Vulkan error returned: %d", inVkResult);
}

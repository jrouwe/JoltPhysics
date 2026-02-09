// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/StringTools.h>

#ifdef JPH_USE_VK

JPH_SUPPRESS_WARNINGS_STD_BEGIN
JPH_CLANG_SUPPRESS_WARNING("-Wc++98-compat-pedantic")

#include <vulkan/vulkan.h>

JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

inline bool VKFailed(VkResult inResult)
{
	if (inResult == VK_SUCCESS)
		return false;

	Trace("Vulkan call failed with error code: %d", (int)inResult);
	JPH_ASSERT(false);
	return true;
}

template <class Result>
inline bool VKFailed(VkResult inResult, Result &outResult)
{
	if (inResult == VK_SUCCESS)
		return false;

	String error = StringFormat("Vulkan call failed with error code: %d", (int)inResult);
	outResult.SetError(error);
	return true;
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK

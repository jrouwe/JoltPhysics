// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#if defined(JPH_USE_VK) && defined(JPH_ASAN_ENABLED)

// Suppress ASAN leak detection for the Vulkan driver
extern "C" const char *__lsan_default_suppressions()
{
	return "leak:libvulkan";
}

#endif

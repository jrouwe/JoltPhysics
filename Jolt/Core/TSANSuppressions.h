// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_TSAN_ENABLED

JPH_SUPPRESS_WARNING_PUSH
JPH_CLANG_SUPPRESS_WARNING("-Wreserved-identifier")

// Suppress TSAN data race reports that are known to be benign. Include this file in exactly one cpp file of an executable.
extern "C" const char *__tsan_default_suppressions();
extern "C" const char *__tsan_default_suppressions()
{
	return
		// BodyManager::TryGetBody reads mBodies.size() without taking mBodiesMutex while BodyManager::AddBody can push_back to it.
		// mBodies is reserved at Init time so it never reallocates and it only grows. A thread that obtained the body ID has already observed
		// a size that is larger than the index of the body, so a concurrent add cannot change the outcome. JPH_TSAN_NO_SANITIZE cannot be
		// used here because the race is reported in Array::size(), which doesn't get inlined into a function with a different sanitizer setting.
		"race:JPH::BodyManager::TryGetBody\n"
	#ifdef JPH_USE_VK
		// Data races inside the Vulkan driver (e.g. Mesa llvmpipe)
		"race:libvulkan\n"
	#endif
		;
}

JPH_SUPPRESS_WARNING_POP

#endif

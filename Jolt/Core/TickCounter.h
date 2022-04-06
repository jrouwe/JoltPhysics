// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Include for __rdtsc
#if defined(JPH_PLATFORM_WINDOWS)
	#include <intrin.h> 
#elif defined(JPH_CPU_X64) && defined(JPH_COMPILER_GCC)
	#include <x86intrin.h>
#endif

JPH_NAMESPACE_BEGIN

#ifdef JPH_PLATFORM_WINDOWS_UWP

/// Functionality to get the processors cycle counter
uint64 GetProcessorTickCount(); // Not inline to avoid having to include Windows.h

#else

/// Functionality to get the processors cycle counter
JPH_INLINE uint64 GetProcessorTickCount()
{
#if defined(JPH_PLATFORM_BLUE)
	return JPH_PLATFORM_BLUE_GET_TICKS();
#elif defined(JPH_CPU_X64)
	return __rdtsc();
#elif defined(JPH_CPU_ARM64)
	uint64 val;
    asm volatile("mrs %0, cntvct_el0" : "=r" (val));
	return val;
#else
	#error Undefined
#endif
}

#endif // JPH_PLATFORM_WINDOWS_UWP

/// Get the amount of ticks per second, note that this number will never be fully accurate as the amound of ticks per second may vary with CPU load, so this number is only to be used to give an indication of time for profiling purposes
uint64 GetProcessorTicksPerSecond();

JPH_NAMESPACE_END

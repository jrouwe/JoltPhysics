// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_PLATFORM_WINDOWS
	#include <intrin.h> // for __rdtsc
#endif

namespace JPH {

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

/// Get the amount of ticks per second, note that this number will never be fully accurate as the amound of ticks per second may vary with CPU load, so this number is only to be used to give an indication of time for profiling purposes
uint64 GetProcessorTicksPerSecond();

} // JPH

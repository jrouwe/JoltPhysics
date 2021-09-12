// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Determine platform
#if defined(JPH_PLATFORM_BLUE)
	// Correct define already defined, this overrides everything else
#elif defined(_WIN32) || defined(_WIN64)
	#define JPH_PLATFORM_WINDOWS
#elif defined(__ANDROID__) // Android is linux too, so that's why we check it first
	#define JPH_PLATFORM_ANDROID
#elif defined(__linux__)
	#define JPH_PLATFORM_LINUX
#endif

// Turn off warnings
#if defined(__clang__)
	#pragma clang diagnostic ignored "-Wc++98-compat"
	#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
	#pragma clang diagnostic ignored "-Wfloat-equal"
	#pragma clang diagnostic ignored "-Wnewline-eof"
	#pragma clang diagnostic ignored "-Wsign-conversion"
	#pragma clang diagnostic ignored "-Wold-style-cast"
	#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
	#pragma clang diagnostic ignored "-Wnested-anon-types"
	#pragma clang diagnostic ignored "-Wglobal-constructors"
	#pragma clang diagnostic ignored "-Wexit-time-destructors"
	#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
	#pragma clang diagnostic ignored "-Wlanguage-extension-token"
	#pragma clang diagnostic ignored "-Wunused-parameter"
	#pragma clang diagnostic ignored "-Wformat-nonliteral"
	#pragma clang diagnostic ignored "-Wcovered-switch-default"
	#pragma clang diagnostic ignored "-Wcast-align"
	#pragma clang diagnostic ignored "-Winvalid-offsetof"
	#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
	#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
	#pragma clang diagnostic ignored "-Wctad-maybe-unsupported"
	#ifndef JPH_PLATFORM_ANDROID
		#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
	#endif
#elif defined(_MSC_VER)
	#pragma warning (disable : 4514) // 'X' : unreferenced inline function has been removed
	#pragma warning (disable : 4710) // 'X' : function not inlined
	#pragma warning (disable : 4711) // function 'X' selected for automatic inline expansion
	#pragma warning (disable : 4820) // 'X': 'Y' bytes padding added after data member 'Z'
	#pragma warning (disable : 4100) // 'X' : unreferenced formal parameter
	#pragma warning (disable : 4626) // 'X' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
	#pragma warning (disable : 5027) // 'X' : move assignment operator was implicitly defined as deleted because a base class move assignment operator is inaccessible or deleted
	#pragma warning (disable : 4365) // 'argument' : conversion from 'X' to 'Y', signed / unsigned mismatch
	#pragma warning (disable : 4324) // 'X' : structure was padded due to alignment specifier
	#pragma warning (disable : 4625) // 'X' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
	#pragma warning (disable : 5026) // 'X': move constructor was implicitly defined as deleted because a base class move constructor is inaccessible or deleted
	#pragma warning (disable : 4623) // 'X' : default constructor was implicitly defined as deleted
	#pragma warning (disable : 4201) // nonstandard extension used: nameless struct/union
	#pragma warning (disable : 4371) // 'X': layout of class may have changed from a previous version of the compiler due to better packing of member 'Y'
	#pragma warning (disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
	#pragma warning (disable : 4583) // 'X': destructor is not implicitly called
	#pragma warning (disable : 4582) // 'X': constructor is not implicitly called
	#pragma warning (disable : 5219) // implicit conversion from 'X' to 'Y', possible loss of data 
#endif

// Detect CPU architecture
#if defined(__x86_64__) || defined(_M_X64)
	// X86 CPU architecture
	#define JPH_CPU_X64
	#define JPH_USE_SSE

	// Detect enabled instruction sets
	#if (defined(__F16C__) || defined(__AVX2__)) && !defined(JPH_USE_F16C)
		#define JPH_USE_F16C
	#endif
	#if (defined(__LZCNT__) || defined(__AVX2__)) && !defined(JPH_USE_LZCNT)
		#define JPH_USE_LZCNT
	#endif
	#if defined(__AVX__) && !defined(JPH_USE_AVX)
		#define JPH_USE_AVX
	#endif
	#if defined(__AVX2__) && !defined(JPH_USE_AVX2)
		#define JPH_USE_AVX2
	#endif
	#if defined(__clang__)
		#if defined(__FMA__) && !defined(JPH_USE_FMADD)
			#define JPH_USE_FMADD
		#endif
	#elif defined(_MSC_VER)
		#if defined(__AVX2__) && !defined(JPH_USE_FMADD) // AVX2 also enables fused multiply add
			#define JPH_USE_FMADD
		#endif
	#else
		#error Undefined compiler
	#endif
#elif defined(__aarch64__) || defined(_M_ARM64)
	// ARM64 CPU architecture
	#define JPH_CPU_ARM64
	#define JPH_USE_NEON
#else
	#error Unsupported CPU architecture
#endif

// OS-specific includes
#if defined(JPH_PLATFORM_WINDOWS)
	#define JPH_BREAKPOINT		__debugbreak()
#elif defined(JPH_PLATFORM_BLUE) 
	// Configuration for a popular game console.
	// This file is not distributed because it would violate an NDA. 
	// Creating one should only be a couple of minutes of work if you have the documentation for the platform 
	// (you only need to define JPH_BREAKPOINT, JPH_PLATFORM_BLUE_GET_TICKS and JPH_PLATFORM_BLUE_GET_TICK_FREQUENCY and include the right header).
	#include <Core/PlatformBlue.h> 
#elif defined(JPH_PLATFORM_LINUX) || defined(JPH_PLATFORM_ANDROID)
	#include <float.h>
	#include <limits.h>
	#include <string.h>

	#if defined(JPH_CPU_X64)
		#define JPH_BREAKPOINT		__asm volatile ("int $0x3")
	#elif defined(JPH_CPU_ARM64)
		#define JPH_BREAKPOINT		__builtin_trap()
	#endif
#else
	#error Unknown platform
#endif

// Crashes the application
#define JPH_CRASH				do { int *ptr = nullptr; *ptr = 0; } while (false)

// Standard C++ includes
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <sstream>
#include <functional>
#if defined(JPH_USE_SSE)
	#include <immintrin.h>
#elif defined(JPH_USE_NEON)
	#include <arm_neon.h>
#endif

namespace JPH {

using namespace std;

// Standard types
using uint = unsigned int;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// Assert sizes of types
static_assert(sizeof(uint) >= 4, "Invalid size of uint");
static_assert(sizeof(uint8) == 1, "Invalid size of uint8");
static_assert(sizeof(uint16) == 2, "Invalid size of uint16");
static_assert(sizeof(uint32) == 4, "Invalid size of uint32");
static_assert(sizeof(uint64) == 8, "Invalid size of uint64");
static_assert(sizeof(void *) == 8, "Invalid size of pointer");

// Define inline macro
#if defined(__clang__)
	#define JPH_INLINE __inline__ __attribute__((always_inline))
#elif defined(_MSC_VER)
	#define JPH_INLINE __forceinline
#else
	#error Undefined
#endif

// Cache line size (used for aligning to cache line)
#ifndef JPH_CACHE_LINE_SIZE
	#define JPH_CACHE_LINE_SIZE 64
#endif

// Define macro to get current function name
#if defined(__clang__)
	#define JPH_FUNCTION_NAME	__PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define JPH_FUNCTION_NAME	__FUNCTION__
#else
	#error Undefined
#endif

// Stack allocation
#define JPH_STACK_ALLOC(n)		alloca(n)
	
// Shorthand for #ifdef _DEBUG / #endif
#ifdef _DEBUG
	#define JPH_IF_DEBUG(...)	__VA_ARGS__
#else
	#define JPH_IF_DEBUG(...)
#endif

// Macro to indicate that a parameter / variable is unused
#define JPH_UNUSED(x)			(void)x

// Macro to enable floating point precise mode and to disable fused multiply add instructions
#if defined(__clang__)
	// In clang it appears you cannot turn off -ffast-math and -ffp-contract=fast for a code block
	// There is #pragma clang fp contract (off) but that doesn't seem to work under clang 9 & 10 when -ffast-math is specified on the commandline (you override it to turn it on, but not off)
	// There is #pragma float_control(precise, on) but that doesn't work under clang 9.
	// So for now we compile clang without -ffast-math so the macros are empty
	#define JPH_PRECISE_MATH_ON
	#define JPH_PRECISE_MATH_OFF
#elif defined(_MSC_VER)
	// Unfortunately there is no way to push the state of fp_contract, so we have to assume it was turned on before JPH_PRECISE_MATH_ON
	#define JPH_PRECISE_MATH_ON						\
		__pragma(float_control(precise, on, push))	\
		__pragma(fp_contract(off))
	#define JPH_PRECISE_MATH_OFF					\
		__pragma(fp_contract(on))					\
		__pragma(float_control(pop))
#else
	#error Undefined
#endif

} // JPH

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Determine platform
#if defined(JPH_PLATFORM_BLUE)
	// Correct define already defined, this overrides everything else
#elif defined(_WIN32) || defined(_WIN64)
	#include <winapifamily.h>
	#if WINAPI_FAMILY == WINAPI_FAMILY_APP
		#define JPH_PLATFORM_WINDOWS_UWP // Building for Universal Windows Platform
	#endif
	#define JPH_PLATFORM_WINDOWS
#elif defined(__ANDROID__) // Android is linux too, so that's why we check it first
	#define JPH_PLATFORM_ANDROID
#elif defined(__linux__)
	#define JPH_PLATFORM_LINUX
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_IPHONE) && !TARGET_OS_IPHONE
        #define JPH_PLATFORM_MACOS
    #else
        #define JPH_PLATFORM_IOS
    #endif
#elif defined(__EMSCRIPTEN__)
	#define JPH_PLATFORM_WASM
#endif

// Platform helper macros
#ifdef JPH_PLATFORM_ANDROID
	#define JPH_IF_NOT_ANDROID(x)
#else
	#define JPH_IF_NOT_ANDROID(x) x
#endif

// Determine compiler
#if defined(__clang__)
	#define JPH_COMPILER_CLANG
#elif defined(__MINGW64__) || defined (__MINGW32__)
	#define JPH_COMPILER_GCC
	#define JPH_COMPILER_MINGW
#elif defined(__GNUC__)
	#define JPH_COMPILER_GCC
#elif defined(_MSC_VER)
	#define JPH_COMPILER_MSVC
#endif

// Detect CPU architecture
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	// X86 CPU architecture
	#define JPH_CPU_X86
	#if defined(__x86_64__) || defined(_M_X64)
		#define JPH_CPU_ADDRESS_BITS 64
	#else
		#define JPH_CPU_ADDRESS_BITS 32
	#endif
	#define JPH_USE_SSE

	// Detect enabled instruction sets
	#if (defined(__F16C__) || defined(__AVX2__)) && !defined(JPH_USE_F16C)
		#define JPH_USE_F16C
	#endif
	#if (defined(__LZCNT__) || defined(__AVX2__)) && !defined(JPH_USE_LZCNT)
		#define JPH_USE_LZCNT
	#endif
	#if (defined(__BMI__) || defined(__AVX2__)) && !defined(JPH_USE_TZCNT)
		#define JPH_USE_TZCNT
	#endif
	#if (defined(__SSE4_1__) || defined(__AVX__)) && !defined(JPH_USE_SSE4_1)
		#define JPH_USE_SSE4_1
	#endif
	#if (defined(__SSE4_2__) || defined(__AVX__)) && !defined(JPH_USE_SSE4_2)
		#define JPH_USE_SSE4_2
	#endif
	#if defined(__AVX__) && !defined(JPH_USE_AVX)
		#define JPH_USE_AVX
	#endif
	#if defined(__AVX2__) && !defined(JPH_USE_AVX2)
		#define JPH_USE_AVX2
	#endif
	#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && !defined(JPH_USE_AVX512)
		#define JPH_USE_AVX512
	#endif
	#ifndef JPH_CROSS_PLATFORM_DETERMINISTIC // FMA is not compatible with cross platform determinism
		#if defined(JPH_COMPILER_CLANG) || defined(JPH_COMPILER_GCC)
			#if defined(__FMA__) && !defined(JPH_USE_FMADD)
				#define JPH_USE_FMADD
			#endif
		#elif defined(JPH_COMPILER_MSVC)
			#if defined(__AVX2__) && !defined(JPH_USE_FMADD) // AVX2 also enables fused multiply add
				#define JPH_USE_FMADD
			#endif
		#else
			#error Undefined compiler
		#endif
	#endif
#elif defined(__aarch64__) || defined(_M_ARM64)
	// ARM64 CPU architecture
	#define JPH_CPU_ARM64
	#define JPH_USE_NEON
	#define JPH_CPU_ADDRESS_BITS 64
#elif defined(JPH_PLATFORM_WASM)
	// WebAssembly CPU architecture
	#define JPH_CPU_WASM
	#define JPH_CPU_ADDRESS_BITS 32
	#define JPH_DISABLE_CUSTOM_ALLOCATOR
#else
	#error Unsupported CPU architecture
#endif

// Pragmas to store / restore the warning state and to disable individual warnings
#ifdef JPH_COMPILER_CLANG
#define JPH_PRAGMA(x)					_Pragma(#x)
#define JPH_SUPPRESS_WARNING_PUSH		JPH_PRAGMA(clang diagnostic push)
#define JPH_SUPPRESS_WARNING_POP		JPH_PRAGMA(clang diagnostic pop)
#define JPH_CLANG_SUPPRESS_WARNING(w)	JPH_PRAGMA(clang diagnostic ignored w)
	#if __clang_major__ >= 16
		#define JPH_CLANG16_SUPPRESS_WARNING(w)	JPH_PRAGMA(clang diagnostic ignored w)
	#else
		#define JPH_CLANG16_SUPPRESS_WARNING(w)
	#endif
#else
#define JPH_CLANG_SUPPRESS_WARNING(w)
#define JPH_CLANG16_SUPPRESS_WARNING(w)
#endif
#ifdef JPH_COMPILER_GCC
#define JPH_PRAGMA(x)					_Pragma(#x)
#define JPH_SUPPRESS_WARNING_PUSH		JPH_PRAGMA(GCC diagnostic push)
#define JPH_SUPPRESS_WARNING_POP		JPH_PRAGMA(GCC diagnostic pop)
#define JPH_GCC_SUPPRESS_WARNING(w)		JPH_PRAGMA(GCC diagnostic ignored w)
#else
#define JPH_GCC_SUPPRESS_WARNING(w)
#endif
#ifdef JPH_COMPILER_MSVC
#define JPH_PRAGMA(x)					__pragma(x)
#define JPH_SUPPRESS_WARNING_PUSH		JPH_PRAGMA(warning (push))
#define JPH_SUPPRESS_WARNING_POP		JPH_PRAGMA(warning (pop))
#define JPH_MSVC_SUPPRESS_WARNING(w)	JPH_PRAGMA(warning (disable : w))
#else
#define JPH_MSVC_SUPPRESS_WARNING(w)
#endif

// Disable common warnings triggered by Jolt when compiling with -Wall
#define JPH_SUPPRESS_WARNINGS																	\
	JPH_CLANG_SUPPRESS_WARNING("-Wc++98-compat")												\
	JPH_CLANG_SUPPRESS_WARNING("-Wc++98-compat-pedantic")										\
	JPH_CLANG_SUPPRESS_WARNING("-Wfloat-equal")													\
	JPH_CLANG_SUPPRESS_WARNING("-Wsign-conversion")												\
	JPH_CLANG_SUPPRESS_WARNING("-Wold-style-cast")												\
	JPH_CLANG_SUPPRESS_WARNING("-Wgnu-anonymous-struct")										\
	JPH_CLANG_SUPPRESS_WARNING("-Wnested-anon-types")											\
	JPH_CLANG_SUPPRESS_WARNING("-Wglobal-constructors")											\
	JPH_CLANG_SUPPRESS_WARNING("-Wexit-time-destructors")										\
	JPH_CLANG_SUPPRESS_WARNING("-Wnonportable-system-include-path")								\
	JPH_CLANG_SUPPRESS_WARNING("-Wlanguage-extension-token")									\
	JPH_CLANG_SUPPRESS_WARNING("-Wunused-parameter")											\
	JPH_CLANG_SUPPRESS_WARNING("-Wformat-nonliteral")											\
	JPH_CLANG_SUPPRESS_WARNING("-Wcovered-switch-default")										\
	JPH_CLANG_SUPPRESS_WARNING("-Wcast-align")													\
	JPH_CLANG_SUPPRESS_WARNING("-Winvalid-offsetof")											\
	JPH_CLANG_SUPPRESS_WARNING("-Wgnu-zero-variadic-macro-arguments")							\
	JPH_CLANG_SUPPRESS_WARNING("-Wdocumentation-unknown-command")								\
	JPH_CLANG_SUPPRESS_WARNING("-Wctad-maybe-unsupported")										\
	JPH_CLANG16_SUPPRESS_WARNING("-Wunqualified-std-cast-call")									\
	JPH_IF_NOT_ANDROID(JPH_CLANG_SUPPRESS_WARNING("-Wimplicit-int-float-conversion"))			\
																								\
	JPH_GCC_SUPPRESS_WARNING("-Wcomment")														\
	JPH_GCC_SUPPRESS_WARNING("-Winvalid-offsetof")												\
	JPH_GCC_SUPPRESS_WARNING("-Wclass-memaccess")												\
																								\
	JPH_MSVC_SUPPRESS_WARNING(4514) /* 'X' : unreferenced inline function has been removed */	\
	JPH_MSVC_SUPPRESS_WARNING(4710) /* 'X' : function not inlined */							\
	JPH_MSVC_SUPPRESS_WARNING(4711) /* function 'X' selected for automatic inline expansion */	\
	JPH_MSVC_SUPPRESS_WARNING(4820) /* 'X': 'Y' bytes padding added after data member 'Z' */	\
	JPH_MSVC_SUPPRESS_WARNING(4100) /* 'X' : unreferenced formal parameter */					\
	JPH_MSVC_SUPPRESS_WARNING(4626) /* 'X' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted */ \
	JPH_MSVC_SUPPRESS_WARNING(5027) /* 'X' : move assignment operator was implicitly defined as deleted because a base class move assignment operator is inaccessible or deleted */ \
	JPH_MSVC_SUPPRESS_WARNING(4365) /* 'argument' : conversion from 'X' to 'Y', signed / unsigned mismatch */ \
	JPH_MSVC_SUPPRESS_WARNING(4324) /* 'X' : structure was padded due to alignment specifier */ \
	JPH_MSVC_SUPPRESS_WARNING(4625) /* 'X' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted */ \
	JPH_MSVC_SUPPRESS_WARNING(5026) /* 'X': move constructor was implicitly defined as deleted because a base class move constructor is inaccessible or deleted */ \
	JPH_MSVC_SUPPRESS_WARNING(4623) /* 'X' : default constructor was implicitly defined as deleted */ \
	JPH_MSVC_SUPPRESS_WARNING(4201) /* nonstandard extension used: nameless struct/union */		\
	JPH_MSVC_SUPPRESS_WARNING(4371) /* 'X': layout of class may have changed from a previous version of the compiler due to better packing of member 'Y' */ \
	JPH_MSVC_SUPPRESS_WARNING(5045) /* Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified */ \
	JPH_MSVC_SUPPRESS_WARNING(4583) /* 'X': destructor is not implicitly called */				\
	JPH_MSVC_SUPPRESS_WARNING(4582) /* 'X': constructor is not implicitly called */				\
	JPH_MSVC_SUPPRESS_WARNING(5219) /* implicit conversion from 'X' to 'Y', possible loss of data  */ \
	JPH_MSVC_SUPPRESS_WARNING(4826) /* Conversion from 'X *' to 'JPH::uint64' is sign-extended. This may cause unexpected runtime behavior. (32-bit) */

// OS-specific includes
#if defined(JPH_PLATFORM_WINDOWS)
	#define JPH_BREAKPOINT		__debugbreak()
#elif defined(JPH_PLATFORM_BLUE) 
	// Configuration for a popular game console.
	// This file is not distributed because it would violate an NDA. 
	// Creating one should only be a couple of minutes of work if you have the documentation for the platform 
	// (you only need to define JPH_BREAKPOINT, JPH_PLATFORM_BLUE_GET_TICKS and JPH_PLATFORM_BLUE_GET_TICK_FREQUENCY and include the right header).
	#include <Jolt/Core/PlatformBlue.h> 
#elif defined(JPH_PLATFORM_LINUX) || defined(JPH_PLATFORM_ANDROID) || defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
	#if defined(JPH_CPU_X86)
		#define JPH_BREAKPOINT		__asm volatile ("int $0x3")
	#elif defined(JPH_CPU_ARM64)
		#define JPH_BREAKPOINT		__builtin_trap()
	#endif
#elif defined(JPH_PLATFORM_WASM)
	#define JPH_BREAKPOINT		do { } while (false) // Not supported
#else
	#error Unknown platform
#endif

// Crashes the application
#define JPH_CRASH				do { int *ptr = nullptr; *ptr = 0; } while (false)

// Begin the JPH namespace
#define JPH_NAMESPACE_BEGIN																		\
	JPH_SUPPRESS_WARNING_PUSH																	\
	JPH_SUPPRESS_WARNINGS																		\
	namespace JPH {

// End the JPH namespace
#define JPH_NAMESPACE_END																		\
	}																							\
	JPH_SUPPRESS_WARNING_POP

// Suppress warnings generated by the standard template library
#define JPH_SUPPRESS_WARNINGS_STD_BEGIN															\
	JPH_SUPPRESS_WARNING_PUSH																	\
	JPH_MSVC_SUPPRESS_WARNING(4710)																\
	JPH_MSVC_SUPPRESS_WARNING(4711)																\
	JPH_MSVC_SUPPRESS_WARNING(4820)																\
	JPH_MSVC_SUPPRESS_WARNING(4514)																\
																								\
	JPH_GCC_SUPPRESS_WARNING("-Wstringop-overflow=")

#define JPH_SUPPRESS_WARNINGS_STD_END															\
	JPH_SUPPRESS_WARNING_POP

// Standard C++ includes
JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <vector>
#include <utility>
#include <cmath>
#include <sstream>
#include <functional>
#include <algorithm>
JPH_SUPPRESS_WARNINGS_STD_END
#include <limits.h>
#include <float.h>
#include <string.h>
#if defined(JPH_USE_SSE)
	#include <immintrin.h>
#elif defined(JPH_USE_NEON)
	#include <arm_neon.h>
#endif

JPH_NAMESPACE_BEGIN

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
static_assert(sizeof(void *) == (JPH_CPU_ADDRESS_BITS == 64? 8 : 4), "Invalid size of pointer" );

// Define inline macro
#if defined(JPH_COMPILER_CLANG) || defined(JPH_COMPILER_GCC)
	#define JPH_INLINE __inline__ __attribute__((always_inline))
#elif defined(JPH_COMPILER_MSVC)
	#define JPH_INLINE __forceinline
#else
	#error Undefined
#endif

// Cache line size (used for aligning to cache line)
#ifndef JPH_CACHE_LINE_SIZE
	#define JPH_CACHE_LINE_SIZE 64
#endif

// Define macro to get current function name
#if defined(JPH_COMPILER_CLANG) || defined(JPH_COMPILER_GCC)
	#define JPH_FUNCTION_NAME	__PRETTY_FUNCTION__
#elif defined(JPH_COMPILER_MSVC)
	#define JPH_FUNCTION_NAME	__FUNCTION__
#else
	#error Undefined
#endif

// Stack allocation
#define JPH_STACK_ALLOC(n)		alloca(n)
	
// Shorthand for #ifdef _DEBUG / #endif
#ifdef _DEBUG
	#define JPH_IF_DEBUG(...)	__VA_ARGS__
	#define JPH_IF_NOT_DEBUG(...)
#else
	#define JPH_IF_DEBUG(...)
	#define JPH_IF_NOT_DEBUG(...) __VA_ARGS__
#endif

// Shorthand for #ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED / #endif
#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	#define JPH_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(...)	__VA_ARGS__
#else
	#define JPH_IF_FLOATING_POINT_EXCEPTIONS_ENABLED(...)
#endif

// Macro to indicate that a parameter / variable is unused
#define JPH_UNUSED(x)			(void)x

// Macro to enable floating point precise mode and to disable fused multiply add instructions
#if defined(JPH_COMPILER_CLANG) || defined(JPH_COMPILER_GCC) || defined(JPH_CROSS_PLATFORM_DETERMINISTIC)
	// In clang it appears you cannot turn off -ffast-math and -ffp-contract=fast for a code block
	// There is #pragma clang fp contract (off) but that doesn't seem to work under clang 9 & 10 when -ffast-math is specified on the commandline (you override it to turn it on, but not off)
	// There is #pragma float_control(precise, on) but that doesn't work under clang 9.
	// So for now we compile clang without -ffast-math so the macros are empty
	#define JPH_PRECISE_MATH_ON
	#define JPH_PRECISE_MATH_OFF
#elif defined(JPH_COMPILER_MSVC)
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

JPH_NAMESPACE_END

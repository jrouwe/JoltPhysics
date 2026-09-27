// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// The constant \f$\pi\f$
static constexpr float JPH_PI = 3.14159265358979323846f;

/// A large floating point value which, when squared, is still much smaller than FLT_MAX
static constexpr float cLargeFloat = 1.0e15f;

/// Convert a value from degrees to radians
JPH_INLINE constexpr float DegreesToRadians(float inV)
{
	return inV * (JPH_PI / 180.0f);
}

/// Convert a value from radians to degrees
JPH_INLINE constexpr float RadiansToDegrees(float inV)
{
	return inV * (180.0f / JPH_PI);
}

/// Convert angle in radians to the range \f$[-\pi, \pi]\f$
inline float CenterAngleAroundZero(float inV)
{
	if (inV < -JPH_PI)
	{
		do
			inV += 2.0f * JPH_PI;
		while (inV < -JPH_PI);
	}
	else if (inV > JPH_PI)
	{
		do
			inV -= 2.0f * JPH_PI;
		while (inV > JPH_PI);
	}
	JPH_ASSERT(inV >= -JPH_PI && inV <= JPH_PI);
	return inV;
}

/// Calculates inA * inB - inC * inD with higher accuracy when fused multiply add instructions are available.
/// If inA * inB and inC * inD are large, the subtraction can cause a large loss of precision when the result is small.
/// See: https://pharr.org/matt/blog/2019/11/03/difference-of-floats (or search for Kahan's algorithm)
JPH_INLINE float DifferenceOfProducts(float inA, float inB, float inC, float inD)
{
#ifdef JPH_USE_FMADD
	float cd = inC * inD;
	float err = std::fma(-inC, inD, cd);
	float dop = std::fma(inA, inB, -cd);
	return dop + err;
#else
	return inA * inB - inC * inD;
#endif
}

/// Clamp a value between two values
template <typename T>
JPH_INLINE constexpr T Clamp(T inV, T inMin, T inMax)
{
	return min(max(inV, inMin), inMax);
}

/// Square a value
template <typename T>
JPH_INLINE constexpr T Square(T inV)
{
	return inV * inV;
}

/// Take the square root of a float value
JPH_INLINE float Sqrt(float inV)
{
#ifdef JPH_USE_SSE
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(inV)));
#elif defined(JPH_USE_NEON)
	return vget_lane_f32(vsqrt_f32(vdup_n_f32(inV)), 0);
#elif defined(JPH_CPU_RISCV)
	float res;
	asm("fsqrt.s %0, %1" : "=f"(res) : "f"(inV));
	return res;
#else
	return std::sqrt(inV);
#endif
}

/// Take the square root of a double value
JPH_INLINE double Sqrt(double inV)
{
#ifdef JPH_USE_SSE
	return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_undefined_pd(), _mm_set_sd(inV)));
#elif defined(JPH_USE_NEON)
	return vget_lane_f64(vsqrt_f64(vdup_n_f64(inV)), 0);
#elif defined(JPH_CPU_RISCV)
	double res;
	asm("fsqrt.d %0, %1" : "=f"(res) : "f"(inV));
	return res;
#else
	return std::sqrt(inV);
#endif
}

/// Returns \f$inV^3\f$.
template <typename T>
JPH_INLINE constexpr T Cubed(T inV)
{
	return inV * inV * inV;
}

/// Calculates approximate inA / Sqrt(inB).
/// Approximate reciprocal square roots are not bit identical between architectures,
/// so the fast paths are only used when cross platform determinism is not required.
/// Note that inB must be bigger or equal than FLT_MIN, otherwise the result can be Inf.
JPH_INLINE float MulRSqrtApproximate(float inA, float inB)
{
	JPH_ASSERT(inB >= FLT_MIN);

#ifndef JPH_CROSS_PLATFORM_DETERMINISTIC
	#if defined(JPH_USE_SSE) && !defined(JPH_PLATFORM_WASM)
		// One Newton-Raphson step: y' = y * (1.5 - 0.5 * x * y^2)
		__m128 b = _mm_set_ss(inB);
		__m128 rsqrtb = _mm_rsqrt_ss(b);
		rsqrtb = _mm_mul_ss(rsqrtb, _mm_sub_ss(_mm_set_ss(1.5f), _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f), b), _mm_mul_ss(rsqrtb, rsqrtb))));
		return inA * _mm_cvtss_f32(rsqrtb);
	#elif defined(JPH_USE_NEON)
		// vrsqrts_f32 accuracy is less so two Newton-Raphson steps are needed
		float32x2_t b = vdup_n_f32(inB);
		float32x2_t rsqrtb = vrsqrte_f32(b);
		rsqrtb = vmul_f32(rsqrtb, vrsqrts_f32(b, vmul_f32(rsqrtb, rsqrtb)));
		rsqrtb = vmul_f32(rsqrtb, vrsqrts_f32(b, vmul_f32(rsqrtb, rsqrtb)));
		return inA * vget_lane_f32(rsqrtb, 0);
	#elif defined(JPH_USE_RVV)
		// vfrsqrt7 also needs two Newton-Raphson iterations
		vfloat32m1_t b = __riscv_vfmv_v_f_f32m1(inB, 1);
		vfloat32m1_t half_b = __riscv_vfmul_vf_f32m1(b, 0.5f, 1);
		vfloat32m1_t rsqrtb = __riscv_vfrsqrt7_v_f32m1(b, 1);
		rsqrtb = __riscv_vfmul_vv_f32m1(rsqrtb, __riscv_vfrsub_vf_f32m1(__riscv_vfmul_vv_f32m1(half_b, __riscv_vfmul_vv_f32m1(rsqrtb, rsqrtb, 1), 1), 1.5f, 1), 1);
		rsqrtb = __riscv_vfmul_vv_f32m1(rsqrtb, __riscv_vfrsub_vf_f32m1(__riscv_vfmul_vv_f32m1(half_b, __riscv_vfmul_vv_f32m1(rsqrtb, rsqrtb, 1), 1), 1.5f, 1), 1);
		return inA * __riscv_vfmv_f_s_f32m1_f32(rsqrtb);
	#else
		return inA / Sqrt(inB);
	#endif
#else
	return inA / Sqrt(inB);
#endif // JPH_CROSS_PLATFORM_DETERMINISTIC	
}

/// Get the sign of a value
template <typename T>
JPH_INLINE constexpr T Sign(T inV)
{
	return inV < 0? T(-1) : T(1);
}

/// Check if inV is a power of 2
template <typename T>
constexpr bool IsPowerOf2(T inV)
{
	return inV > 0 && (inV & (inV - 1)) == 0;
}

/// Align inV up to the next inAlignment bytes
template <typename T>
inline T AlignUp(T inV, uint64 inAlignment)
{
	JPH_ASSERT(IsPowerOf2(inAlignment));
	return T((uint64(inV) + inAlignment - 1) & ~(inAlignment - 1));
}

/// Check if inV is inAlignment aligned
template <typename T>
inline bool IsAligned(T inV, uint64 inAlignment)
{
	JPH_ASSERT(IsPowerOf2(inAlignment));
	return (uint64(inV) & (inAlignment - 1)) == 0;
}

/// Compute number of trailing zero bits (how many low bits are zero)
inline uint CountTrailingZeros(uint32 inValue)
{
#if defined(JPH_CPU_X86) || defined(JPH_CPU_WASM)
	#if defined(JPH_USE_TZCNT)
		return _tzcnt_u32(inValue);
	#elif defined(JPH_COMPILER_MSVC)
		if (inValue == 0)
			return 32;
		unsigned long result;
		_BitScanForward(&result, inValue);
		return result;
	#else
		if (inValue == 0)
			return 32;
		return __builtin_ctz(inValue);
	#endif
#elif defined(JPH_CPU_ARM)
	#if defined(JPH_COMPILER_MSVC)
		if (inValue == 0)
			return 32;
		unsigned long result;
		_BitScanForward(&result, inValue);
		return result;
	#else
		if (inValue == 0)
			return 32;
		return __builtin_ctz(inValue);
	#endif
#elif defined(JPH_CPU_E2K) || defined(JPH_CPU_RISCV) || defined(JPH_CPU_PPC) || defined(JPH_CPU_LOONGARCH)
	return inValue ? __builtin_ctz(inValue) : 32;
#else
	#error Undefined
#endif
}

/// Compute the number of leading zero bits (how many high bits are zero)
inline uint CountLeadingZeros(uint32 inValue)
{
#if defined(JPH_CPU_X86) || defined(JPH_CPU_WASM)
	#if defined(JPH_USE_LZCNT)
		return _lzcnt_u32(inValue);
	#elif defined(JPH_COMPILER_MSVC)
		if (inValue == 0)
			return 32;
		unsigned long result;
		_BitScanReverse(&result, inValue);
		return 31 - result;
	#else
		if (inValue == 0)
			return 32;
		return __builtin_clz(inValue);
	#endif
#elif defined(JPH_CPU_ARM)
	#if defined(JPH_COMPILER_MSVC)
		return _CountLeadingZeros(inValue);
	#else
		return __builtin_clz(inValue);
	#endif
#elif defined(JPH_CPU_E2K) || defined(JPH_CPU_RISCV) || defined(JPH_CPU_PPC) || defined(JPH_CPU_LOONGARCH)
	return inValue ? __builtin_clz(inValue) : 32;
#else
	#error Undefined
#endif
}

/// Count the number of 1 bits in a value
inline uint CountBits(uint32 inValue)
{
#if defined(JPH_COMPILER_CLANG) || defined(JPH_COMPILER_GCC)
	return __builtin_popcount(inValue);
#elif defined(JPH_COMPILER_MSVC)
	#if defined(JPH_USE_SSE4_2)
		return _mm_popcnt_u32(inValue);
	#elif defined(JPH_USE_NEON) && (_MSC_VER >= 1930) // _CountOneBits not available on MSVC2019
		return _CountOneBits(inValue);
	#else
		inValue = inValue - ((inValue >> 1) & 0x55555555);
		inValue = (inValue & 0x33333333) + ((inValue >> 2) & 0x33333333);
		inValue = (inValue + (inValue >> 4)) & 0x0F0F0F0F;
		return (inValue * 0x01010101) >> 24;
	#endif
#else
	#error Undefined
#endif
}

/// Get the next higher power of 2 of a value, or the value itself if the value is already a power of 2
inline uint32 GetNextPowerOf2(uint32 inValue)
{
	return inValue <= 1? uint32(1) : uint32(1) << (32 - CountLeadingZeros(inValue - 1));
}

/// Simple implementation of C++20 std::bit_cast
template <class To, class From>
JPH_INLINE constexpr To BitCast(const From &inValue)
{
	static_assert(std::is_trivially_constructible_v<To>);
	static_assert(sizeof(From) == sizeof(To));
	return __builtin_bit_cast(To, inValue);
}

JPH_NAMESPACE_END

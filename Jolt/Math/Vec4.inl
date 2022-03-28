// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/UVec4.h>

JPH_NAMESPACE_BEGIN

// Constructor
Vec4::Vec4(Vec3Arg inRHS) : 
	mValue(inRHS.mValue) 
{ 
}

Vec4::Vec4(Vec3Arg inRHS, float inW)
{
#if defined(JPH_USE_SSE4_1)
	mValue = _mm_blend_ps(inRHS.mValue, _mm_set1_ps(inW), 8);
#elif defined(JPH_USE_NEON)
	mValue = vsetq_lane_f32(inW, inRHS.mValue, 3);
#else
	for (int i = 0; i < 3; i++)
		mF32[i] = inRHS.mF32[i];
	mF32[3] = inW;
#endif
}

Vec4::Vec4(float inX, float inY, float inZ, float inW)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_set_ps(inW, inZ, inY, inX);
#elif defined(JPH_USE_NEON)
	uint32x2_t xy = vcreate_f32(static_cast<uint64>(*reinterpret_cast<uint32 *>(&inX)) | (static_cast<uint64>(*reinterpret_cast<uint32 *>(&inY)) << 32));
	uint32x2_t zw = vcreate_f32(static_cast<uint64>(*reinterpret_cast<uint32* >(&inZ)) | (static_cast<uint64>(*reinterpret_cast<uint32 *>(&inW)) << 32));
	mValue = vcombine_f32(xy, zw);
#else
	#error Undefined CPU architecture
#endif
}

template<uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
Vec4 Vec4::Swizzle() const
{
	static_assert(SwizzleX <= 3, "SwizzleX template parameter out of range");
	static_assert(SwizzleY <= 3, "SwizzleY template parameter out of range");
	static_assert(SwizzleZ <= 3, "SwizzleZ template parameter out of range");
	static_assert(SwizzleW <= 3, "SwizzleW template parameter out of range");

#if defined(JPH_USE_SSE)
	return _mm_shuffle_ps(mValue, mValue, _MM_SHUFFLE(SwizzleW, SwizzleZ, SwizzleY, SwizzleX));
#elif defined(JPH_USE_NEON)
	return __builtin_shufflevector(mValue, mValue, SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sZero()
{
#if defined(JPH_USE_SSE)
	return _mm_setzero_ps();
#elif defined(JPH_USE_NEON)
	return vdupq_n_f32(0);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sReplicate(float inV)
{
#if defined(JPH_USE_SSE)
	return _mm_set1_ps(inV);
#elif defined(JPH_USE_NEON)
	return vdupq_n_f32(inV);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sNaN()
{
	return sReplicate(numeric_limits<float>::quiet_NaN());
}

Vec4 Vec4::sLoadFloat4(const Float4 *inV)
{
#if defined(JPH_USE_SSE)
	return _mm_loadu_ps(&inV->x);
#elif defined(JPH_USE_NEON)
	return vld1q_f32(&inV->x);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sLoadFloat4Aligned(const Float4 *inV)
{
#if defined(JPH_USE_SSE)
	return _mm_load_ps(&inV->x);
#elif defined(JPH_USE_NEON)
	return vld1q_f32(&inV->x);
#else
	#error Unsupported CPU architecture
#endif
}

template <const int Scale>
Vec4 Vec4::sGatherFloat4(const float *inBase, UVec4Arg inOffsets)
{
#if defined(JPH_USE_SSE)
	#ifdef JPH_USE_AVX2
		return _mm_i32gather_ps(inBase, inOffsets.mValue, Scale);
	#else
		const uint8 *base = reinterpret_cast<const uint8 *>(inBase);
		Type x = _mm_load_ss(reinterpret_cast<const float *>(base + inOffsets.GetX() * Scale));
		Type y = _mm_load_ss(reinterpret_cast<const float *>(base + inOffsets.GetY() * Scale));
		Type xy = _mm_unpacklo_ps(x, y);
		Type z = _mm_load_ss(reinterpret_cast<const float *>(base + inOffsets.GetZ() * Scale));
		Type w = _mm_load_ss(reinterpret_cast<const float *>(base + inOffsets.GetW() * Scale));
		Type zw = _mm_unpacklo_ps(z, w);
		return _mm_movelh_ps(xy, zw);
	#endif
#else
	const uint8 *base = reinterpret_cast<const uint8 *>(inBase);
	float x = *reinterpret_cast<const float *>(base + inOffsets.GetX() * Scale);
	float y = *reinterpret_cast<const float *>(base + inOffsets.GetY() * Scale);
	float z = *reinterpret_cast<const float *>(base + inOffsets.GetZ() * Scale);
	float w = *reinterpret_cast<const float *>(base + inOffsets.GetW() * Scale);
	return Vec4(x, y, z, w);
#endif
}

Vec4 Vec4::sMin(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_min_ps(inV1.mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vminq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sMax(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_max_ps(inV1.mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vmaxq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::sEquals(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_castps_si128(_mm_cmpeq_ps(inV1.mValue, inV2.mValue));
#elif defined(JPH_USE_NEON)
	return vceqq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::sLess(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_castps_si128(_mm_cmplt_ps(inV1.mValue, inV2.mValue));
#elif defined(JPH_USE_NEON)
	return vcltq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::sLessOrEqual(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_castps_si128(_mm_cmple_ps(inV1.mValue, inV2.mValue));
#elif defined(JPH_USE_NEON)
	return vcleq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::sGreater(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_castps_si128(_mm_cmpgt_ps(inV1.mValue, inV2.mValue));
#elif defined(JPH_USE_NEON)
	return vcgtq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::sGreaterOrEqual(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_castps_si128(_mm_cmpge_ps(inV1.mValue, inV2.mValue));
#elif defined(JPH_USE_NEON)
	return vcgeq_f32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sFusedMultiplyAdd(Vec4Arg inMul1, Vec4Arg inMul2, Vec4Arg inAdd)
{
#if defined(JPH_USE_SSE)
	#ifdef JPH_USE_FMADD
		return _mm_fmadd_ps(inMul1.mValue, inMul2.mValue, inAdd.mValue);
	#else
		return _mm_add_ps(_mm_mul_ps(inMul1.mValue, inMul2.mValue), inAdd.mValue);
	#endif
#elif defined(JPH_USE_NEON)
	return vmlaq_f32(inAdd.mValue, inMul1.mValue, inMul2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sSelect(Vec4Arg inV1, Vec4Arg inV2, UVec4Arg inControl)
{
#if defined(JPH_USE_SSE4_1)
	return _mm_blendv_ps(inV1.mValue, inV2.mValue, _mm_castsi128_ps(inControl.mValue));
#elif defined(JPH_USE_NEON)
	return vbslq_f32(vshrq_n_s32(inControl.mValue, 31), inV2.mValue, inV1.mValue);
#else
	Vec4 result;
	for (int i = 0; i < 4; i++)
		result.mF32[i] = inControl.mU32[i] ? inV2.mF32[i] : inV1.mF32[i];
	return result;
#endif
}

Vec4 Vec4::sOr(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_or_ps(inV1.mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vorrq_s32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sXor(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_xor_ps(inV1.mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return veorq_s32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::sAnd(Vec4Arg inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_and_ps(inV1.mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vandq_s32(inV1.mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

void Vec4::sSort4(Vec4 &ioValue, UVec4 &ioIndex)
{
	// Pass 1, test 1st vs 3rd, 2nd vs 4th
	Vec4 v1 = ioValue.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>();
	UVec4 i1 = ioIndex.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>();
	UVec4 c1 = sLess(ioValue, v1).Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v1, c1);
	ioIndex = UVec4::sSelect(ioIndex, i1, c1);

	// Pass 2, test 1st vs 2nd, 3rd vs 4th
	Vec4 v2 = ioValue.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>();
	UVec4 i2 = ioIndex.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>();
	UVec4 c2 = sLess(ioValue, v2).Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v2, c2);
	ioIndex = UVec4::sSelect(ioIndex, i2, c2);

	// Pass 3, test 2nd vs 3rd component
	Vec4 v3 = ioValue.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>();
	UVec4 i3 = ioIndex.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>();
	UVec4 c3 = sLess(ioValue, v3).Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v3, c3);
	ioIndex = UVec4::sSelect(ioIndex, i3, c3);
}

void Vec4::sSort4Reverse(Vec4 &ioValue, UVec4 &ioIndex)
{
	// Pass 1, test 1st vs 3rd, 2nd vs 4th
	Vec4 v1 = ioValue.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>();
	UVec4 i1 = ioIndex.Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_X, SWIZZLE_Y>();
	UVec4 c1 = sGreater(ioValue, v1).Swizzle<SWIZZLE_Z, SWIZZLE_W, SWIZZLE_Z, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v1, c1);
	ioIndex = UVec4::sSelect(ioIndex, i1, c1);

	// Pass 2, test 1st vs 2nd, 3rd vs 4th
	Vec4 v2 = ioValue.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>();
	UVec4 i2 = ioIndex.Swizzle<SWIZZLE_Y, SWIZZLE_X, SWIZZLE_W, SWIZZLE_Z>();
	UVec4 c2 = sGreater(ioValue, v2).Swizzle<SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_W, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v2, c2);
	ioIndex = UVec4::sSelect(ioIndex, i2, c2);

	// Pass 3, test 2nd vs 3rd component
	Vec4 v3 = ioValue.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>();
	UVec4 i3 = ioIndex.Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Y, SWIZZLE_W>();
	UVec4 c3 = sGreater(ioValue, v3).Swizzle<SWIZZLE_X, SWIZZLE_Z, SWIZZLE_Z, SWIZZLE_W>();
	ioValue = sSelect(ioValue, v3, c3);
	ioIndex = UVec4::sSelect(ioIndex, i3, c3);
}

bool Vec4::operator == (Vec4Arg inV2) const 
{ 
	return sEquals(*this, inV2).TestAllTrue();
}

bool Vec4::IsClose(Vec4Arg inV2, float inMaxDistSq) const
{
	return (inV2 - *this).LengthSq() <= inMaxDistSq;
}

bool Vec4::IsNormalized(float inTolerance) const 
{ 
	return abs(LengthSq() - 1.0f) <= inTolerance; 
}

bool Vec4::IsNaN() const
{
#if defined(JPH_USE_SSE)
	return _mm_movemask_ps(_mm_cmpunord_ps(mValue, mValue)) != 0;
#elif defined(JPH_USE_NEON)
	uint32x4_t is_equal = vceqq_f32(mValue, mValue); // If a number is not equal to itself it's a NaN
	return vaddvq_u32(vshrq_n_u32(is_equal, 31)) != 4;
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::operator * (Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_mul_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vmulq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::operator * (float inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_mul_ps(mValue, _mm_set1_ps(inV2));
#elif defined(JPH_USE_NEON)
	return vmulq_n_f32(mValue, inV2);
#else
	#error Unsupported CPU architecture
#endif
}

/// Multiply vector with float
Vec4 operator * (float inV1, Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	return _mm_mul_ps(_mm_set1_ps(inV1), inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vmulq_n_f32(inV2.mValue, inV1);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::operator / (float inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_div_ps(mValue, _mm_set1_ps(inV2));
#elif defined(JPH_USE_NEON)
	return vdivq_f32(mValue, vdupq_n_f32(inV2));
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 &Vec4::operator *= (float inV2)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_mul_ps(mValue, _mm_set1_ps(inV2));
#elif defined(JPH_USE_NEON)
	mValue = vmulq_n_f32(mValue, inV2);
#else
	#error Unsupported CPU architecture
#endif
	return *this;
}

Vec4 &Vec4::operator *= (Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_mul_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	mValue = vmulq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
	return *this;
}

Vec4 &Vec4::operator /= (float inV2)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_div_ps(mValue, _mm_set1_ps(inV2));
#elif defined(JPH_USE_NEON)
	mValue = vdivq_f32(mValue, vdupq_n_f32(inV2));
#else
	#error Unsupported CPU architecture
#endif
	return *this;
}

Vec4 Vec4::operator + (Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_add_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vaddq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 &Vec4::operator += (Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_add_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	mValue = vaddq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
	return *this;
}

Vec4 Vec4::operator - () const
{
#if defined(JPH_USE_SSE)
	return _mm_sub_ps(_mm_setzero_ps(), mValue);
#elif defined(JPH_USE_NEON)
	return vnegq_f32(mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::operator - (Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_sub_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vsubq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 &Vec4::operator -= (Vec4Arg inV2)
{
#if defined(JPH_USE_SSE)
	mValue = _mm_sub_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	mValue = vsubq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
	return *this;
}

Vec4 Vec4::operator / (Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE)
	return _mm_div_ps(mValue, inV2.mValue);
#elif defined(JPH_USE_NEON)
	return vdivq_f32(mValue, inV2.mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::SplatX() const
{
#if defined(JPH_USE_SSE)
	return _mm_shuffle_ps(mValue, mValue, _MM_SHUFFLE(0, 0, 0, 0));
#elif defined(JPH_USE_NEON)
	return vdupq_laneq_f32(mValue, 0);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::SplatY() const
{
#if defined(JPH_USE_SSE)
	return _mm_shuffle_ps(mValue, mValue, _MM_SHUFFLE(1, 1, 1, 1));
#elif defined(JPH_USE_NEON)
	return vdupq_laneq_f32(mValue, 1);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::SplatZ() const
{
#if defined(JPH_USE_SSE)
	return _mm_shuffle_ps(mValue, mValue, _MM_SHUFFLE(2, 2, 2, 2));
#elif defined(JPH_USE_NEON)
	return vdupq_laneq_f32(mValue, 2);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::SplatW() const
{
#if defined(JPH_USE_SSE)
	return _mm_shuffle_ps(mValue, mValue, _MM_SHUFFLE(3, 3, 3, 3));
#elif defined(JPH_USE_NEON)
	return vdupq_laneq_f32(mValue, 3);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::Abs() const
{
#if defined(JPH_USE_SSE)
	return _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), mValue), mValue);
#elif defined(JPH_USE_NEON)
	return vabsq_f32(mValue);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::Reciprocal() const
{
	return sReplicate(1.0f) / mValue;
}

Vec4 Vec4::DotV(Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE4_1)
	return _mm_dp_ps(mValue, inV2.mValue, 0xff);
#elif defined(JPH_USE_NEON)
    float32x4_t mul = vmulq_f32(mValue, inV2.mValue);
    return vdupq_n_f32(vaddvq_f32(mul));
#else
	float dot = 0.0f;
	for (int i = 0; i < 4; i++)
		dot += mF32[i] * inV2.mF32[i];
	return Vec4::sReplicate(dot);
#endif
}

float Vec4::Dot(Vec4Arg inV2) const
{
#if defined(JPH_USE_SSE4_1)
	return _mm_cvtss_f32(_mm_dp_ps(mValue, inV2.mValue, 0xff));
#elif defined(JPH_USE_NEON)
    float32x4_t mul = vmulq_f32(mValue, inV2.mValue);
    return vaddvq_f32(mul);
#else
	float dot = 0.0f;
	for (int i = 0; i < 4; i++)
		dot += mF32[i] * inV2.mF32[i];
	return dot;
#endif
}

float Vec4::LengthSq() const
{
#if defined(JPH_USE_SSE4_1)
	return _mm_cvtss_f32(_mm_dp_ps(mValue, mValue, 0xff));
#elif defined(JPH_USE_NEON)
    float32x4_t mul = vmulq_f32(mValue, mValue);
    return vaddvq_f32(mul);
#else
	float len_sq = 0.0f;
	for (int i = 0; i < 4; i++)
		len_sq += mF32[i] * mF32[i];
	return len_sq;
#endif
}

float Vec4::Length() const
{
#if defined(JPH_USE_SSE4_1)
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(mValue, mValue, 0xff)));
#elif defined(JPH_USE_NEON)
    float32x4_t mul = vmulq_f32(mValue, mValue);
    float32x2_t sum = vdup_n_f32(vaddvq_f32(mul));
    return vget_lane_f32(vsqrt_f32(sum), 0);
#else
	return sqrt(LengthSq());
#endif
}

Vec4 Vec4::Sqrt() const
{
#if defined(JPH_USE_SSE)
	return _mm_sqrt_ps(mValue);
#elif defined(JPH_USE_NEON)
	return vsqrtq_f32(mValue);
#else
	#error Unsupported CPU architecture
#endif
}


Vec4 Vec4::GetSign() const
{
#if defined(JPH_USE_SSE)
	Type minus_one = _mm_set1_ps(-1.0f);
	Type one = _mm_set1_ps(1.0f);
	return _mm_or_ps(_mm_and_ps(mValue, minus_one), one);
#elif defined(JPH_USE_NEON)
	Type minus_one = vdupq_n_f32(-1.0f);
	Type one = vdupq_n_f32(1.0f);
	return vorrq_s32(vandq_s32(mValue, minus_one), one);
#else
	#error Unsupported CPU architecture
#endif
}

Vec4 Vec4::Normalized() const
{
#if defined(JPH_USE_SSE4_1)
	return _mm_div_ps(mValue, _mm_sqrt_ps(_mm_dp_ps(mValue, mValue, 0xff)));
#elif defined(JPH_USE_NEON)
    float32x4_t mul = vmulq_f32(mValue, mValue);
    float32x4_t sum = vdupq_n_f32(vaddvq_f32(mul));
    return vdivq_f32(mValue, vsqrtq_f32(sum));
#else
	return *this / Length();
#endif
}

void Vec4::StoreFloat4(Float4 *outV) const
{
#if defined(JPH_USE_SSE)
	_mm_storeu_ps(&outV->x, mValue);
#elif defined(JPH_USE_NEON)
    vst1q_f32(&outV->x, mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::ToInt() const
{
#if defined(JPH_USE_SSE)
	return _mm_cvttps_epi32(mValue);
#elif defined(JPH_USE_NEON)
	return vcvtq_u32_f32(mValue);
#else
	#error Unsupported CPU architecture
#endif
}

UVec4 Vec4::ReinterpretAsInt() const
{
#if defined(JPH_USE_SSE)
	return UVec4(_mm_castps_si128(mValue));
#elif defined(JPH_USE_NEON)
	return vreinterpretq_u32_f32(mValue);
#else
	#error Unsupported CPU architecture
#endif
}

int Vec4::GetSignBits() const
{
#if defined(JPH_USE_SSE)
	return _mm_movemask_ps(mValue);
#elif defined(JPH_USE_NEON)
    int32x4_t shift = { 0, 1, 2, 3 };
    return vaddvq_u32(vshlq_u32(vshrq_n_u32(vreinterpretq_u32_f32(mValue), 31), shift));
#else
	#error Unsupported CPU architecture
#endif
}

float Vec4::ReduceMin() const
{
	Vec4 v = sMin(mValue, Swizzle<SWIZZLE_Y, SWIZZLE_UNUSED, SWIZZLE_W, SWIZZLE_UNUSED>());
	v = sMin(v, v.Swizzle<SWIZZLE_Z, SWIZZLE_UNUSED, SWIZZLE_UNUSED, SWIZZLE_UNUSED>());
	return v.GetX();
}

float Vec4::ReduceMax() const
{
	Vec4 v = sMax(mValue, Swizzle<SWIZZLE_Y, SWIZZLE_UNUSED, SWIZZLE_W, SWIZZLE_UNUSED>());
	v = sMax(v, v.Swizzle<SWIZZLE_Z, SWIZZLE_UNUSED, SWIZZLE_UNUSED, SWIZZLE_UNUSED>());
	return v.GetX();
}

JPH_NAMESPACE_END

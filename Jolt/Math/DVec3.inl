// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/HashCombine.h>

// Create a std::hash for DVec3
JPH_MAKE_HASHABLE(JPH::DVec3, t.GetX(), t.GetY(), t.GetZ())

JPH_NAMESPACE_BEGIN

DVec3::DVec3(Vec3Arg inRHS)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_cvtps_pd(inRHS.mValue);
#else
	mD32[0] = (double)inRHS.GetX();
	mD32[1] = (double)inRHS.GetY();
	mD32[2] = (double)inRHS.GetZ();
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
}

DVec3::DVec3(double inX, double inY, double inZ)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_set_pd(inZ, inZ, inY, inX); // Assure Z and W are the same
#else
	mD32[0] = inX;
	mD32[1] = inY;
	mD32[2] = inZ;
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
}

void DVec3::CheckW() const
{
#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	// Avoid asserts when both components are NaN
	JPH_ASSERT(reinterpret_cast<const uint64 *>(mD32)[2] == reinterpret_cast<const uint64 *>(mD32)[3]); 
#endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
} 
	
/// Internal helper function that ensures that the Z component is replicated to the W component to prevent divisions by zero
DVec3::Type DVec3::sFixW(Type inValue)
{
#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	#if defined(JPH_USE_AVX)
		return _mm256_shuffle_pd(inValue, inValue, 2);
	#else
		Type value;
		value.mData[0] = inValue.mData[0];
		value.mData[1] = inValue.mData[1];
		value.mData[2] = inValue.mData[2];
		value.mData[3] = inValue.mData[2];
		return value;
	#endif
#else
	return inValue;
#endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
}

DVec3 DVec3::sZero()
{
	#if defined(JPH_USE_AVX)
		return _mm256_setzero_pd();
	#else
		return DVec3(0, 0, 0);
	#endif
}

DVec3 DVec3::sReplicate(double inV)
{
	#if defined(JPH_USE_AVX)
		return _mm256_set1_pd(inV);
	#else
		return DVec3(inV, inV, inV);
	#endif
}

DVec3 DVec3::sLoadDouble3Unsafe(const double *inV)
{
	#if defined(JPH_USE_AVX)
		Type v = _mm256_loadu_pd(inV);
	#else
		Type v = { inV[0], inV[1], inV[2] };
	#endif
	return sFixW(v);
}

Vec3 DVec3::ToVec3() const
{
	#if defined(JPH_USE_AVX)
		return _mm256_cvtpd_ps(mValue);
	#else
		return Vec3((float)GetX(), (float)GetY(), (float)GetZ());
	#endif
}

DVec3 DVec3::sMin(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_min_pd(inV1.mValue, inV2.mValue);
#else
	return DVec3(min(inV1.mD32[0], inV2.mD32[0]), 
				 min(inV1.mD32[1], inV2.mD32[1]), 
				 min(inV1.mD32[2], inV2.mD32[2]));
#endif
}

DVec3 DVec3::sMax(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_max_pd(inV1.mValue, inV2.mValue);
#else
	return DVec3(max(inV1.mD32[0], inV2.mD32[0]), 
				 max(inV1.mD32[1], inV2.mD32[1]), 
				 max(inV1.mD32[2], inV2.mD32[2]));
#endif
}

DVec3 DVec3::sClamp(DVec3Arg inV, DVec3Arg inMin, DVec3Arg inMax)
{
	return sMax(sMin(inV, inMax), inMin);
}

DVec3 DVec3::sEquals(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_EQ_OQ);
#else
	return DVec3(inV1.mD32[0] == inV2.mD32[0]? cTrue : cFalse, 
				 inV1.mD32[1] == inV2.mD32[1]? cTrue : cFalse, 
				 inV1.mD32[2] == inV2.mD32[2]? cTrue : cFalse);
#endif
}

DVec3 DVec3::sLess(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_LT_OQ);
#else
	return DVec3(inV1.mD32[0] < inV2.mD32[0]? cTrue : cFalse, 
				 inV1.mD32[1] < inV2.mD32[1]? cTrue : cFalse, 
				 inV1.mD32[2] < inV2.mD32[2]? cTrue : cFalse);
#endif
}

DVec3 DVec3::sLessOrEqual(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_LE_OQ);
#else
	return DVec3(inV1.mD32[0] <= inV2.mD32[0]? cTrue : cFalse, 
				 inV1.mD32[1] <= inV2.mD32[1]? cTrue : cFalse, 
				 inV1.mD32[2] <= inV2.mD32[2]? cTrue : cFalse);
#endif
}

DVec3 DVec3::sGreater(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_GT_OQ);
#else
	return DVec3(inV1.mD32[0] > inV2.mD32[0]? cTrue : cFalse, 
				 inV1.mD32[1] > inV2.mD32[1]? cTrue : cFalse, 
				 inV1.mD32[2] > inV2.mD32[2]? cTrue : cFalse);
#endif
}

DVec3 DVec3::sGreaterOrEqual(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_GE_OQ);
#else
	return DVec3(inV1.mD32[0] >= inV2.mD32[0]? cTrue : cFalse, 
				 inV1.mD32[1] >= inV2.mD32[1]? cTrue : cFalse, 
				 inV1.mD32[2] >= inV2.mD32[2]? cTrue : cFalse);
#endif
}

DVec3 DVec3::sFusedMultiplyAdd(DVec3Arg inMul1, DVec3Arg inMul2, DVec3Arg inAdd)
{
#if defined(JPH_USE_AVX)
	#ifdef JPH_USE_FMADD
		return _mm256_fmadd_pd(inMul1.mValue, inMul2.mValue, inAdd.mValue);
	#else
		return _mm256_add_pd(_mm256_mul_pd(inMul1.mValue, inMul2.mValue), inAdd.mValue);
	#endif
#else
	return inMul1 * inMul2 + inAdd;
#endif
}

DVec3 DVec3::sSelect(DVec3Arg inV1, DVec3Arg inV2, DVec3Arg inControl)
{
#if defined(JPH_USE_AVX)
	return _mm256_blendv_pd(inV1.mValue, inV2.mValue, inControl.mValue);
#else
	DVec3 result;
	for (int i = 0; i < 3; i++)
		result.mD32[i] = inControl.mD32[i] == cTrue? inV2.mD32[i] : inV1.mD32[i];
#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	result.mD32[3] = result.mD32[2];
#endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	return result;
#endif
}

DVec3 DVec3::sOr(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_or_pd(inV1.mValue, inV2.mValue);
#else
	return DVec3(BitCast<double>(BitCast<uint64>(inV1.mD32[0]) | BitCast<uint64>(inV1.mD32[0])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[1]) | BitCast<uint64>(inV1.mD32[1])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[2]) | BitCast<uint64>(inV1.mD32[2])));
#endif
}

DVec3 DVec3::sXor(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_xor_pd(inV1.mValue, inV2.mValue);
#else
	return DVec3(BitCast<double>(BitCast<uint64>(inV1.mD32[0]) ^ BitCast<uint64>(inV1.mD32[0])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[1]) ^ BitCast<uint64>(inV1.mD32[1])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[2]) ^ BitCast<uint64>(inV1.mD32[2])));
#endif
}

DVec3 DVec3::sAnd(DVec3Arg inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_and_pd(inV1.mValue, inV2.mValue);
#else
	return DVec3(BitCast<double>(BitCast<uint64>(inV1.mD32[0]) & BitCast<uint64>(inV1.mD32[0])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[1]) & BitCast<uint64>(inV1.mD32[1])),
				 BitCast<double>(BitCast<uint64>(inV1.mD32[2]) & BitCast<uint64>(inV1.mD32[2])));
#endif
}

int DVec3::GetTrues() const
{
#if defined(JPH_USE_AVX)
	return _mm256_movemask_pd(mValue) & 0x7;
#else
	return int((BitCast<uint64>(mD32[0]) >> 63) | ((BitCast<uint64>(mD32[1]) >> 63) << 1) | ((BitCast<uint64>(mD32[2]) >> 63) << 2));
#endif
}

bool DVec3::TestAnyTrue() const
{
	return GetTrues() != 0;
}

bool DVec3::TestAllTrue() const
{
	return GetTrues() == 0x7;
}

bool DVec3::operator == (DVec3Arg inV2) const 
{ 
	return sEquals(*this, inV2).TestAllTrue();
}

bool DVec3::IsClose(DVec3Arg inV2, double inMaxDistSq) const
{
	return (inV2 - *this).LengthSq() <= inMaxDistSq;
}

bool DVec3::IsNearZero(double inMaxDistSq) const
{
	return LengthSq() <= inMaxDistSq;
}

DVec3 DVec3::operator * (DVec3Arg inV2) const
{
#if defined(JPH_USE_AVX)
	return _mm256_mul_pd(mValue, inV2.mValue);
#else
	return DVec3(mD32[0] * inV2.mD32[0], mD32[1] * inV2.mD32[1], mD32[2] * inV2.mD32[2]);
#endif
}

DVec3 DVec3::operator * (double inV2) const
{
#if defined(JPH_USE_AVX)
	return _mm256_mul_pd(mValue, _mm256_set1_pd(inV2));
#else
	return DVec3(mD32[0] * inV2, mD32[1] * inV2, mD32[2] * inV2);
#endif
}

DVec3 operator * (double inV1, DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	return _mm256_mul_pd(_mm256_set1_pd(inV1), inV2.mValue);
#else
	return DVec3(inV1 * inV2.mD32[0], inV1 * inV2.mD32[1], inV1 * inV2.mD32[2]);
#endif
}

DVec3 DVec3::operator / (double inV2) const
{
#if defined(JPH_USE_AVX)
	return _mm256_div_pd(mValue, _mm256_set1_pd(inV2));
#else
	return DVec3(mD32[0] / inV2, mD32[1] / inV2, mD32[2] / inV2);
#endif
}

DVec3 &DVec3::operator *= (double inV2)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_mul_pd(mValue, _mm256_set1_pd(inV2));
#else
	for (int i = 0; i < 3; ++i)
		mD32[i] *= inV2;
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
	return *this;
}

DVec3 &DVec3::operator *= (DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_mul_pd(mValue, inV2.mValue);
#else
	for (int i = 0; i < 3; ++i)
		mD32[i] *= inV2.mD32[i];
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
	return *this;
}

DVec3 &DVec3::operator /= (double inV2)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_div_pd(mValue, _mm256_set1_pd(inV2));
#else
	for (int i = 0; i < 3; ++i)
		mD32[i] /= inV2;
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
	return *this;
}

DVec3 DVec3::operator + (DVec3Arg inV2) const
{
#if defined(JPH_USE_AVX)
	return _mm256_add_pd(mValue, inV2.mValue);
#else
	return DVec3(mD32[0] + inV2.mD32[0], mD32[1] + inV2.mD32[1], mD32[2] + inV2.mD32[2]);
#endif
}

DVec3 &DVec3::operator += (DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_add_pd(mValue, inV2.mValue);
#else
	for (int i = 0; i < 3; ++i)
		mD32[i] += inV2.mD32[i];
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
	return *this;
}

DVec3 DVec3::operator - () const
{
#if defined(JPH_USE_AVX)
	return _mm256_sub_pd(_mm256_setzero_pd(), mValue);
#else
	return DVec3(-mD32[0], -mD32[1], -mD32[2]);
#endif
}

DVec3 DVec3::operator - (DVec3Arg inV2) const
{
#if defined(JPH_USE_AVX)
	return _mm256_sub_pd(mValue, inV2.mValue);
#else
	return DVec3(mD32[0] - inV2.mD32[0], mD32[1] - inV2.mD32[1], mD32[2] - inV2.mD32[2]);
#endif
}

DVec3 &DVec3::operator -= (DVec3Arg inV2)
{
#if defined(JPH_USE_AVX)
	mValue = _mm256_sub_pd(mValue, inV2.mValue);
#else
	for (int i = 0; i < 3; ++i)
		mD32[i] -= inV2.mD32[i];
	#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
		mD32[3] = mD32[2];
	#endif
#endif
	return *this;
}

DVec3 DVec3::operator / (DVec3Arg inV2) const
{
	inV2.CheckW();
#if defined(JPH_USE_AVX)
	return _mm256_div_pd(mValue, inV2.mValue);
#else
	return DVec3(mD32[0] / inV2.mD32[0], mD32[1] / inV2.mD32[1], mD32[2] / inV2.mD32[2]);
#endif
}

DVec3 DVec3::Abs() const
{
#if defined(JPH_USE_AVX512)
	return _mm256_range_pd(mValue, mValue, 0b1000);
#elif defined(JPH_USE_AVX)
	return _mm256_max_pd(_mm256_sub_pd(_mm256_setzero_pd(), mValue), mValue);
#else
	return DVec3(abs(mD32[0]), abs(mD32[1]), abs(mD32[2]));
#endif
}

DVec3 DVec3::Reciprocal() const
{
	return sReplicate(1.0) / mValue;
}

DVec3 DVec3::Cross(DVec3Arg inV2) const
{
#if defined(JPH_USE_AVX2)
	__m256d t1 = _mm256_permute4x64_pd(inV2.mValue, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
    t1 = _mm256_mul_pd(t1, mValue);
    __m256d t2 = _mm256_permute4x64_pd(mValue, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
    t2 = _mm256_mul_pd(t2, inV2.mValue);
    __m256d t3 = _mm256_sub_pd(t1, t2);
    return _mm256_permute4x64_pd(t3, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
#else
	return DVec3(mD32[1] * inV2.mD32[2] - mD32[2] * inV2.mD32[1],
				 mD32[2] * inV2.mD32[0] - mD32[0] * inV2.mD32[2],
				 mD32[0] * inV2.mD32[1] - mD32[1] * inV2.mD32[0]);
#endif
}

double DVec3::Dot(DVec3Arg inV2) const
{
#if defined(JPH_USE_AVX)
	__m256d mul = _mm256_mul_pd(mValue, inV2.mValue);
    __m128d xy = _mm256_castpd256_pd128(mul);
	__m128d yx = _mm_shuffle_pd(xy, xy, 1);
	__m128d sum = _mm_add_pd(xy, yx);
    __m128d zw = _mm256_extractf128_pd(mul, 1);
	sum = _mm_add_pd(sum, zw);
	return _mm_cvtsd_f64(sum);
#else
	double dot = 0.0;
	for (int i = 0; i < 3; i++)
		dot += mD32[i] * inV2.mD32[i];
	return dot;
#endif
}

double DVec3::LengthSq() const
{
	return Dot(*this);
}

DVec3 DVec3::Sqrt() const
{
#if defined(JPH_USE_AVX)
	return _mm256_sqrt_pd(mValue);
#else
	return DVec3(sqrt(mD32[0]), sqrt(mD32[1]), sqrt(mD32[2]));
#endif
}

double DVec3::Length() const
{
	return sqrt(Dot(*this));
}

DVec3 DVec3::Normalized() const
{
	return *this / Length();
}

bool DVec3::IsNormalized(double inTolerance) const 
{ 
	return abs(LengthSq() - 1.0) <= inTolerance; 
}

DVec3 DVec3::GetSign() const
{
#if defined(JPH_USE_AVX)
	__m256d minus_one = _mm256_set1_pd(-1.0);
	__m256d one = _mm256_set1_pd(1.0);
	return _mm256_or_pd(_mm256_and_pd(mValue, minus_one), one);
#else
	return DVec3(std::signbit(mD32[0])? -1.0 : 1.0, 
				 std::signbit(mD32[1])? -1.0 : 1.0, 
				 std::signbit(mD32[2])? -1.0 : 1.0);
#endif
}

JPH_NAMESPACE_END

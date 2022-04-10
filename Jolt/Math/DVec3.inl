// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_AVX2

#include <Jolt/Core/HashCombine.h>

// Create a std::hash for DVec3
JPH_MAKE_HASHABLE(JPH::DVec3, t.GetX(), t.GetY(), t.GetZ())

JPH_NAMESPACE_BEGIN

DVec3::DVec3(Vec3Arg inRHS) : 
	mValue(_mm256_cvtps_pd(inRHS.mValue))
{
}

DVec3::DVec3(double inX, double inY, double inZ) : 
	mValue(_mm256_set_pd(inZ, inZ, inY, inX)) // Assure Z and W are the same
{
}

DVec3 DVec3::sZero()
{
	return _mm256_setzero_pd();
}

DVec3 DVec3::sReplicate(double inV)
{
	return _mm256_set1_pd(inV);
}

DVec3 DVec3::sLoadDouble3Unsafe(const double *inV)
{
	__m256d v = _mm256_loadu_pd(inV);
	return sFixW(v);
}

Vec3 DVec3::ToVec3() const
{
	return _mm256_cvtpd_ps(mValue);
}

DVec3 DVec3::sMin(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_min_pd(inV1.mValue, inV2.mValue);
}

DVec3 DVec3::sMax(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_max_pd(inV1.mValue, inV2.mValue);
}

DVec3 DVec3::sClamp(DVec3Arg inV, DVec3Arg inMin, DVec3Arg inMax)
{
	return sMax(sMin(inV, inMax), inMin);
}

DVec3 DVec3::sEquals(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_EQ_OQ);
}

DVec3 DVec3::sLess(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_LT_OQ);
}

DVec3 DVec3::sLessOrEqual(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_LE_OQ);
}

DVec3 DVec3::sGreater(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_GT_OQ);
}

DVec3 DVec3::sGreaterOrEqual(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_cmp_pd(inV1.mValue, inV2.mValue, _CMP_GE_OQ);
}

DVec3 DVec3::sFusedMultiplyAdd(DVec3Arg inMul1, DVec3Arg inMul2, DVec3Arg inAdd)
{
#ifdef JPH_USE_FMADD
	return _mm256_fmadd_pd(inMul1.mValue, inMul2.mValue, inAdd.mValue);
#else
	return _mm256_add_pd(_mm256_mul_pd(inMul1.mValue, inMul2.mValue), inAdd.mValue);
#endif
}

DVec3 DVec3::sSelect(DVec3Arg inV1, DVec3Arg inV2, DVec3Arg inControl)
{
	return _mm256_blendv_pd(inV1.mValue, inV2.mValue, inControl.mValue);
}

DVec3 DVec3::sOr(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_or_pd(inV1.mValue, inV2.mValue);
}

DVec3 DVec3::sXor(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_xor_pd(inV1.mValue, inV2.mValue);
}

DVec3 DVec3::sAnd(DVec3Arg inV1, DVec3Arg inV2)
{
	return _mm256_and_pd(inV1.mValue, inV2.mValue);
}

int DVec3::GetTrues() const
{
	return _mm256_movemask_pd(mValue);
}

bool DVec3::TestAnyTrue() const
{
	return (_mm256_movemask_pd(mValue) & 0x7) != 0;
}

bool DVec3::TestAllTrue() const
{
	return (_mm256_movemask_pd(mValue) & 0x7) == 0x7;
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
	return _mm256_mul_pd(mValue, inV2.mValue);
}

DVec3 DVec3::operator * (double inV2) const
{
	return _mm256_mul_pd(mValue, _mm256_set1_pd(inV2));
}

DVec3 operator * (double inV1, DVec3Arg inV2)
{
	return _mm256_mul_pd(_mm256_set1_pd(inV1), inV2.mValue);
}

DVec3 DVec3::operator / (double inV2) const
{
	return _mm256_div_pd(mValue, _mm256_set1_pd(inV2));
}

DVec3 &DVec3::operator *= (double inV2)
{
	mValue = _mm256_mul_pd(mValue, _mm256_set1_pd(inV2));
	return *this;
}

DVec3 &DVec3::operator *= (DVec3Arg inV2)
{
	mValue = _mm256_mul_pd(mValue, inV2.mValue);
	return *this;
}

DVec3 &DVec3::operator /= (double inV2)
{
	mValue = _mm256_div_pd(mValue, _mm256_set1_pd(inV2));
	return *this;
}

DVec3 DVec3::operator + (DVec3Arg inV2) const
{
	return _mm256_add_pd(mValue, inV2.mValue);
}

DVec3 &DVec3::operator += (DVec3Arg inV2)
{
	mValue = _mm256_add_pd(mValue, inV2.mValue);
	return *this;
}

DVec3 DVec3::operator - () const
{
	return _mm256_sub_pd(_mm256_setzero_pd(), mValue);
}

DVec3 DVec3::operator - (DVec3Arg inV2) const
{
	return _mm256_sub_pd(mValue, inV2.mValue);
}

DVec3 &DVec3::operator -= (DVec3Arg inV2)
{
	mValue = _mm256_sub_pd(mValue, inV2.mValue);
	return *this;
}

DVec3 DVec3::operator / (DVec3Arg inV2) const
{
	inV2.CheckW();
	return _mm256_div_pd(mValue, inV2.mValue);
}

DVec3 DVec3::Abs() const
{
	return _mm256_max_pd(_mm256_sub_pd(_mm256_setzero_pd(), mValue), mValue);
}

DVec3 DVec3::Reciprocal() const
{
	return sReplicate(1.0) / mValue;
}

DVec3 DVec3::Cross(DVec3Arg inV2) const
{
	__m256d t1 = _mm256_permute4x64_pd(inV2.mValue, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
    t1 = _mm256_mul_pd(t1, mValue);
    __m256d t2 = _mm256_permute4x64_pd(mValue, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
    t2 = _mm256_mul_pd(t2, inV2.mValue);
    __m256d t3 = _mm256_sub_pd(t1, t2);
    return _mm256_permute4x64_pd(t3, _MM_SHUFFLE(0, 0, 2, 1)); // Assure Z and W are the same
}

double DVec3::Dot(DVec3Arg inV2) const
{
	__m256d mul = _mm256_mul_pd(mValue, inV2.mValue);
    __m128d xy = _mm256_castpd256_pd128(mul);
	__m128d yx = _mm_shuffle_pd(xy, xy, 1);
	__m128d sum = _mm_add_pd(xy, yx);
    __m128d zw = _mm256_extractf128_pd(mul, 1);
	sum = _mm_add_pd(sum, zw);
	return _mm_cvtsd_f64(sum);
}

double DVec3::LengthSq() const
{
	return Dot(*this);
}

DVec3 DVec3::Sqrt() const
{
	return _mm256_sqrt_pd(mValue);
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
	__m256d minus_one = _mm256_set1_pd(-1.0);
	__m256d one = _mm256_set1_pd(1.0);
	return _mm256_or_pd(_mm256_and_pd(mValue, minus_one), one);
}

JPH_NAMESPACE_END

#endif // JPH_USE_AVX2

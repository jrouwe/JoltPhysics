// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Math/DVec3.h>

JPH_NAMESPACE_BEGIN

DMat44::DMat44(Vec4Arg inC1, Vec4Arg inC2, Vec4Arg inC3, DVec3Arg inC4) : 
	mCol { inC1, inC2, inC3 },
	mCol3(inC4)
{ 
}

DMat44::DMat44(Type inC1, Type inC2, Type inC3, DTypeArg inC4) : 
	mCol { inC1, inC2, inC3 },
	mCol3(inC4)
{
}

DMat44::DMat44(Mat44Arg inM) :
	mCol { inM.GetColumn4(0), inM.GetColumn4(1), inM.GetColumn4(2) },
	mCol3(inM.GetTranslation())
{
}

DMat44::DMat44(Mat44Arg inRot, DVec3Arg inT) :
	mCol { inRot.GetColumn4(0), inRot.GetColumn4(1), inRot.GetColumn4(2) },
	mCol3(inT)
{
}

DMat44 DMat44::sZero()
{
	return DMat44(Vec4::sZero(), Vec4::sZero(), Vec4::sZero(), DVec3::sZero());
}

DMat44 DMat44::sIdentity()
{
	return DMat44(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), DVec3::sZero());
}

DMat44 DMat44::sInverseRotationTranslation(QuatArg inR, DVec3Arg inT)
{
	Mat44 m = Mat44::sRotation(inR.Conjugated());
	DMat44 dm(m, DVec3::sZero());
	dm.SetTranslation(-dm.Multiply3x3(inT));
	return dm;
}

bool DMat44::operator == (DMat44Arg inM2) const
{
	return mCol[0] == inM2.mCol[0]
		&& mCol[1] == inM2.mCol[1]
		&& mCol[2] == inM2.mCol[2]
		&& mCol3 == inM2.mCol3;
}

bool DMat44::IsClose(DMat44Arg inM2, float inMaxDistSq) const
{
	for (int i = 0; i < 3; ++i)
		if (!mCol[i].IsClose(inM2.mCol[i], inMaxDistSq))
			return false;
	return mCol3.IsClose(inM2.mCol3, double(inMaxDistSq));
}

DMat44 DMat44::operator * (Mat44Arg inM) const
{
	DMat44 result;

	// Rotation part
#if defined(JPH_USE_SSE)
	for (int i = 0; i < 3; ++i)
	{
		__m128 c = inM.GetColumn4(i).mValue;
		__m128 t = _mm_mul_ps(mCol[0].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
		t = _mm_add_ps(t, _mm_mul_ps(mCol[1].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
		t = _mm_add_ps(t, _mm_mul_ps(mCol[2].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))));
		result.mCol[i].mValue = t;
	}
#else
	for (int i = 0; i < 3; ++i)
	{
		Vec4 coli = inM.GetColumn4(i);
		result.mCol[i] = mCol[0] * coli.mF32[0] + mCol[1] * coli.mF32[1] + mCol[2] * coli.mF32[2];
	}
#endif

	// Translation part
#if defined(JPH_USE_AVX)
	__m128 col3 = inM.GetColumn4(3).mValue;
	__m128 t = _mm_mul_ps(mCol[0].mValue, _mm_shuffle_ps(col3, col3, _MM_SHUFFLE(0, 0, 0, 0)));
	t = _mm_add_ps(t, _mm_mul_ps(mCol[1].mValue, _mm_shuffle_ps(col3, col3, _MM_SHUFFLE(1, 1, 1, 1))));
	t = _mm_add_ps(t, _mm_mul_ps(mCol[2].mValue, _mm_shuffle_ps(col3, col3, _MM_SHUFFLE(2, 2, 2, 2))));
	result.mCol3 = DVec3::sFixW(_mm256_add_pd(mCol3.mValue, _mm256_cvtps_pd(t)));
#else
	Vec4 col3 = inM.GetColumn4(3);
	result.mCol3 = mCol3 + Vec3(mCol[0] * col3.mF32[0] + mCol[1] * col3.mF32[1] + mCol[2] * col3.mF32[2]);
#endif

	return result;
}

DMat44 DMat44::operator * (DMat44Arg inM) const
{
	DMat44 result;

	// Rotation part
#if defined(JPH_USE_SSE)
	for (int i = 0; i < 3; ++i)
	{
		__m128 c = inM.mCol[i].mValue;
		__m128 t = _mm_mul_ps(mCol[0].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
		t = _mm_add_ps(t, _mm_mul_ps(mCol[1].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
		t = _mm_add_ps(t, _mm_mul_ps(mCol[2].mValue, _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))));
		result.mCol[i].mValue = t;
	}
#else
	for (int i = 0; i < 3; ++i)
	{
		Vec4 coli = inM.mCol[i];
		result.mCol[i] = mCol[0] * coli.mF32[0] + mCol[1] * coli.mF32[1] + mCol[2] * coli.mF32[2];
	}
#endif // JPH_USE_SSE

	// Translation part
#if defined(JPH_USE_AVX)
	__m256d t = mCol3.mValue;
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[0].mValue), _mm256_set1_pd(inM.mCol3.mF64[0])));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[1].mValue), _mm256_set1_pd(inM.mCol3.mF64[1])));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[2].mValue), _mm256_set1_pd(inM.mCol3.mF64[2])));
	result.mCol3 = DVec3::sFixW(t);
#else
	result.mCol3 = mCol3 + DVec3(mCol[0]) * inM.mCol3.mF64[0] + DVec3(mCol[1]) * inM.mCol3.mF64[1] + DVec3(mCol[2]) * inM.mCol3.mF64[2];
#endif

	return result;
}

DVec3 DMat44::operator * (Vec3Arg inV) const
{
#if defined(JPH_USE_AVX)
	__m128 t = _mm_mul_ps(mCol[0].mValue, _mm_shuffle_ps(inV.mValue, inV.mValue, _MM_SHUFFLE(0, 0, 0, 0)));
	t = _mm_add_ps(t, _mm_mul_ps(mCol[1].mValue, _mm_shuffle_ps(inV.mValue, inV.mValue, _MM_SHUFFLE(1, 1, 1, 1))));
	t = _mm_add_ps(t, _mm_mul_ps(mCol[2].mValue, _mm_shuffle_ps(inV.mValue, inV.mValue, _MM_SHUFFLE(2, 2, 2, 2))));
	return DVec3::sFixW(_mm256_add_pd(mCol3.mValue, _mm256_cvtps_pd(t)));
#else
	return DVec3(
		mCol3.mF64[0] + double(mCol[0].mF32[0] * inV.mF32[0] + mCol[1].mF32[0] * inV.mF32[1] + mCol[2].mF32[0] * inV.mF32[2]), 
		mCol3.mF64[1] + double(mCol[0].mF32[1] * inV.mF32[0] + mCol[1].mF32[1] * inV.mF32[1] + mCol[2].mF32[1] * inV.mF32[2]), 
		mCol3.mF64[2] + double(mCol[0].mF32[2] * inV.mF32[0] + mCol[1].mF32[2] * inV.mF32[1] + mCol[2].mF32[2] * inV.mF32[2]));
#endif
}

DVec3 DMat44::operator * (DVec3Arg inV) const
{
#if defined(JPH_USE_AVX)
	__m256d t = _mm256_add_pd(mCol3.mValue, _mm256_mul_pd(_mm256_cvtps_pd(mCol[0].mValue), _mm256_set1_pd(inV.mF64[0])));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[1].mValue), _mm256_set1_pd(inV.mF64[1])));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[2].mValue), _mm256_set1_pd(inV.mF64[2])));
	return DVec3::sFixW(t);
#else
	return DVec3(
		mCol3.mF64[0] + double(mCol[0].mF32[0]) * inV.mF64[0] + double(mCol[1].mF32[0]) * inV.mF64[1] + double(mCol[2].mF32[0]) * inV.mF64[2], 
		mCol3.mF64[1] + double(mCol[0].mF32[1]) * inV.mF64[0] + double(mCol[1].mF32[1]) * inV.mF64[1] + double(mCol[2].mF32[1]) * inV.mF64[2], 
		mCol3.mF64[2] + double(mCol[0].mF32[2]) * inV.mF64[0] + double(mCol[1].mF32[2]) * inV.mF64[1] + double(mCol[2].mF32[2]) * inV.mF64[2]);
#endif
}

DVec3 DMat44::Multiply3x3(DVec3Arg inV) const
{
#if defined(JPH_USE_AVX)
	__m256d t = _mm256_mul_pd(_mm256_cvtps_pd(mCol[0].mValue), _mm256_set1_pd(inV.mF64[0]));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[1].mValue), _mm256_set1_pd(inV.mF64[1])));
	t = _mm256_add_pd(t, _mm256_mul_pd(_mm256_cvtps_pd(mCol[2].mValue), _mm256_set1_pd(inV.mF64[2])));
	return DVec3::sFixW(t);
#else
	return DVec3(
		double(mCol[0].mF32[0]) * inV.mF64[0] + double(mCol[1].mF32[0]) * inV.mF64[1] + double(mCol[2].mF32[0]) * inV.mF64[2], 
		double(mCol[0].mF32[1]) * inV.mF64[0] + double(mCol[1].mF32[1]) * inV.mF64[1] + double(mCol[2].mF32[1]) * inV.mF64[2], 
		double(mCol[0].mF32[2]) * inV.mF64[0] + double(mCol[1].mF32[2]) * inV.mF64[1] + double(mCol[2].mF32[2]) * inV.mF64[2]);
#endif
}

void DMat44::SetRotation(Mat44Arg inRotation)
{
	mCol[0] = inRotation.GetColumn4(0);
	mCol[1] = inRotation.GetColumn4(1);
	mCol[2] = inRotation.GetColumn4(2);
}

DMat44 DMat44::PreScaled(Vec3Arg inScale) const
{
	return DMat44(inScale.GetX() * mCol[0], inScale.GetY() * mCol[1], inScale.GetZ() * mCol[2], mCol3);
}

DMat44 DMat44::PostScaled(Vec3Arg inScale) const
{
	Vec4 scale(inScale, 1);
	return DMat44(scale * mCol[0], scale * mCol[1], scale * mCol[2], DVec3(scale) * mCol3);
}

DMat44 DMat44::PreTranslated(Vec3Arg inTranslation) const
{
	return DMat44(mCol[0], mCol[1], mCol[2], GetTranslation() + Multiply3x3(inTranslation));
}

DMat44 DMat44::PreTranslated(DVec3Arg inTranslation) const
{
	return DMat44(mCol[0], mCol[1], mCol[2], GetTranslation() + Multiply3x3(inTranslation));
}

DMat44 DMat44::PostTranslated(Vec3Arg inTranslation) const
{
	return DMat44(mCol[0], mCol[1], mCol[2], GetTranslation() + inTranslation);
}

DMat44 DMat44::PostTranslated(DVec3Arg inTranslation) const
{
	return DMat44(mCol[0], mCol[1], mCol[2], GetTranslation() + inTranslation);
}

DMat44 DMat44::Inversed() const
{
	DMat44 m(GetRotation().Inversed3x3());
	m.mCol3 = -m.Multiply3x3(mCol3);
	return m;
}

DMat44 DMat44::InversedRotationTranslation() const
{
	DMat44 m(GetRotation().Transposed3x3());
	m.mCol3 = -m.Multiply3x3(mCol3);
	return m;
}

JPH_NAMESPACE_END

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Math/Swizzle.h>

#ifdef JPH_USE_AVX2 // DVec3 currently uses AVX2 intrinsics but the class is currently unused so we can leave it out (it will be used in the future to support objects at a large distance from the origin)

JPH_NAMESPACE_BEGIN

/// 3 component vector of doubles (stored as 4 vectors). 
/// Note that we keep the 4th component the same as the 3rd component to avoid divisions by zero when JPH_FLOATING_POINT_EXCEPTIONS_ENABLED defined
class [[nodiscard]] DVec3
{
public:
#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
	/// Internal helper function that checks that W is equal to Z, so e.g. dividing by it should not generate div by 0
	JPH_INLINE void				CheckW() const									{ JPH_ASSERT(reinterpret_cast<const uint64 *>(mD32)[2] == reinterpret_cast<const uint64 *>(mD32)[3]); } // Avoid asserts when both components are NaN
	
	/// Internal helper function that ensures that the Z component is replicated to the W component to prevent divisions by zero
	static JPH_INLINE __m256d	sFixW(__m256d inValue)							{ return _mm256_shuffle_pd(inValue, inValue, 2); }
#else
	/// Stub function
	JPH_INLINE void				CheckW() const									{ }
	
	/// Stub function
	static JPH_INLINE __m256d	sFixW(__m256d inValue)							{ return inValue; }
#endif // JPH_FLOATING_POINT_EXCEPTIONS_ENABLED

	/// Constructor
								DVec3() = default; ///< Intentionally not initialized for performance reasons
								DVec3(const DVec3 &inRHS) = default;
	JPH_INLINE explicit			DVec3(Vec3Arg inRHS);
	JPH_INLINE					DVec3(__m256d inRHS) : mValue(inRHS)			{ CheckW(); }

	/// Create a vector from 3 components
	JPH_INLINE					DVec3(double inX, double inY, double inZ);

	/// Vector with all zeros
	static JPH_INLINE DVec3		sZero();

	/// Vectors with the principal axis
	static JPH_INLINE DVec3		sAxisX()										{ return DVec3(1, 0, 0); }
	static JPH_INLINE DVec3		sAxisY()										{ return DVec3(0, 1, 0); }
	static JPH_INLINE DVec3		sAxisZ()										{ return DVec3(0, 0, 1); }

	/// Replicate inV across all components
	static JPH_INLINE DVec3		sReplicate(double inV);
		
	/// Load 3 doubles from memory (reads 64 bits extra which it doesn't use)
	static JPH_INLINE DVec3		sLoadDouble3Unsafe(const double *inV);

	/// Convert to float vector 3
	JPH_INLINE Vec3				ToVec3() const;

	/// Return the minimum value of each of the components
	static JPH_INLINE DVec3		sMin(DVec3Arg inV1, DVec3Arg inV2);

	/// Return the maximum of each of the components
	static JPH_INLINE DVec3		sMax(DVec3Arg inV1, DVec3Arg inV2);

	/// Clamp a vector between min and max (component wise)
	static JPH_INLINE DVec3		sClamp(DVec3Arg inV, DVec3Arg inMin, DVec3Arg inMax);

	/// Equals (component wise)
	static JPH_INLINE DVec3		sEquals(DVec3Arg inV1, DVec3Arg inV2);

	/// Less than (component wise)
	static JPH_INLINE DVec3		sLess(DVec3Arg inV1, DVec3Arg inV2);

	/// Less than or equal (component wise)
	static JPH_INLINE DVec3		sLessOrEqual(DVec3Arg inV1, DVec3Arg inV2);

	/// Greater than (component wise)
	static JPH_INLINE DVec3		sGreater(DVec3Arg inV1, DVec3Arg inV2);

	/// Greater than or equal (component wise)
	static JPH_INLINE DVec3		sGreaterOrEqual(DVec3Arg inV1, DVec3Arg inV2);

	/// Calculates inMul1 * inMul2 + inAdd
	static JPH_INLINE DVec3		sFusedMultiplyAdd(DVec3Arg inMul1, DVec3Arg inMul2, DVec3Arg inAdd);

	/// Component wise select, returns inV1 when highest bit of inControl = 0 and inV2 when highest bit of inControl = 1
	static JPH_INLINE DVec3		sSelect(DVec3Arg inV1, DVec3Arg inV2, DVec3Arg inControl);

	/// Logical or (component wise)
	static JPH_INLINE DVec3		sOr(DVec3Arg inV1, DVec3Arg inV2);

	/// Logical xor (component wise)
	static JPH_INLINE DVec3		sXor(DVec3Arg inV1, DVec3Arg inV2);

	/// Logical and (component wise)
	static JPH_INLINE DVec3		sAnd(DVec3Arg inV1, DVec3Arg inV2);

	/// Store if X is true in bit 0, Y in bit 1, Z in bit 2 and W in bit 3 (true is when highest bit of component is set)
	JPH_INLINE int				GetTrues() const;

	/// Test if any of the components are true (true is when highest bit of component is set)
	JPH_INLINE bool				TestAnyTrue() const;

	/// Test if all components are true (true is when highest bit of component is set)
	JPH_INLINE bool				TestAllTrue() const;

	/// Get individual components
	JPH_INLINE double			GetX() const									{ return _mm_cvtsd_f64(_mm256_castpd256_pd128(mValue)); }
	JPH_INLINE double			GetY() const									{ return mD32[1]; }
	JPH_INLINE double			GetZ() const									{ return mD32[2]; }
	
	/// Set individual components
	JPH_INLINE void				SetX(double inX)								{ mD32[0] = inX; }
	JPH_INLINE void				SetY(double inY)								{ mD32[1] = inY; }
	JPH_INLINE void				SetZ(double inZ)								{ mD32[2] = mD32[3] = inZ; } // Assure Z and W are the same
	
	/// Get double component by index
	JPH_INLINE double			operator [] (uint inCoordinate) const			{ JPH_ASSERT(inCoordinate < 3); return mD32[inCoordinate]; }

	/// Set double component by index
	JPH_INLINE void				SetComponent(uint inCoordinate, double inValue)	{ JPH_ASSERT(inCoordinate < 3); mD32[inCoordinate] = inValue; mValue = sFixW(mValue); } // Assure Z and W are the same

	/// Comparison
	JPH_INLINE bool				operator == (DVec3Arg inV2) const;
	JPH_INLINE bool				operator != (DVec3Arg inV2) const				{ return !(*this == inV2); }

	/// Test if two vectors are close
	JPH_INLINE bool				IsClose(DVec3Arg inV2, double inMaxDistSq = 1.0e-24) const;

	/// Test if vector is near zero
	JPH_INLINE bool				IsNearZero(double inMaxDistSq = 1.0e-24) const;

	/// Test if vector is normalized
	JPH_INLINE bool				IsNormalized(double inTolerance = 1.0e-12) const;

	/// Multiply two double vectors (component wise)
	JPH_INLINE DVec3			operator * (DVec3Arg inV2) const;

	/// Multiply vector with double
	JPH_INLINE DVec3			operator * (double inV2) const;

	/// Multiply vector with double
	friend JPH_INLINE DVec3		operator * (double inV1, DVec3Arg inV2);

	/// Divide vector by double
	JPH_INLINE DVec3			operator / (double inV2) const;

	/// Multiply vector with double
	JPH_INLINE DVec3 &			operator *= (double inV2);

	/// Multiply vector with vector
	JPH_INLINE DVec3 &			operator *= (DVec3Arg inV2);

	/// Divide vector by double
	JPH_INLINE DVec3 &			operator /= (double inV2);

	/// Add two double vectors (component wise)
	JPH_INLINE DVec3			operator + (DVec3Arg inV2) const;

	/// Add two double vectors (component wise)
	JPH_INLINE DVec3 &			operator += (DVec3Arg inV2);

	/// Negate
	JPH_INLINE DVec3			operator - () const;

	/// Subtract two double vectors (component wise)
	JPH_INLINE DVec3			operator - (DVec3Arg inV2) const;

	/// Add two double vectors (component wise)
	JPH_INLINE DVec3 &			operator -= (DVec3Arg inV2);

	/// Divide (component wise)
	JPH_INLINE DVec3			operator / (DVec3Arg inV2) const;

	/// Return the absolute value of each of the components
	JPH_INLINE DVec3			Abs() const;

	/// Reciprocal vector (1 / value) for each of the components
	JPH_INLINE DVec3			Reciprocal() const;

	/// Cross product
	JPH_INLINE DVec3			Cross(DVec3Arg inV2) const;

	/// Dot product
	JPH_INLINE double			Dot(DVec3Arg inV2) const;

	/// Squared length of vector
	JPH_INLINE double			LengthSq() const;

	/// Length of vector
	JPH_INLINE double			Length() const;

	/// Normalize vector
	JPH_INLINE DVec3			Normalized() const;

	/// Component wise square root
	JPH_INLINE DVec3			Sqrt() const;

	/// Get vector that contains the sign of each element (returns 1 if positive, -1 if negative)
	JPH_INLINE DVec3			GetSign() const;

	/// To String
	friend ostream &			operator << (ostream &inStream, DVec3Arg inV)
	{
		inStream << inV.mD32[0] << ", " << inV.mD32[1] << ", " << inV.mD32[2];
		return inStream;
	}

private:
	union
	{
		__m256d					mValue;
		double					mD32[4];
	};
};

static_assert(is_trivial<DVec3>(), "Is supposed to be a trivial type!");

JPH_NAMESPACE_END

#include "DVec3.inl"

#endif // JPH_USE_AVX2

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/BackFaceMode.h>

JPH_NAMESPACE_BEGIN

/// Structure that holds a single ray cast
template <class Vec, class Mat, class RayCastType>
struct RayCastT
{
	JPH_OVERRIDE_NEW_DELETE

	/// Constructors
								RayCastT() = default; // Allow raycast to be created uninitialized
								RayCastT(typename Vec::ArgType inOrigin, Vec3Arg inDirection) : mOrigin(inOrigin), mDirection(inDirection) { }
								RayCastT(const RayCastT<Vec, Mat, RayCastType> &) = default;

	/// Transform this ray using inTransform
	RayCastType					Transformed(typename Mat::ArgType inTransform) const
	{
		Vec ray_origin = inTransform * mOrigin;
		Vec3 ray_direction(inTransform * (mOrigin + mDirection) - ray_origin);
		return { ray_origin, ray_direction };
	}

	/// Get point with fraction inFraction on ray (0 = start of ray, 1 = end of ray)
	inline Vec					GetPointOnRay(float inFraction) const
	{
		return mOrigin + inFraction * mDirection;
	}

	Vec							mOrigin;					///< Origin of the ray
	Vec3						mDirection;					///< Direction and length of the ray (anything beyond this length will not be reported as a hit)
};

struct RayCast : public RayCastT<Vec3, Mat44, RayCast>
{
	using RayCastT<Vec3, Mat44, RayCast>::RayCastT;
};

struct RRayCast : public RayCastT<RVec3, RMat44, RRayCast>
{
	using RayCastT<RVec3, RMat44, RRayCast>::RayCastT;

	/// Convert from RayCast, converts single to double precision
	explicit					RRayCast(const RayCast &inRay) :
		RRayCast(RVec3(inRay.mOrigin), inRay.mDirection)
	{
	}

	/// Convert to RayCast, which implies casting from double precision to single precision
	explicit					operator RayCast() const
	{
		return RayCast(Vec3(mOrigin), mDirection);
	}
};

/// Settings to be passed with a ray cast
class RayCastSettings
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// How backfacing triangles should be treated
	EBackFaceMode				mBackFaceMode				= EBackFaceMode::IgnoreBackFaces;

	/// If convex shapes should be treated as solid. When true, a ray starting inside a convex shape will generate a hit at fraction 0.
	bool						mTreatConvexAsSolid			= true;
};

JPH_NAMESPACE_END

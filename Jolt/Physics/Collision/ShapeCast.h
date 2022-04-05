// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

JPH_NAMESPACE_BEGIN

/// Structure that holds a single shape cast (a shape moving along a linear path in 3d space with no rotation)
struct ShapeCast
{
	/// Constructor
								ShapeCast(const Shape *inShape, Vec3Arg inScale, Mat44Arg inCenterOfMassStart, Vec3Arg inDirection, const AABox &inWorldSpaceBounds) :
		mShape(inShape),
		mScale(inScale),
		mCenterOfMassStart(inCenterOfMassStart),
		mDirection(inDirection),
		mShapeWorldBounds(inWorldSpaceBounds)
	{
	}

	/// Constructor
								ShapeCast(const Shape *inShape, Vec3Arg inScale, Mat44Arg inCenterOfMassStart, Vec3Arg inDirection) :
		ShapeCast(inShape, inScale, inCenterOfMassStart, inDirection, inShape->GetWorldSpaceBounds(inCenterOfMassStart, inScale))
	{
	}

	/// Construct a shape cast using a world transform for a shape instead of a center of mass transform
	static inline ShapeCast		sFromWorldTransform(const Shape *inShape, Vec3Arg inScale, Mat44Arg inWorldTransform, Vec3Arg inDirection)
	{
		return ShapeCast(inShape, inScale, inWorldTransform.PreTranslated(inShape->GetCenterOfMass()), inDirection);
	}

	/// Transform this shape cast using inTransform. Multiply transform on the left left hand side.
	ShapeCast					PostTransformed(Mat44Arg inTransform) const
	{
		Mat44 start = inTransform * mCenterOfMassStart;
		Vec3 direction = inTransform.Multiply3x3(mDirection);
		return { mShape, mScale, start, direction };
	}

	const Shape *				mShape;								///< Shape that's being cast (cannot be mesh shape). Note that this structure does not assume ownership over the shape for performance reasons.
	const Vec3					mScale;								///< Scale in local space of the shape being cast
	const Mat44					mCenterOfMassStart;					///< Start position and orientation of the center of mass of the shape (construct using sFromWorldTransform if you have a world transform for your shape)
	const Vec3					mDirection;							///< Direction and length of the cast (anything beyond this length will not be reported as a hit)
	const AABox					mShapeWorldBounds;					///< Cached shape's world bounds, calculated in constructor
};

/// Settings to be passed with a shape cast
class ShapeCastSettings : public CollideSettingsBase
{
public:
	/// How backfacing triangles should be treated (should we report moving out of a triangle?)
	EBackFaceMode				mBackFaceModeTriangles				= EBackFaceMode::IgnoreBackFaces;

	/// How backfacing convex objects should be treated (should we report starting inside an object and moving out?)
	EBackFaceMode				mBackFaceModeConvex					= EBackFaceMode::IgnoreBackFaces;

	/// Indicates if we want to shrink the shape by the convex radius and then expand it again. This speeds up collision detection and gives a more accurate normal at the cost of a more 'rounded' shape.
	bool						mUseShrunkenShapeAndConvexRadius = false;

	/// When true, and the shape is intersecting at the beginning of the cast (fraction = 0) then this will calculate the deepest penetration point (costing additional CPU time)
	bool						mReturnDeepestPoint = false;
};

/// Result of a shape cast test
class ShapeCastResult : public CollideShapeResult
{
public:
	/// Default constructor
								ShapeCastResult() = default;

	/// Constructor
	/// @param inFraction Fraction at which the cast hit
	/// @param inContactPoint1 Contact point on shape 1
	/// @param inContactPoint2 Contact point on shape 2
	/// @param inContactNormalOrPenetrationDepth Contact normal pointing from shape 1 to 2 or penetration depth vector when the objects are penetrating (also from 1 to 2)
	/// @param inBackFaceHit If this hit was a back face hit
	/// @param inSubShapeID1 Sub shape id for shape 1
	/// @param inSubShapeID2 Sub shape id for shape 2
	/// @param inBodyID2 BodyID that was hit
								ShapeCastResult(float inFraction, Vec3Arg inContactPoint1, Vec3Arg inContactPoint2, Vec3Arg inContactNormalOrPenetrationDepth, bool inBackFaceHit, const SubShapeID &inSubShapeID1, const SubShapeID &inSubShapeID2, const BodyID &inBodyID2) :
		CollideShapeResult(inContactPoint1, inContactPoint2, inContactNormalOrPenetrationDepth, (inContactPoint2 - inContactPoint1).Length(), inSubShapeID1, inSubShapeID2, inBodyID2),
		mFraction(inFraction),
		mIsBackFaceHit(inBackFaceHit)
	{
	}

	/// Function required by the CollisionCollector. A smaller fraction is considered to be a 'better hit'. For rays/cast shapes we can just use the collision fraction. The fraction and penetration depth are combined in such a way that deeper hits at fraction 0 go first.
	inline float				GetEarlyOutFraction() const			{ return mFraction > 0.0f? mFraction : -mPenetrationDepth; }

	float						mFraction;							///< This is the fraction where the shape hit the other shape: CenterOfMassOnHit = Start + value * (End - Start)
	bool						mIsBackFaceHit;						///< True if the shape was hit from the back side
};

JPH_NAMESPACE_END

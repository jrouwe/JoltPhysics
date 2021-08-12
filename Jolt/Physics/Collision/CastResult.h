// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Body/BodyID.h>
#include <Physics/Collision/Shape/SubShapeID.h>

namespace JPH {

/// Structure that holds a ray cast or other object cast hit
class BroadPhaseCastResult
{
public:
	/// Function required by the CollisionCollector. A smaller fraction is considered to be a 'better hit'. For rays/cast shapes we can just use the collision fraction.
	inline float	GetEarlyOutFraction() const			{ return mFraction; }

	BodyID			mBodyID;							///< Body that was hit
	float			mFraction = 1.0f + FLT_EPSILON;		///< Hit fraction of the ray/object [0, 1], HitPoint = Start + mFraction * (End - Start) 
};

/// Specialization of cast result against a shape
class RayCastResult : public BroadPhaseCastResult
{
public:
	SubShapeID		mSubShapeID2;						///< Sub shape ID of shape that we collided against
};

} // JPH
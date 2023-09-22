// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

class Body;
class SoftBodyMotionProperties;

/// Temporary data used by the update of a soft body
class SoftBodyUpdateContext : public NonCopyable
{
public:
	static constexpr uint				cVertexCollisionBatch = 64;					///< Number of vertices to process in a batch in DetermineCollisionPlanes

	// Input
	Body *								mBody;										///< Body that is being updated
	SoftBodyMotionProperties *			mMotionProperties;							///< Motion properties of that body
	RMat44								mCenterOfMassTransform;						///< Transform of the body relative to the soft body
	Vec3								mGravity;									///< Gravity vector in local space of the soft body
	Vec3								mDisplacementDueToGravity;					///< Displacement of the center of mass due to gravity in the current time step
	float								mDeltaTime;									///< Delta time for the current time step
	float								mSubStepDeltaTime;							///< Delta time for each sub step

	// Keeping track of the state of the update
	atomic<uint>						mNextCollisionVertex { 0 };					///< Next vertex to process for DetermineCollisionPlanes
	atomic<uint>						mNumCollisionVerticesProcessed { 0 };		///< Number of vertices processed by DetermineCollisionPlanes, used to determine if we can start simulating
	atomic<bool>						mShouldIterate { true };					///< If true, the body has not had its simulation iterations yet

	// Output
	Vec3								mDeltaPosition;								///< Delta position of the body in the current time step, should be applied after the update
	ECanSleep							mCanSleep;									///< Can the body sleep? Should be applied after the update
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Physics/Body/MotionProperties.h>

JPH_NAMESPACE_BEGIN

class Body;
class SoftBodyMotionProperties;

/// Temporary data used by the update of a soft body
class SoftBodyUpdateContext : public NonCopyable
{
public:
	static constexpr uint				cVertexCollisionBatch = 64;					///< Number of vertices to process in a batch in DetermineCollisionPlanes
	static constexpr uint				cEdgeConstraintBatch = 256;					///< Number of edge constraints to process in a batch in ApplyEdgeConstraints

	// Input
	Body *								mBody;										///< Body that is being updated
	SoftBodyMotionProperties *			mMotionProperties;							///< Motion properties of that body
	RMat44								mCenterOfMassTransform;						///< Transform of the body relative to the soft body
	Vec3								mGravity;									///< Gravity vector in local space of the soft body
	Vec3								mDisplacementDueToGravity;					///< Displacement of the center of mass due to gravity in the current time step
	float								mDeltaTime;									///< Delta time for the current time step
	float								mSubStepDeltaTime;							///< Delta time for each sub step

	/// Describes progress in the current update
	enum class EState
	{
		DetermineCollisionPlanes,													///< Determine collision planes for vertices in parallel
		ApplyEdgeConstraints,														///< Apply edge constraints in parallel
		Done																		///< Update is finished
	};

	/// Construct the edge constraint iterator starting at a new group
	static inline uint64				sGetEdgeGroupStart(uint32 inGroup)
	{
		return uint64(inGroup) << 32;
	}

	/// Get the group and start index from the edge constraint iterator
	static inline void					sGetEdgeGroupAndStartIdx(uint64 inNextEdgeConstraint, uint32 &outGroup, uint32 &outStartIdx)
	{
		outGroup = uint32(inNextEdgeConstraint >> 32);
		outStartIdx = uint32(inNextEdgeConstraint);
	}

	// State of the update
	atomic<EState>						mState { EState::DetermineCollisionPlanes };///< Current state of the update
	atomic<uint>						mNextCollisionVertex { 0 };					///< Next vertex to process for DetermineCollisionPlanes
	atomic<uint>						mNumCollisionVerticesProcessed { 0 };		///< Number of vertices processed by DetermineCollisionPlanes, used to determine if we can start simulating
	atomic<uint>						mNextIteration { 0 };						///< Next simulation iteration to process
	atomic<uint64>						mNextEdgeConstraint { 0 };					///< Next edge constraint group and start index to process
	atomic<uint>						mNumEdgeConstraintsProcessed { 0 };			///< Number of edge constraints processed by ApplyEdgeConstraints, used to determine if we can go to the next group / iteration

	// Output
	Vec3								mDeltaPosition;								///< Delta position of the body in the current time step, should be applied after the update
	ECanSleep							mCanSleep;									///< Can the body sleep? Should be applied after the update
};

JPH_NAMESPACE_END

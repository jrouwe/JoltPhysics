// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Constraints/Constraint.h>
#include <Physics/PhysicsLock.h>
#include <Core/Mutex.h>

namespace JPH {

class IslandBuilder;
class BodyManager;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// A constraint manager manages all constraints of the same type
class ConstraintManager : public NonCopyable
{
public:
	/// Add a new constraint. This is thread safe.
	/// Note that the inConstraints array is allowed to have nullptrs, these will be ignored.
	void					Add(Constraint **inConstraints, int inNumber);

	/// Remove a constraint. This is thread safe.
	/// Note that the inConstraints array is allowed to have nullptrs, these will be ignored.
	void					Remove(Constraint **inConstraint, int inNumber);

	/// Get total number of constraints
	inline uint32			GetNumConstraints() const					{ return (uint32)mConstraints.size(); }

	/// Determine the active constraints of a subset of the constraints
	void					GetActiveConstraints(uint32 inStartConstraintIdx, uint32 inEndConstraintIdx, Constraint **outActiveConstraints, uint32 &outNumActiveConstraints) const;

	/// Link bodies to form islands
	void					BuildIslands(Constraint **inActiveConstraints, uint32 inNumActiveConstraints, IslandBuilder &ioBuilder, BodyManager &inBodyManager);

	/// In order to have a deterministic simulation, we need to sort the constraints of an island before solving them
	void					SortConstraints(Constraint **inActiveConstraints, uint32 *inConstraintIdxBegin, uint32 *inConstraintIdxEnd) const;

	/// Prior to solving the velocity constraints, you must call SetupVelocityConstraints once to precalculate values that are independent of velocity
	void					SetupVelocityConstraints(Constraint **inActiveConstraints, uint32 inNumActiveConstraints, float inDeltaTime);

	/// Same as above, but applies to a limited amount of constraints only
	void					SetupVelocityConstraints(Constraint **inActiveConstraints, const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inDeltaTime);

	/// Apply last frame's impulses, must be called prior to SolveVelocityConstraints
	void					WarmStartVelocityConstraints(Constraint **inActiveConstraints, const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inWarmStartImpulseRatio);

	/// This function is called multiple times to iteratively come to a solution that meets all velocity constraints
	bool					SolveVelocityConstraints(Constraint **inActiveConstraints, const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inDeltaTime);

	/// This function is called multiple times to iteratively come to a solution that meets all position constraints
	bool					SolvePositionConstraints(Constraint **inActiveConstraints, const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inDeltaTime, float inBaumgarte);

#ifdef JPH_STAT_COLLECTOR
	/// Collect stats for all constraints
	void					CollectStats() const;
#endif // JPH_STAT_COLLECTOR

#ifdef JPH_DEBUG_RENDERER
	/// Draw all constraints
	void					DrawConstraints(DebugRenderer *inRenderer) const;

	/// Draw all constraint limits
	void					DrawConstraintLimits(DebugRenderer *inRenderer) const;

	/// Draw all constraint reference frames
	void					DrawConstraintReferenceFrame(DebugRenderer *inRenderer) const;
#endif // JPH_DEBUG_RENDERER

	/// Save state of constraints
	void					SaveState(StateRecorder &inStream) const;

	/// Restore the state of constraints. Returns false if failed.
	bool					RestoreState(StateRecorder &inStream);

	/// Lock all constraints. This should only be done during PhysicsSystem::Update().
	void					LockAllConstraints()						{ PhysicsLock::sLock(mConstraintsMutex, EPhysicsLockTypes::ConstraintsList); }
	void					UnlockAllConstraints()						{ PhysicsLock::sUnlock(mConstraintsMutex, EPhysicsLockTypes::ConstraintsList); }

private:
	using Constraints = vector<Ref<Constraint>>;

	Constraints				mConstraints;
	mutable Mutex			mConstraintsMutex;
};

} // JPH
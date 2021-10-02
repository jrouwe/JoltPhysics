// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Body/BodyInterface.h>
#include <Physics/Collision/NarrowPhaseQuery.h>
#include <Physics/Collision/ContactListener.h>
#include <Physics/Constraints/ContactConstraintManager.h>
#include <Physics/Constraints/ConstraintManager.h>
#include <Physics/IslandBuilder.h>
#include <Physics/PhysicsUpdateContext.h>

namespace JPH {

class JobSystem;
class StateRecorder;
class TempAllocator;
class PhysicsStepListener;

/// The main class for the physics system. It contains all rigid bodies and simulates them.
///
/// The main simulation is performed by the Update() call on multiple threads (if the JobSystem is configured to use them). Please refer to the general architecture overview in the Docs folder for more information.
class PhysicsSystem : public NonCopyable
{
public:
	/// Constructor / Destructor
								PhysicsSystem()												: mContactManager(mPhysicsSettings) { }
								~PhysicsSystem();

	/// Initialize the system.
	/// @param inMaxBodies Maximum number of bodies to support.
	/// @param inNumBodyMutexes Number of body mutexes to use. Should be a power of 2 in the range [1, 64], use 0 to auto detect.
	/// @param inMaxBodyPairs Maximum amount of body pairs to process (anything else will fall through the world), this number should generally be much higher than the max amount of contact points as there will be lots of bodies close that are not actually touching
	/// @param inMaxContactConstraints Maximum amount of contact constraints to process (anything else will fall through the world)
	/// @param inObjectToBroadPhaseLayer Maps object layer to broadphase layer, @see ObjectToBroadPhaseLayer.
	/// @param inObjectLayerPairFilter Filter callback function that is used to determine if two object layers collide.
	void						Init(uint inMaxBodies, uint inNumBodyMutexes, uint inMaxBodyPairs, uint inMaxContactConstraints, const ObjectToBroadPhaseLayer &inObjectToBroadPhaseLayer, ObjectVsBroadPhaseLayerFilter inObjectVsBroadPhaseLayerFilter, ObjectLayerPairFilter inObjectLayerPairFilter);
	
	/// Listener that is notified whenever a body is activated/deactivated
	void						SetBodyActivationListener(BodyActivationListener *inListener) { mBodyManager.SetBodyActivationListener(inListener); }
	BodyActivationListener *	GetBodyActivationListener() const							{ return mBodyManager.GetBodyActivationListener(); }

	/// Listener that is notified whenever a contact point between two bodies is added/updated/removed
	void						SetContactListener(ContactListener *inListener)				{ mContactManager.SetContactListener(inListener); }
	ContactListener *			GetContactListener() const									{ return mContactManager.GetContactListener(); }

	/// Set the function that combines the friction of two bodies and returns it
	/// Default method is the geometric mean: sqrt(friction1 * friction2).
	void						SetCombineFriction(ContactConstraintManager::CombineFunction inCombineFriction) { mContactManager.SetCombineFriction(inCombineFriction); }

	/// Set the function that combines the restitution of two bodies and returns it
	/// Default method is max(restitution1, restitution1)
	void						SetCombineRestitution(ContactConstraintManager::CombineFunction inCombineRestition) { mContactManager.SetCombineRestitution(inCombineRestition); }

	/// Control the main constants of the physics simulation
	void						SetPhysicsSettings(const PhysicsSettings &inSettings)		{ mPhysicsSettings = inSettings; }
	const PhysicsSettings &		GetPhysicsSettings() const									{ return mPhysicsSettings; }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	/// Set function that converts a broadphase layer to a human readable string for debugging purposes
	void						SetBroadPhaseLayerToString(BroadPhaseLayerToString inBroadPhaseLayerToString) { mBroadPhase->SetBroadPhaseLayerToString(inBroadPhaseLayerToString); }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	/// Access to the body interface. This interface allows to to create / remove bodies and to change their properties.
	BodyInterface &				GetBodyInterface() 											{ return mBodyInterfaceLocking; }
	BodyInterface & 			GetBodyInterfaceNoLock()									{ return mBodyInterfaceNoLock; } ///< Version that does not lock the bodies, use with great care!

	/// Access to the broadphase interface that allows coarse collision queries
	const BroadPhaseQuery &		GetBroadPhaseQuery() const									{ return *mBroadPhase; }

	/// Interface that allows fine collision queries against first the broad phase and then the narrow phase.
	const NarrowPhaseQuery &	GetNarrowPhaseQuery() const									{ return mNarrowPhaseQueryLocking; }
	const NarrowPhaseQuery & 	GetNarrowPhaseQueryNoLock() const							{ return mNarrowPhaseQueryNoLock; } ///< Version that does not lock the bodies, use with great care!

	/// Add constraint to the world
	void						AddConstraint(Constraint *inConstraint)						{ mConstraintManager.Add(&inConstraint, 1); }
	
	/// Remove constraint from the world
	void						RemoveConstraint(Constraint *inConstraint)					{ mConstraintManager.Remove(&inConstraint, 1); }

	/// Batch add constraints. Note that the inConstraints array is allowed to have nullptrs, these will be ignored.
	void						AddConstraints(Constraint **inConstraints, int inNumber)	{ mConstraintManager.Add(inConstraints, inNumber); }

	/// Batch remove constraints. Note that the inConstraints array is allowed to have nullptrs, these will be ignored.
	void						RemoveConstraints(Constraint **inConstraints, int inNumber)	{ mConstraintManager.Remove(inConstraints, inNumber); }

	/// Optimize the broadphase, needed only if you've added many bodies prior to calling Update() for the first time.
	void						OptimizeBroadPhase();

	/// Adds a new step listener
	void						AddStepListener(PhysicsStepListener *inListener);

	/// Removes a step listener
	void						RemoveStepListener(PhysicsStepListener *inListener);

	/// Simulate the system.
	/// The world steps for a total of inDeltaTime seconds. This is divided in inCollisionSteps iterations. Each iteration
	/// consists of collision detection followed by inIntegrationSubSteps integration steps.
	void						Update(float inDeltaTime, int inCollisionSteps, int inIntegrationSubSteps, TempAllocator *inTempAllocator, JobSystem *inJobSystem);

#ifdef JPH_STAT_COLLECTOR
	/// Collect stats of the previous time step
	void						CollectStats();
#endif // JPH_STAT_COLLECTOR

	/// Saving state for replay
	void						SaveState(StateRecorder &inStream) const;

	/// Restoring state for replay. Returns false if failed.
	bool						RestoreState(StateRecorder &inStream);

#ifdef JPH_DEBUG_RENDERER
	// Drawing properties
	static bool					sDrawMotionQualityLinearCast;								///< Draw debug info for objects that perform continuous collision detection through the linear cast motion quality

	/// Draw the state of the bodies (debugging purposes)
	void						DrawBodies(const BodyManager::DrawSettings &inSettings, DebugRenderer *inRenderer) { mBodyManager.Draw(inSettings, mPhysicsSettings, inRenderer); }

	/// Draw the constraints only (debugging purposes)
	void						DrawConstraints(DebugRenderer *inRenderer)					{ mConstraintManager.DrawConstraints(inRenderer); }

	/// Draw the constraint limits only (debugging purposes)
	void						DrawConstraintLimits(DebugRenderer *inRenderer)				{ mConstraintManager.DrawConstraintLimits(inRenderer); }

	/// Draw the constraint reference frames only (debugging purposes)
	void						DrawConstraintReferenceFrame(DebugRenderer *inRenderer)	{ mConstraintManager.DrawConstraintReferenceFrame(inRenderer); }
#endif // JPH_DEBUG_RENDERER

	/// Set gravity value
	void						SetGravity(Vec3Arg inGravity)								{ mGravity = inGravity; }
	Vec3		 				GetGravity() const											{ return mGravity; }

	/// Returns a locking interface that won't actually lock the body. Use with great care!
	inline const BodyLockInterfaceNoLock &	GetBodyLockInterfaceNoLock() const				{ return mBodyLockInterfaceNoLock; }

	/// Returns a locking interface that locks the body so other threads cannot modify it.
	inline const BodyLockInterfaceLocking &	GetBodyLockInterface() const					{ return mBodyLockInterfaceLocking; }

	/// Get an broadphase layer filter that uses the default pair filter and a specified object layer to determine if broadphase layers collide
	DefaultBroadPhaseLayerFilter GetDefaultBroadPhaseLayerFilter(ObjectLayer inLayer) const	{ return DefaultBroadPhaseLayerFilter(mObjectVsBroadPhaseLayerFilter, inLayer); }

	/// Get an object layer filter that uses the default pair filter and a specified layer to determine if layers collide
	DefaultObjectLayerFilter	GetDefaultLayerFilter(ObjectLayer inLayer) const			{ return DefaultObjectLayerFilter(mObjectLayerPairFilter, inLayer); }

	/// Gets the current amount of bodies that are in the body manager
	uint						GetNumBodies() const										{ return mBodyManager.GetNumBodies(); }

	/// Gets the current amount of active bodies that are in the body manager
	uint32						GetNumActiveBodies() const									{ return mBodyManager.GetNumActiveBodies(); }

	/// Get the maximum amount of bodies that this physics system supports
	uint						GetMaxBodies() const										{ return mBodyManager.GetMaxBodies(); }

	/// Helper struct that counts the number of bodies of each type
	using BodyStats = BodyManager::BodyStats;

	/// Get stats about the bodies in the body manager (slow, iterates through all bodies)
	BodyStats					GetBodyStats() const										{ return mBodyManager.GetBodyStats(); }

	/// Get copy of the list of all bodies under protection of a lock.
	/// @param outBodyIDs On return, this will contain the list of BodyIDs
	void						GetBodies(BodyIDVector &outBodyIDs) const					{ return mBodyManager.GetBodyIDs(outBodyIDs); }

	/// Get copy of the list of active bodies under protection of a lock.
	/// @param outBodyIDs On return, this will contain the list of BodyIDs
	void						GetActiveBodies(BodyIDVector &outBodyIDs) const				{ return mBodyManager.GetActiveBodies(outBodyIDs); }

#ifdef JPH_TRACK_BROADPHASE_STATS
	/// Trace the accumulated broadphase stats to the TTY
	void						ReportBroadphaseStats()										{ mBroadPhase->ReportStats(); }
#endif // JPH_TRACK_BROADPHASE_STATS

private:
	using CCDBody = PhysicsUpdateContext::SubStep::CCDBody;

	// Various job entry points
	void						JobStepListeners(PhysicsUpdateContext::Step *ioStep);
	void						JobDetermineActiveConstraints(PhysicsUpdateContext::Step *ioStep);
	void						JobApplyGravity(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::Step *ioStep);	
	void						JobSetupVelocityConstraints(float inDeltaTime, PhysicsUpdateContext::Step *ioStep);
	void						JobBuildIslandsFromConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::Step *ioStep);
	void						JobFindCollisions(PhysicsUpdateContext::Step *ioStep, int inJobIndex);
	void						JobFinalizeIslands(PhysicsUpdateContext *ioContext);
	void						JobBodySetIslandIndex(PhysicsUpdateContext *ioContext);
	void						JobSolveVelocityConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep);
	void						JobIntegrateVelocity(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep);
	void						JobFindCCDContacts(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep);
	void						JobResolveCCDContacts(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep);
	void						JobContactRemovedCallbacks();
	void						JobSolvePositionConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep);

	/// Tries to spawn a new FindCollisions job if max concurrency hasn't been reached yet
	void						TrySpawnJobFindCollisions(PhysicsUpdateContext::Step *ioStep);

	/// Process narrow phase for a single body pair
	void						ProcessBodyPair(const BodyPair &inBodyPair);

	/// Number of constraints to process at once in JobDetermineActiveConstraints
	static constexpr int		cDetermineActiveConstraintsBatchSize = 64;

	/// Number of bodies to process at once in JobApplyGravity
	static constexpr int		cApplyGravityBatchSize = 64;

	/// Number of active bodies to test for collisions per batch
	static constexpr int		cActiveBodiesBatchSize = 16;

	/// Number of contacts that need to be queued before another narrow phase job is started
	static constexpr int		cNarrowPhaseBatchSize = 16;

	/// Number of continuous collision shape casts that need to be queued before another job is started
	static constexpr int		cNumCCDBodiesPerJob = 4;

	/// Broadphase layer filter that decides if two objects can collide
	ObjectVsBroadPhaseLayerFilter mObjectVsBroadPhaseLayerFilter = nullptr;

	/// Object layer filter that decides if two objects can collide
	ObjectLayerPairFilter		mObjectLayerPairFilter = nullptr;

	/// The body manager keeps track which bodies are in the simulation
	BodyManager					mBodyManager;

	/// Body locking interfaces
	BodyLockInterfaceNoLock		mBodyLockInterfaceNoLock { mBodyManager };
	BodyLockInterfaceLocking	mBodyLockInterfaceLocking { mBodyManager };

	/// Body interfaces
	BodyInterface				mBodyInterfaceNoLock;
	BodyInterface				mBodyInterfaceLocking;

	/// Narrow phase query interface
	NarrowPhaseQuery			mNarrowPhaseQueryNoLock;
	NarrowPhaseQuery			mNarrowPhaseQueryLocking;

	/// The broadphase does quick collision detection between body pairs
	BroadPhase *				mBroadPhase = nullptr;

	/// The contact manager resolves all contacts during a simulation step
	ContactConstraintManager	mContactManager;

	/// All non-contact constraints
	ConstraintManager			mConstraintManager;

	/// Keeps track of connected bodies and builds islands for multithreaded velocity/position update
	IslandBuilder				mIslandBuilder;

	/// Mutex protecting mStepListeners
	Mutex						mStepListenersMutex;

	/// List of physics step listeners
	using StepListeners = vector<PhysicsStepListener *>;
	StepListeners				mStepListeners;

	/// This is the global gravity vector
	Vec3						mGravity = Vec3(0, -9.81f, 0);

	/// Previous frame's delta time of one sub step to allow scaling previous frame's constraint impulses
	float						mPreviousSubStepDeltaTime = 0.0f;

	/// Simulation settings
	PhysicsSettings				mPhysicsSettings;

#ifdef JPH_STAT_COLLECTOR
	/// Statistics
	alignas(JPH_CACHE_LINE_SIZE) atomic<int> mManifoldsBeforeReduction { 0 };
	alignas(JPH_CACHE_LINE_SIZE) atomic<int> mManifoldsAfterReduction { 0 };
#endif // JPH_STAT_COLLECTOR
};

} // JPH
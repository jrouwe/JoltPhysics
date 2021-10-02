// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/BroadPhase/BroadPhaseQuery.h>
#include <Physics/Collision/BroadPhase/BroadPhaseLayer.h>

// Shorthand function to ifdef out code if broadphase stats tracking is off
#ifdef JPH_TRACK_BROADPHASE_STATS
	#define JPH_IF_TRACK_BROADPHASE_STATS(...) __VA_ARGS__
#else
	#define JPH_IF_TRACK_BROADPHASE_STATS(...)
#endif // JPH_TRACK_BROADPHASE_STATS

namespace JPH {

class BodyManager;
struct BodyPair;

using BodyPairCollector = CollisionCollector<BodyPair, CollisionCollectorTraitsCollideShape>;

/// Used to do coarse collision detection operations to quickly prune out bodies that will not collide.
class BroadPhase : public BroadPhaseQuery
{
public:
	/// Initialize the broadphase.
	/// @param inBodyManager The body manager singleton
	/// @param inObjectToBroadPhaseLayer Maps object layer to broadphase layer, @see ObjectToBroadPhaseLayer. 
	/// Note that the broadphase takes a pointer to the data inside inObjectToBroadPhaseLayer so this object should remain static.
	virtual void		Init(BodyManager *inBodyManager, const ObjectToBroadPhaseLayer &inObjectToBroadPhaseLayer);

	/// Should be called after many objects have been inserted to make the broadphase more efficient, usually done on startup only
	virtual void		Optimize()															{ }

	/// Must be called just before updating the broadphase when none of the body mutexes are locked
	virtual void		FrameSync()															{ }

	/// Must be called before UpdatePrepare to prevent modifications from being made to the tree
	virtual void		LockModifications()													{ }

	/// Context used during broadphase update
	struct UpdateState { void *mData[4]; };

	/// Update the broadphase, needs to be called frequently to update the internal state when bodies have been modified.
	/// The UpdatePrepare() function can run in a background thread without influencing the broadphase
	virtual	UpdateState	UpdatePrepare()														{ return UpdateState(); }

	/// Finalizing the update will quickly apply the changes
	virtual void		UpdateFinalize(UpdateState &inUpdateState)							{ }

	/// Must be called after UpdateFinalize to allow modifications to the broadphase
	virtual void		UnlockModifications()												{ }

	/// Handle used during adding bodies to the broadphase
	using AddState = void *;

	/// Prepare adding inNumber bodies at ioBodies to the broadphase, returns a handle that should be used in AddBodiesFinalize/Abort.
	/// This can be done on a background thread without influencing the broadphase.
	/// ioBodies may be shuffled around by this function and should be kept that way until AddBodiesFinalize/Abort is called.
	virtual AddState	AddBodiesPrepare(BodyID *ioBodies, int inNumber)					{ return nullptr; } // By default the broadphase doesn't support this

	/// Finalize adding bodies to the broadphase, supply the return value of AddBodiesPrepare in inAddState.
	/// Please ensure that the ioBodies array passed to AddBodiesPrepare is unmodified and passed again to this function.
	virtual void		AddBodiesFinalize(BodyID *ioBodies, int inNumber, AddState inAddState) = 0;

	/// Abort adding bodies to the broadphase, supply the return value of AddBodiesPrepare in inAddState.
	/// This can be done on a background thread without influencing the broadphase.
	/// Please ensure that the ioBodies array passed to AddBodiesPrepare is unmodified and passed again to this function.
	virtual void		AddBodiesAbort(BodyID *ioBodies, int inNumber, AddState inAddState)	{ } // By default nothing needs to be done

	/// Remove inNumber bodies in ioBodies from the broadphase.
	/// ioBodies may be shuffled around by this function.
	virtual void		RemoveBodies(BodyID *ioBodies, int inNumber) = 0;

	/// Call whenever the aabb of a body changes (can change order of ioBodies array)
	/// inTakeLock should be false if we're between LockModifications/UnlockModificiations in which case care needs to be taken to not call this between UpdatePrepare/UpdateFinalize
	virtual void		NotifyBodiesAABBChanged(BodyID *ioBodies, int inNumber, bool inTakeLock = true) = 0;

	/// Call whenever the layer (and optionally the aabb as well) of a body changes (can change order of ioBodies array)
	virtual void		NotifyBodiesLayerChanged(BodyID *ioBodies, int inNumber) = 0;

	/// Find all colliding pairs between dynamic bodies
	/// Note that this function is very specifically tailored for the PhysicsSystem::Update function, hence it is not part of the BroadPhaseQuery interface.
	/// One of the assumptions it can make is that no locking is needed during the query as it will only be called during a very particular part of the update.
	/// @param ioActiveBodies is a list of bodies for which we need to find colliding pairs (this function can change the order of the ioActiveBodies array). This can be a subset of the set of active bodies in the system.
	/// @param inNumActiveBodies is the size of the ioActiveBodies array.
	/// @param inSpeculativeContactDistance Distance at which speculative contact points will be created.
	/// @param inObjectVsBroadPhaseLayerFilter is the filter that determines if an object can collide with a broadphase layer.
	/// @param inObjectLayerPairFilter is the filter that determines if two objects can collide.
	/// @param ioPairCollector receives callbacks for every body pair found.
	virtual void		FindCollidingPairs(BodyID *ioActiveBodies, int inNumActiveBodies, float inSpeculativeContactDistance, ObjectVsBroadPhaseLayerFilter inObjectVsBroadPhaseLayerFilter, ObjectLayerPairFilter inObjectLayerPairFilter, BodyPairCollector &ioPairCollector) const = 0;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	/// Set function that converts a broadphase layer to a human readable string for debugging purposes
	virtual void		SetBroadPhaseLayerToString(BroadPhaseLayerToString inBroadPhaseLayerToString) { /* Can be implemented by derived classes */ }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

#ifdef JPH_TRACK_BROADPHASE_STATS
	/// Trace the collected broadphase stats in CSV form.
	/// This report can be used to judge and tweak the efficiency of the broadphase.
	virtual void		ReportStats()														{ /* Can be implemented by derived classes */ }
#endif // JPH_TRACK_BROADPHASE_STATS

protected:
	/// Link to the body manager that manages the bodies in this broadphase
	BodyManager *		mBodyManager = nullptr;
};

} // JPH
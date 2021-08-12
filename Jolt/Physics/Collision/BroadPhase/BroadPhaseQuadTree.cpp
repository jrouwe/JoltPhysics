// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>
#include <Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <Physics/Collision/RayCast.h>
#include <Physics/Collision/AABoxCast.h>
#include <Physics/Collision/CastResult.h>
#include <Physics/PhysicsLock.h>

namespace JPH {

BroadPhaseQuadTree::~BroadPhaseQuadTree()
{
	delete [] mLayers;
}

void BroadPhaseQuadTree::Init(BodyManager* inBodyManager, const ObjectToBroadPhaseLayer &inObjectToBroadPhaseLayer)
{
	BroadPhase::Init(inBodyManager, inObjectToBroadPhaseLayer);

	// Store max bodies
	mMaxBodies = inBodyManager->GetMaxBodies();

	// Initialize tracking data
	mTracking.resize(mMaxBodies);

	// Init allocator
	// Estimate the amount of nodes we're going to need
	uint32 num_leaves = (uint32)(mMaxBodies + 1) / 2; // Assume 50% fill
	uint32 num_leaves_plus_internal_nodes = num_leaves + (num_leaves + 2) / 3; // = Sum(num_leaves * 4^-i) with i = [0, Inf].
	mAllocator.Init(2 * num_leaves_plus_internal_nodes, 256); // We use double the amount of nodes while rebuilding the tree during Update()

	// Determine min and max layers
	BroadPhaseLayer::Type min_layer = (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid, max_layer = 0;
	for (BroadPhaseLayer layer : inObjectToBroadPhaseLayer)
	{
		min_layer = min(min_layer, (BroadPhaseLayer::Type)layer);
		max_layer = max(max_layer, (BroadPhaseLayer::Type)layer);
	}
	JPH_ASSERT(min_layer == 0); // Assume layers start at 0
	JPH_ASSERT(max_layer != (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid); // Assume the invalid layer is unused
	mNumLayers = max_layer + 1;

	// Store reference to mapping table
	mObjectToBroadPhaseLayer = inObjectToBroadPhaseLayer.data();
	mNumObjectLayers = (uint)inObjectToBroadPhaseLayer.size();

	// Init sub trees
	mLayers = new QuadTree [mNumLayers];
	for (uint l = 0; l < mNumLayers; ++l)
		mLayers[l].Init(mAllocator);
}

void BroadPhaseQuadTree::FrameSync()
{
	JPH_PROFILE_FUNCTION();

	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		mLayers[l].DiscardOldTree();
}

void BroadPhaseQuadTree::Optimize()
{
	JPH_PROFILE_FUNCTION();

	FrameSync();

	LockModifications();

	for (uint l = 0; l < mNumLayers; ++l)
	{
		QuadTree &t = mLayers[l];
		if (t.HasBodies() && t.IsDirty())
		{
			QuadTree::UpdateState update_state;
			t.UpdatePrepare(mBodyManager->GetBodies(), mTracking, update_state);
			t.UpdateFinalize(mBodyManager->GetBodies(), mTracking, update_state);
		}
	}

	UnlockModifications();

	mNextLayerToUpdate = 0;
}

void BroadPhaseQuadTree::LockModifications()
{
	// From this point on we prevent modifications to the tree
	PhysicsLock::sLock(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);
}

BroadPhase::UpdateState BroadPhaseQuadTree::UpdatePrepare()
{
	// LockModifications should have been called
	JPH_ASSERT(mUpdateMutex.is_locked());

	// Create update state
	UpdateState update_state;
	UpdateStateImpl *update_state_impl = reinterpret_cast<UpdateStateImpl *>(&update_state);

	// Loop until we've seen all layers
	for (uint iteration = 0; iteration < mNumLayers; ++iteration)
	{
		// Get the layer
		QuadTree &t = mLayers[mNextLayerToUpdate];
		mNextLayerToUpdate = (mNextLayerToUpdate + 1) % mNumLayers;

		// If it is dirty we update this one
		if (t.HasBodies() && t.IsDirty() && t.CanBeUpdated())
		{
			update_state_impl->mTree = &t;
			t.UpdatePrepare(mBodyManager->GetBodies(), mTracking, update_state_impl->mUpdateState);
			return update_state;
		}
	}

	// Nothing to update
	update_state_impl->mTree = nullptr;
	return update_state;
}

void BroadPhaseQuadTree::UpdateFinalize(UpdateState &inUpdateState)
{
	// LockModifications should have been called
	JPH_ASSERT(mUpdateMutex.is_locked());

	// Test if a tree was updated
	UpdateStateImpl *update_state_impl = reinterpret_cast<UpdateStateImpl *>(&inUpdateState);
	if (update_state_impl->mTree == nullptr)
		return;

	update_state_impl->mTree->UpdateFinalize(mBodyManager->GetBodies(), mTracking, update_state_impl->mUpdateState);
}

void BroadPhaseQuadTree::UnlockModifications()
{
	// From this point on we allow modifications to the tree again
	PhysicsLock::sUnlock(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);
}

BroadPhase::AddState BroadPhaseQuadTree::AddBodiesPrepare(BodyID *ioBodies, int inNumber) 
{ 
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inNumber > 0);

	const BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());

	LayerState *state = new LayerState [mNumLayers];

	// Sort bodies on layer
	const BroadPhaseLayer *object_to_broadphase = mObjectToBroadPhaseLayer;
	Body * const * const bodies_ptr = bodies.data(); // C pointer or else sort is incredibly slow in debug mode
	sort(ioBodies, ioBodies + inNumber, [bodies_ptr, object_to_broadphase](BodyID inLHS, BodyID inRHS) -> bool { return object_to_broadphase[bodies_ptr[inLHS.GetIndex()]->GetObjectLayer()] < object_to_broadphase[bodies_ptr[inRHS.GetIndex()]->GetObjectLayer()]; });

	BodyID *b_start = ioBodies, *b_end = ioBodies + inNumber;
	while (b_start < b_end)
	{
		// Get broadphase layer
		ObjectLayer first_body_object_layer = bodies[b_start->GetIndex()]->GetObjectLayer();
		JPH_ASSERT(first_body_object_layer < mNumObjectLayers);
		BroadPhaseLayer::Type broadphase_layer = (BroadPhaseLayer::Type)object_to_broadphase[first_body_object_layer];
		JPH_ASSERT(broadphase_layer < mNumLayers);

		// Find first body with different layer
		BodyID *b_mid = upper_bound(b_start, b_end, broadphase_layer, [bodies_ptr, object_to_broadphase](BroadPhaseLayer::Type inLayer, BodyID inBodyID) -> bool { return inLayer < (BroadPhaseLayer::Type)object_to_broadphase[bodies_ptr[inBodyID.GetIndex()]->GetObjectLayer()]; });

		// Keep track of state for this layer
		LayerState &layer_state = state[broadphase_layer];
		layer_state.mBodyStart = b_start;
		layer_state.mBodyEnd = b_mid;

		// Insert all bodies of the same layer
		mLayers[broadphase_layer].AddBodiesPrepare(bodies, mTracking, b_start, int(b_mid - b_start), layer_state.mAddState);

		// Keep track in which tree we placed the object
		for (BodyID *b = b_start; b < b_mid; ++b)
		{
			uint32 index = b->GetIndex();
			JPH_ASSERT(bodies[index]->GetID() == *b, "Provided BodyID doesn't match BodyID in body manager");
			JPH_ASSERT(!bodies[index]->IsInBroadPhase());
			Tracking &t = mTracking[index];
			JPH_ASSERT(t.mBroadPhaseLayer == (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid);
			t.mBroadPhaseLayer = broadphase_layer;
			JPH_ASSERT(t.mObjectLayer == cObjectLayerInvalid);
			t.mObjectLayer = bodies[index]->GetObjectLayer();
		}

		// Repeat
		b_start = b_mid;
	}

	return state;
}
	
void BroadPhaseQuadTree::AddBodiesFinalize(BodyID *ioBodies, int inNumber, AddState inAddState)
{ 
	JPH_PROFILE_FUNCTION();

	// This cannot run concurrently with UpdatePrepare()/UpdateFinalize()
	SharedLock<SharedMutex> lock(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);

	BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());

	LayerState *state = (LayerState *)inAddState;

	for (BroadPhaseLayer::Type broadphase_layer = 0; broadphase_layer < mNumLayers; broadphase_layer++)
	{
		const LayerState &l = state[broadphase_layer];
		if (l.mBodyStart != nullptr)
		{
			// Insert all bodies of the same layer
			mLayers[broadphase_layer].AddBodiesFinalize(mTracking, int(l.mBodyEnd - l.mBodyStart), l.mAddState);

			// Mark added to broadphase
			for (BodyID *b = l.mBodyStart; b < l.mBodyEnd; ++b)
			{
				uint32 index = b->GetIndex();
				JPH_ASSERT(bodies[index]->GetID() == *b, "Provided BodyID doesn't match BodyID in body manager");
				JPH_ASSERT(mTracking[index].mBroadPhaseLayer == broadphase_layer);
				JPH_ASSERT(mTracking[index].mObjectLayer == bodies[index]->GetObjectLayer());
				JPH_ASSERT(!bodies[index]->IsInBroadPhase());
				bodies[index]->SetInBroadPhaseInternal(true);
			}
		}
	}

	delete [] state;
}
	
void BroadPhaseQuadTree::AddBodiesAbort(BodyID *ioBodies, int inNumber, AddState inAddState)
{ 
	JPH_PROFILE_FUNCTION();

	JPH_IF_ENABLE_ASSERTS(const BodyVector &bodies = mBodyManager->GetBodies();)
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());

	LayerState *state = (LayerState *)inAddState;

	for (BroadPhaseLayer::Type broadphase_layer = 0; broadphase_layer < mNumLayers; broadphase_layer++)
	{
		const LayerState &l = state[broadphase_layer];
		if (l.mBodyStart != nullptr)
		{
			// Insert all bodies of the same layer
			mLayers[broadphase_layer].AddBodiesAbort(mTracking, l.mAddState);

			// Reset bookkeeping
			for (BodyID *b = l.mBodyStart; b < l.mBodyEnd; ++b)
			{
				uint32 index = b->GetIndex();
				JPH_ASSERT(bodies[index]->GetID() == *b, "Provided BodyID doesn't match BodyID in body manager");
				JPH_ASSERT(!bodies[index]->IsInBroadPhase());
				Tracking &t = mTracking[index];
				JPH_ASSERT(t.mBroadPhaseLayer == broadphase_layer);
				t.mBroadPhaseLayer = (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid;
				t.mObjectLayer = cObjectLayerInvalid;
			}
		}
	}

	delete [] state;
}
	
void BroadPhaseQuadTree::RemoveBodies(BodyID *ioBodies, int inNumber) 
{ 
	JPH_PROFILE_FUNCTION();

	// This cannot run concurrently with UpdatePrepare()/UpdateFinalize()
	SharedLock<SharedMutex> lock(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);

	JPH_ASSERT(inNumber > 0);

	BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());

	// Sort bodies on layer
	Tracking *tracking = mTracking.data(); // C pointer or else sort is incredibly slow in debug mode
	sort(ioBodies, ioBodies + inNumber, [tracking](BodyID inLHS, BodyID inRHS) -> bool { return tracking[inLHS.GetIndex()].mBroadPhaseLayer < tracking[inRHS.GetIndex()].mBroadPhaseLayer; });

	BodyID *b_start = ioBodies, *b_end = ioBodies + inNumber;
	while (b_start < b_end)
	{
		// Get broad phase layer
		BroadPhaseLayer::Type broadphase_layer = mTracking[b_start->GetIndex()].mBroadPhaseLayer;
		JPH_ASSERT(broadphase_layer != (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid);

		// Find first body with different layer
		BodyID *b_mid = upper_bound(b_start, b_end, broadphase_layer, [tracking](BroadPhaseLayer::Type inLayer, BodyID inBodyID) -> bool { return inLayer < tracking[inBodyID.GetIndex()].mBroadPhaseLayer; });

		// Remove all bodies of the same layer
		mLayers[broadphase_layer].RemoveBodies(bodies, mTracking, b_start, int(b_mid - b_start));

		for (BodyID *b = b_start; b < b_mid; ++b)
		{
			// Reset bookkeeping
			uint32 index = b->GetIndex();
			Tracking &t = tracking[index];
			t.mBroadPhaseLayer = (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid;
			t.mObjectLayer = cObjectLayerInvalid;

			// Mark removed from broadphase
			JPH_ASSERT(bodies[index]->IsInBroadPhase());
			bodies[index]->SetInBroadPhaseInternal(false);
		}

		// Repeat
		b_start = b_mid;
	}
}

void BroadPhaseQuadTree::NotifyBodiesAABBChanged(BodyID *ioBodies, int inNumber, bool inTakeLock) 
{ 
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inNumber > 0);

	// This cannot run concurrently with UpdatePrepare()/UpdateFinalize()
	if (inTakeLock)
		PhysicsLock::sLockShared(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);
	else
		JPH_ASSERT(mUpdateMutex.is_locked());

	const BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());

	// Sort bodies on layer
	Tracking *tracking = mTracking.data(); // C pointer or else sort is incredibly slow in debug mode
	sort(ioBodies, ioBodies + inNumber, [tracking](BodyID inLHS, BodyID inRHS) -> bool { return tracking[inLHS.GetIndex()].mBroadPhaseLayer < tracking[inRHS.GetIndex()].mBroadPhaseLayer; });

	BodyID *b_start = ioBodies, *b_end = ioBodies + inNumber;
	while (b_start < b_end)
	{
		// Get broadphase layer
		BroadPhaseLayer::Type broadphase_layer = tracking[b_start->GetIndex()].mBroadPhaseLayer;
		JPH_ASSERT(broadphase_layer != (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid);

		// Find first body with different layer
		BodyID *b_mid = upper_bound(b_start, b_end, broadphase_layer, [tracking](BroadPhaseLayer::Type inLayer, BodyID inBodyID) -> bool { return inLayer < tracking[inBodyID.GetIndex()].mBroadPhaseLayer; });

		// Nodify all bodies of the same layer changed
		mLayers[broadphase_layer].NotifyBodiesAABBChanged(bodies, mTracking, b_start, int(b_mid - b_start));

		// Repeat
		b_start = b_mid;
	}

	if (inTakeLock)
		PhysicsLock::sUnlockShared(mUpdateMutex, EPhysicsLockTypes::BroadPhaseUpdate);
}

void BroadPhaseQuadTree::NotifyBodiesLayerChanged(BodyID *ioBodies, int inNumber)
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inNumber > 0);

	// First sort the bodies that actually changed layer to beginning of the array
	const BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());
	for (BodyID *body_id = ioBodies + inNumber - 1; body_id >= ioBodies; --body_id)
	{
		uint32 index = body_id->GetIndex();
		JPH_ASSERT(bodies[index]->GetID() == *body_id, "Provided BodyID doesn't match BodyID in body manager");
		ObjectLayer object_layer = bodies[index]->GetObjectLayer();
		JPH_ASSERT(object_layer < mNumObjectLayers);
		BroadPhaseLayer::Type broadphase_layer = (BroadPhaseLayer::Type)mObjectToBroadPhaseLayer[object_layer];
		JPH_ASSERT(broadphase_layer < mNumLayers);
		if (mTracking[index].mBroadPhaseLayer == broadphase_layer)
		{
			// Update tracking information
			mTracking[index].mObjectLayer = object_layer;

			// If move the body to the end, layer didn't change
			swap(*body_id, ioBodies[inNumber - 1]);
			--inNumber;
		}
	}

	if (inNumber > 0)
	{
		// Changing layer requires us to remove from one tree and add to another, so this is equivalent to removing all bodies first and then adding them again
		RemoveBodies(ioBodies, inNumber);
		AddState add_state = AddBodiesPrepare(ioBodies, inNumber);
		AddBodiesFinalize(ioBodies, inNumber, add_state);
	}
}

void BroadPhaseQuadTree::CastRay(const RayCast &inRay, RayCastBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const 
{ 
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CastRay(inRay, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::CollideAABox(const AABox &inBox, CollideShapeBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const 
{ 
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CollideAABox(inBox, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::CollideSphere(Vec3Arg inCenter, float inRadius, CollideShapeBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CollideSphere(inCenter, inRadius, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::CollidePoint(Vec3Arg inPoint, CollideShapeBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CollidePoint(inPoint, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::CollideOrientedBox(const OrientedBox &inBox, CollideShapeBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CollideOrientedBox(inBox, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::CastAABox(const AABoxCast &inBox, CastShapeBodyCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter) const 
{ 
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Loop over all layers and test the ones that could hit
	for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
		if (inBroadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(l)))
		{
			mLayers[l].CastAABox(inBox, ioCollector, inObjectLayerFilter, mTracking);
			if (ioCollector.ShouldEarlyOut())
				break;
		}
}

void BroadPhaseQuadTree::FindCollidingPairs(BodyID *ioActiveBodies, int inNumActiveBodies, float inSpeculativeContactDistance, BroadPhaseLayerPairFilter inBroadPhaseLayerPairFilter, ObjectLayerPairFilter inObjectLayerPairFilter, BodyPairCollector &ioPairCollector) const 
{ 
	JPH_PROFILE_FUNCTION();

	const BodyVector &bodies = mBodyManager->GetBodies();
	JPH_ASSERT(mMaxBodies == mBodyManager->GetMaxBodies());	

	// Sort bodies on layer
	const Tracking *tracking = mTracking.data(); // C pointer or else sort is incredibly slow in debug mode
	sort(ioActiveBodies, ioActiveBodies + inNumActiveBodies, [tracking](BodyID inLHS, BodyID inRHS) -> bool { return tracking[inLHS.GetIndex()].mBroadPhaseLayer < tracking[inRHS.GetIndex()].mBroadPhaseLayer; });

	BodyID *b_start = ioActiveBodies, *b_end = ioActiveBodies + inNumActiveBodies;
	while (b_start < b_end)
	{
		// Get broadphase layer
		BroadPhaseLayer::Type broadphase_layer = tracking[b_start->GetIndex()].mBroadPhaseLayer;
		JPH_ASSERT(broadphase_layer != (BroadPhaseLayer::Type)cBroadPhaseLayerInvalid);

		// Find first body with different layer
		BodyID *b_mid = upper_bound(b_start, b_end, broadphase_layer, [tracking](BroadPhaseLayer::Type inLayer, BodyID inBodyID) -> bool { return inLayer < tracking[inBodyID.GetIndex()].mBroadPhaseLayer; });

		// Loop over all layers and test the ones that could hit
		for (BroadPhaseLayer::Type l = 0; l < mNumLayers; ++l)
			if (inBroadPhaseLayerPairFilter(BroadPhaseLayer(broadphase_layer), BroadPhaseLayer(l)))
				mLayers[l].FindCollidingPairs(bodies, b_start, int(b_mid - b_start), inSpeculativeContactDistance, ioPairCollector, inObjectLayerPairFilter);

		// Repeat
		b_start = b_mid;
	}
}

} // JPH
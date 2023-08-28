// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/BroadPhase/BroadPhaseInsertionTest.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Geometry/RayAABox.h>
#include <Utils/Log.h>
#include <Utils/DebugRendererSP.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(BroadPhaseInsertionTest)
{
	JPH_ADD_BASE_CLASS(BroadPhaseInsertionTest, BroadPhaseTest)
}

void BroadPhaseInsertionTest::Initialize()
{
	BroadPhaseTest::Initialize();

	CreateBalancedDistribution(mBodyManager, mBodyManager->GetMaxBodies());
}

void BroadPhaseInsertionTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Check if we need to change direction
	if (mDirection == 1 && mCurrentBody >= mBodyManager->GetMaxBodies())
		mDirection = -1;
	if (mDirection == -1 && mCurrentBody == 0)
		mDirection = 1;

	int num_this_step = int(mBodyManager->GetMaxBodies() / 10);

	if (mDirection < 0)
		mCurrentBody -= num_this_step;

	Body **body_vector = mBodyManager->GetBodies().data();

	// Randomly move bodies around
	if (mCurrentBody > 0)
	{
		const int cNumBodiesToMove = 100;
		BodyID *bodies_to_move = new BodyID [cNumBodiesToMove];
		uniform_int_distribution<int> body_selector(0, (int)mCurrentBody - 1);
		uniform_real_distribution<float> translation_selector(1.0f, 5.0f);
		for (int i = 0; i < cNumBodiesToMove; ++i)
		{
			Body &body = *body_vector[body_selector(mRandomGenerator)];
			JPH_ASSERT(body.IsInBroadPhase());
			body.SetPositionAndRotationInternal(body.GetPosition() + translation_selector(mRandomGenerator) * Vec3::sRandom(mRandomGenerator), Quat::sIdentity());
			bodies_to_move[i] = body.GetID();
		}
		mBroadPhase->NotifyBodiesAABBChanged(bodies_to_move, cNumBodiesToMove);
		delete [] bodies_to_move;
	}

	// Create batch of bodies
	BodyID *bodies_to_add_or_remove = new BodyID [num_this_step];
	for (int b = 0; b < num_this_step; ++b)
		bodies_to_add_or_remove[b] = body_vector[mCurrentBody + b]->GetID();

	// Add/remove them
	if (mDirection == 1)
	{
		// Prepare and abort
		BroadPhase::AddState add_state = mBroadPhase->AddBodiesPrepare(bodies_to_add_or_remove, num_this_step);
		mBroadPhase->AddBodiesAbort(bodies_to_add_or_remove, num_this_step, add_state);

		// Prepare and add
		add_state = mBroadPhase->AddBodiesPrepare(bodies_to_add_or_remove, num_this_step);
		mBroadPhase->AddBodiesFinalize(bodies_to_add_or_remove, num_this_step, add_state);
	}
	else
		mBroadPhase->RemoveBodies(bodies_to_add_or_remove, num_this_step);

	// Delete temp array
	delete [] bodies_to_add_or_remove;

	// Create ray
	default_random_engine random;
	Vec3 from = 1000.0f * Vec3::sRandom(random);
	RayCast ray { from, -2.0f * from };

	// Raycast before update
	AllHitCollisionCollector<RayCastBodyCollector> hits_before;
	mBroadPhase->CastRay(ray, hits_before);
	int num_before = (int)hits_before.mHits.size();
	BroadPhaseCastResult *results_before = hits_before.mHits.data();
	Trace("Before update: %d results found", num_before);

	// Draw results
	DrawLineSP(mDebugRenderer, ray.mOrigin, ray.mOrigin + ray.mDirection, Color::sRed);
	for (int i = 0; i < num_before; ++i)
		DrawMarkerSP(mDebugRenderer, ray.GetPointOnRay(results_before[i].mFraction), Color::sGreen, 10.0f);

	// Update the broadphase
	mBroadPhase->Optimize();

	// Raycast after update
	AllHitCollisionCollector<RayCastBodyCollector> hits_after;
	mBroadPhase->CastRay(ray, hits_after);
	int num_after = (int)hits_after.mHits.size();
	BroadPhaseCastResult *results_after = hits_after.mHits.data();
	Trace("After update: %d results found", num_after);

	// Before update we may have some false hits, check that there are less hits after update than before
	if (num_after > num_before)
		FatalError("BroadPhaseInsertionTest: After has more hits than before");
	for (BroadPhaseCastResult *ra = results_after, *ra_end = results_after + num_after; ra < ra_end; ++ra)
	{
		bool found = false;
		for (BroadPhaseCastResult *rb = results_before, *rb_end = results_before + num_before; rb < rb_end; ++rb)
			if (ra->mBodyID == rb->mBodyID)
			{
				found = true;
				break;
			}
		if (!found)
			FatalError("BroadPhaseInsertionTest: Result after not found in result before");
	}

	// Validate with brute force approach
	for (const Body *b : mBodyManager->GetBodies())
	{
		bool found = false;
		for (BroadPhaseCastResult *r = results_after, *r_end = results_after + num_after; r < r_end; ++r)
			if (r->mBodyID == b->GetID())
			{
				found = true;
				break;
			}

		if (b->IsInBroadPhase()
			&& RayAABoxHits(ray.mOrigin, ray.mDirection, b->GetWorldSpaceBounds().mMin, b->GetWorldSpaceBounds().mMax))
		{
			if (!found)
				FatalError("BroadPhaseInsertionTest: Is intersecting but was not found");
		}
		else
		{
			if (found)
				FatalError("BroadPhaseInsertionTest: Is not intersecting but was found");
		}
	}

	if (mDirection > 0)
		mCurrentBody += num_this_step;
}

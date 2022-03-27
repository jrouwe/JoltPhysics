// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/BroadPhase/BroadPhaseCastRayTest.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Renderer/DebugRendererImp.h>
#include <random>

JPH_IMPLEMENT_RTTI_VIRTUAL(BroadPhaseCastRayTest) 
{ 
	JPH_ADD_BASE_CLASS(BroadPhaseCastRayTest, BroadPhaseTest) 
}

void BroadPhaseCastRayTest::Initialize()
{
	BroadPhaseTest::Initialize();

	int num_bodies = int(mBodyManager->GetMaxBodies());

	// Create random boxes
	CreateBalancedDistribution(mBodyManager, num_bodies);

	// Add all bodies to the broadphase
	Body **body_vector = mBodyManager->GetBodies().data();
	BodyID *bodies_to_add = new BodyID [num_bodies];
	for (int b = 0; b < num_bodies; ++b)
		bodies_to_add[b] = body_vector[b]->GetID();
	BroadPhase::AddState add_state = mBroadPhase->AddBodiesPrepare(bodies_to_add, num_bodies);
	mBroadPhase->AddBodiesFinalize(bodies_to_add, num_bodies, add_state);
	delete [] bodies_to_add;

	// Optimize the broadphase
	mBroadPhase->Optimize();
}

void BroadPhaseCastRayTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Create ray
	default_random_engine random;
	Vec3 from = 1000.0f * Vec3::sRandom(random);
	RayCast ray { from, -2.0f * from };

	// Raycast before update
	AllHitCollisionCollector<RayCastBodyCollector> collector;
	mBroadPhase->CastRay(ray, collector);
	int num_hits = (int)collector.mHits.size();
	BroadPhaseCastResult *results = collector.mHits.data();

	// Draw results
	for (int i = 0; i < num_hits; ++i)
		mDebugRenderer->DrawMarker(ray.mOrigin + results[i].mFraction * ray.mDirection, Color::sGreen, 10.0f);
	mDebugRenderer->DrawLine(ray.mOrigin, ray.mOrigin + ray.mDirection, Color::sRed);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/BroadPhase/BroadPhaseTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseBruteForce.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <random>

JPH_IMPLEMENT_RTTI_ABSTRACT(BroadPhaseTest)
{
	JPH_ADD_BASE_CLASS(BroadPhaseTest, Test)
}

#define NUM_BODIES		10000

//#define BROAD_PHASE		BroadPhaseBruteForce()
#define BROAD_PHASE		BroadPhaseQuadTree()

BroadPhaseTest::~BroadPhaseTest()
{
	delete mBroadPhase;
	delete mBodyManager;
}

void BroadPhaseTest::CreateBalancedDistribution(BodyManager *inBodyManager, int inNumBodies, float inEnvironmentSize)
{
	default_random_engine random(0x1ee7c0de);
	uniform_real_distribution<float> zero_to_one(0.0f, 1.0f);
	float n = float(inNumBodies);
	Vec3 max_box_start = Vec3::sReplicate(inEnvironmentSize * (1.0f - pow(n, -1.0f / 3.0f)));
	Vec3 min_box_size = Vec3::sReplicate(1.0f / inEnvironmentSize);
	Vec3 max_box_size = Vec3::sReplicate(inEnvironmentSize * pow(n, -1.0f / 3.0f)) - min_box_size;
	for (int b = 0; b < inNumBodies; ++b)
	{
		AABox box;
		box.mMin = max_box_start * Vec3(zero_to_one(random), zero_to_one(random), zero_to_one(random)) - Vec3::sReplicate(0.5f * inEnvironmentSize);
		box.mMax = box.mMin + min_box_size + max_box_size * Vec3(zero_to_one(random), zero_to_one(random), zero_to_one(random));

		BodyCreationSettings s;
		s.SetShape(new BoxShape(box.GetExtent(), 0.0f));
		s.mPosition = RVec3(box.GetCenter());
		s.mRotation = Quat::sIdentity();
		s.mObjectLayer = (random() % 10) == 0? Layers::MOVING : Layers::NON_MOVING;
		Body *body = inBodyManager->AllocateBody(s);
		inBodyManager->AddBody(body);
	}
}

void BroadPhaseTest::Initialize()
{
	// Create body manager
	mBodyManager = new BodyManager();
	mBodyManager->Init(NUM_BODIES, 0, mBroadPhaseLayerInterface);

	// Crate broadphase
	mBroadPhase = new BROAD_PHASE;
	mBroadPhase->Init(mBodyManager, mBroadPhaseLayerInterface);
}

void BroadPhaseTest::PostPhysicsUpdate(float inDeltaTime)
{
#ifdef JPH_DEBUG_RENDERER
	mBodyManager->Draw(BodyManager::DrawSettings(), PhysicsSettings(), mDebugRenderer);
#endif // JPH_DEBUG_RENDERER
}

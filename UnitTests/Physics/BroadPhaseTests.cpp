// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "Layers.h"

TEST_SUITE("BroadPhaseTests")
{
	TEST_CASE("TestBroadPhaseOptimize")
	{
		BPLayerInterfaceImpl broad_phase_layer_interface; 

		// Create body manager
		BodyManager body_manager;
		body_manager.Init(1, 0, broad_phase_layer_interface);
		
		// Create quad tree
		BroadPhaseQuadTree broadphase;
		broadphase.Init(&body_manager, broad_phase_layer_interface);

		// Create a box
		BodyCreationSettings settings(new BoxShape(Vec3::sReplicate(1.0f)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		Body &body = *body_manager.CreateBody(settings);

		// Add it to the broadphase
		BodyID id = body.GetID();
		BroadPhase::AddState add_state = broadphase.AddBodiesPrepare(&id, 1);
		broadphase.AddBodiesFinalize(&id, 1, add_state);

		// Test that we hit the box at its current location and not where we're going to move it to
		AllHitCollisionCollector<RayCastBodyCollector> collector;
		broadphase.CastRay({ Vec3(0, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
		broadphase.CastRay({ Vec3(2, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());
		broadphase.CastRay({ Vec3(4, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());

		// Move the body
		body.SetPositionAndRotationInternal(Vec3(2, 0, 0), Quat::sIdentity());
		broadphase.NotifyBodiesAABBChanged(&id, 1, true);

		// Test that we hit the box at its previous and current location
		broadphase.CastRay({ Vec3(0, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
		broadphase.CastRay({ Vec3(2, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
		broadphase.CastRay({ Vec3(4, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());

		// Optimize the broadphase
		broadphase.Optimize();

		// Test that we hit the box only at the new location
		broadphase.CastRay({ Vec3(0, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());
		broadphase.CastRay({ Vec3(2, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
		broadphase.CastRay({ Vec3(4, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());

		// Move the body again (so that for the next optimize we'll have to discard a tree)
		body.SetPositionAndRotationInternal(Vec3(4, 0, 0), Quat::sIdentity());
		broadphase.NotifyBodiesAABBChanged(&id, 1, true);

		// Test that we hit the box at its previous and current location
		broadphase.CastRay({ Vec3(0, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());
		broadphase.CastRay({ Vec3(2, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
		broadphase.CastRay({ Vec3(4, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();

		// Optimize the broadphase (this will internally have to discard a tree)
		broadphase.Optimize();

		// Test that we hit the box only at the new location
		broadphase.CastRay({ Vec3(0, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());
		broadphase.CastRay({ Vec3(2, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.empty());
		broadphase.CastRay({ Vec3(4, 2, 0), Vec3(0, -2, 0) }, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
		CHECK(collector.mHits.size() == 1);
		CHECK(collector.mHits[0].mBodyID == id);
		CHECK_APPROX_EQUAL(collector.mHits[0].mFraction, 0.5f);
		collector.Reset();
	}
}

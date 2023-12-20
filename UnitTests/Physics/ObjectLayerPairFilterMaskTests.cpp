// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceMask.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterMask.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterMask.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

TEST_SUITE("ObjectLayerPairFilterMaskTests")
{
	TEST_CASE("ObjectLayerPairFilterMaskTest")
	{
		// Some example layers
		constexpr uint32 FilterDefault = 1;
		constexpr uint32 FilterStatic = 2;
		constexpr uint32 FilterDebris = 4;
		constexpr uint32 FilterSensor = 8;
		constexpr uint32 FilterAll = FilterDefault | FilterStatic | FilterDebris | FilterSensor;

		ObjectLayerPairFilterMask pair_filter;

		ObjectLayer layer1 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault, FilterAll);
		ObjectLayer layer2 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic, FilterAll);
		CHECK(pair_filter.ShouldCollide(layer1, layer2));
		CHECK(pair_filter.ShouldCollide(layer2, layer1));

		layer1 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault, FilterStatic);
		layer2 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic, FilterDefault);
		CHECK(pair_filter.ShouldCollide(layer1, layer2));
		CHECK(pair_filter.ShouldCollide(layer2, layer1));

		layer1 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault, FilterDefault);
		layer2 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic, FilterDefault);
		CHECK(!pair_filter.ShouldCollide(layer1, layer2));
		CHECK(!pair_filter.ShouldCollide(layer2, layer1));

		layer1 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault, FilterStatic);
		layer2 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic, FilterStatic);
		CHECK(!pair_filter.ShouldCollide(layer1, layer2));
		CHECK(!pair_filter.ShouldCollide(layer2, layer1));

		layer1 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault | FilterDebris, FilterAll);
		layer2 = ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic, FilterStatic);
		CHECK(!pair_filter.ShouldCollide(layer1, layer2));
		CHECK(!pair_filter.ShouldCollide(layer2, layer1));

		BroadPhaseLayerInterfaceMask bp_interface(4);
		bp_interface.ConfigureLayer(BroadPhaseLayer(0), FilterDefault, 0); // Default goes to 0
		bp_interface.ConfigureLayer(BroadPhaseLayer(1), FilterStatic, FilterSensor); // Static but not sensor goes to 1
		bp_interface.ConfigureLayer(BroadPhaseLayer(2), FilterStatic, 0); // Everything else static goes to 2
		// Last layer is for everything else

		CHECK(bp_interface.GetBroadPhaseLayer(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault)) == BroadPhaseLayer(0));
		CHECK(bp_interface.GetBroadPhaseLayer(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll)) == BroadPhaseLayer(0));
		CHECK(bp_interface.GetBroadPhaseLayer(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic)) == BroadPhaseLayer(1));
		CHECK(bp_interface.GetBroadPhaseLayer(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic | FilterSensor)) == BroadPhaseLayer(2));
		CHECK(bp_interface.GetBroadPhaseLayer(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDebris)) == BroadPhaseLayer(3));

		ObjectVsBroadPhaseLayerFilterMask bp_filter(bp_interface);

		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterDefault), BroadPhaseLayer(0)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterDefault), BroadPhaseLayer(1)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterDefault), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterDefault), BroadPhaseLayer(3)));

		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterStatic), BroadPhaseLayer(0)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterStatic), BroadPhaseLayer(1)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterStatic), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterStatic), BroadPhaseLayer(3)));

		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterSensor), BroadPhaseLayer(0)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterSensor), BroadPhaseLayer(1)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterSensor), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterAll, FilterSensor), BroadPhaseLayer(3)));
	}

	TEST_CASE("ThreeFloorTest")
	{
		// Define the group bits
		constexpr uint32 GROUP_STATIC = 1;
		constexpr uint32 GROUP_FLOOR1 = 2;
		constexpr uint32 GROUP_FLOOR2 = 4;
		constexpr uint32 GROUP_FLOOR3 = 8;
		constexpr uint32 GROUP_ALL = GROUP_STATIC | GROUP_FLOOR1 | GROUP_FLOOR2 | GROUP_FLOOR3;

		ObjectLayerPairFilterMask pair_filter;

		constexpr uint NUM_BROAD_PHASE_LAYERS = 2;
		BroadPhaseLayer BP_LAYER_STATIC(0);
		BroadPhaseLayer BP_LAYER_DYNAMIC(1);
		BroadPhaseLayerInterfaceMask bp_interface(NUM_BROAD_PHASE_LAYERS);
		bp_interface.ConfigureLayer(BP_LAYER_STATIC, GROUP_STATIC, 0); // Anything that has the static bit set goes into the static broadphase layer
		bp_interface.ConfigureLayer(BP_LAYER_DYNAMIC, GROUP_FLOOR1 | GROUP_FLOOR2 | GROUP_FLOOR3, 0); // Anything that has one of the floor bits set goes into the dynamic broadphase layer

		ObjectVsBroadPhaseLayerFilterMask bp_filter(bp_interface);

		PhysicsSystem system;
		system.Init(1024, 0, 1024, 1024, bp_interface, bp_filter, pair_filter);
		BodyInterface &body_interface = system.GetBodyInterface();

		// Create 3 floors, each colliding with a different group
		RefConst<Shape> floor_shape = new BoxShape(Vec3(10, 0.1f, 10));
		BodyID ground = body_interface.CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(20, 0.1f, 20)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_STATIC, GROUP_FLOOR1)), EActivation::DontActivate);
		BodyID floor1 = body_interface.CreateAndAddBody(BodyCreationSettings(floor_shape, RVec3(0, 2, 0), Quat::sIdentity(), EMotionType::Static, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_STATIC, GROUP_FLOOR1)), EActivation::DontActivate);
		BodyID floor2 = body_interface.CreateAndAddBody(BodyCreationSettings(floor_shape, RVec3(0, 4, 0), Quat::sIdentity(), EMotionType::Static, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_STATIC, GROUP_FLOOR2)), EActivation::DontActivate);
		BodyID floor3 = body_interface.CreateAndAddBody(BodyCreationSettings(floor_shape, RVec3(0, 6, 0), Quat::sIdentity(), EMotionType::Static, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_STATIC, GROUP_FLOOR3)), EActivation::DontActivate);

		// Create dynamic bodies, each colliding with a different floor
		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(0.5f));
		BodyID dynamic_floor1 = body_interface.CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(0, 8, 0), Quat::sIdentity(), EMotionType::Dynamic, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_FLOOR1, GROUP_ALL)), EActivation::Activate);
		BodyID dynamic_floor2 = body_interface.CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(0, 9, 0), Quat::sIdentity(), EMotionType::Dynamic, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_FLOOR2, GROUP_ALL)), EActivation::Activate);
		BodyID dynamic_floor3 = body_interface.CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_FLOOR3, GROUP_ALL)), EActivation::Activate);
		BodyID dynamic_ground = body_interface.CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(15, 8, 0), Quat::sIdentity(), EMotionType::Dynamic, ObjectLayerPairFilterMask::sGetObjectLayer(GROUP_FLOOR1, GROUP_ALL)), EActivation::Activate);

		// Start listening to collision events
		LoggingContactListener listener;
		system.SetContactListener(&listener);

		// Simulate long enough for all objects to fall on the ground
		TempAllocatorImpl allocator(4 * 1024 * 1024);
		JobSystemSingleThreaded job_system(cMaxPhysicsJobs);
		for (int i = 0; i < 100; ++i)
			system.Update(1.0f/ 60.0f, 1, &allocator, &job_system);

		// Dynamic 1 should rest on floor 1
		CHECK(listener.Contains(LoggingContactListener::EType::Add, dynamic_floor1, floor1));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor1, floor2));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor1, floor3));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor1, ground));
		float tolerance = 1.1f * system.GetPhysicsSettings().mPenetrationSlop;
		CHECK_APPROX_EQUAL(body_interface.GetPosition(dynamic_floor1), RVec3(0, 2.6_r, 0), tolerance);

		// Dynamic 2 should rest on floor 2
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor2, floor1));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, dynamic_floor2, floor2));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor2, floor3));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor2, ground));
		CHECK_APPROX_EQUAL(body_interface.GetPosition(dynamic_floor2), RVec3(0, 4.6_r, 0), tolerance);

		// Dynamic 3 should rest on floor 3
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor3, floor1));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor3, floor2));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, dynamic_floor3, floor3));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_floor3, ground));
		CHECK_APPROX_EQUAL(body_interface.GetPosition(dynamic_floor3), RVec3(0, 6.6_r, 0), tolerance);

		// Dynamic 4 should rest on the ground floor
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_ground, floor1));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_ground, floor2));
		CHECK(!listener.Contains(LoggingContactListener::EType::Add, dynamic_ground, floor3));
		CHECK(listener.Contains(LoggingContactListener::EType::Add, dynamic_ground, ground));
		CHECK_APPROX_EQUAL(body_interface.GetPosition(dynamic_ground), RVec3(15, 0.6_r, 0), tolerance);
	}
}

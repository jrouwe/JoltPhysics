// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "Layers.h"
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>

TEST_SUITE("ObjectLayerPairFilterTableTests")
{
	TEST_CASE("ObjectLayerPairFilterTableTest")
	{
		// Init object layers
		ObjectLayerPairFilterTable obj_vs_obj_filter(Layers::NUM_LAYERS);
		obj_vs_obj_filter.EnableCollision(Layers::MOVING, Layers::NON_MOVING);
		obj_vs_obj_filter.EnableCollision(Layers::MOVING, Layers::MOVING);
		obj_vs_obj_filter.EnableCollision(Layers::MOVING, Layers::SENSOR);
		obj_vs_obj_filter.EnableCollision(Layers::LQ_DEBRIS, Layers::NON_MOVING);
		obj_vs_obj_filter.EnableCollision(Layers::HQ_DEBRIS, Layers::NON_MOVING);
		obj_vs_obj_filter.EnableCollision(Layers::HQ_DEBRIS, Layers::MOVING);

		// Check collision pairs
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::NON_MOVING, Layers::NON_MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::NON_MOVING, Layers::MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::NON_MOVING, Layers::HQ_DEBRIS));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::NON_MOVING, Layers::LQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::NON_MOVING, Layers::SENSOR));

		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::MOVING, Layers::NON_MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::MOVING, Layers::MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::MOVING, Layers::HQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::MOVING, Layers::LQ_DEBRIS));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::MOVING, Layers::SENSOR));

		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::HQ_DEBRIS, Layers::NON_MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::HQ_DEBRIS, Layers::MOVING));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::HQ_DEBRIS, Layers::HQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::HQ_DEBRIS, Layers::LQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::HQ_DEBRIS, Layers::SENSOR));

		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::LQ_DEBRIS, Layers::NON_MOVING));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::LQ_DEBRIS, Layers::MOVING));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::LQ_DEBRIS, Layers::HQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::LQ_DEBRIS, Layers::LQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::LQ_DEBRIS, Layers::SENSOR));

		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::SENSOR, Layers::NON_MOVING));
		CHECK(obj_vs_obj_filter.ShouldCollide(Layers::SENSOR, Layers::MOVING));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::SENSOR, Layers::HQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::SENSOR, Layers::LQ_DEBRIS));
		CHECK(!obj_vs_obj_filter.ShouldCollide(Layers::SENSOR, Layers::SENSOR));

		// Init broad phase layers
		BroadPhaseLayerInterfaceTable bp_layer_interface(Layers::NUM_LAYERS, BroadPhaseLayers::NUM_LAYERS);
		bp_layer_interface.MapObjectToBroadPhaseLayer(Layers::NON_MOVING, BroadPhaseLayers::NON_MOVING);
		bp_layer_interface.MapObjectToBroadPhaseLayer(Layers::MOVING, BroadPhaseLayers::MOVING);
		bp_layer_interface.MapObjectToBroadPhaseLayer(Layers::HQ_DEBRIS, BroadPhaseLayers::MOVING);
		bp_layer_interface.MapObjectToBroadPhaseLayer(Layers::LQ_DEBRIS, BroadPhaseLayers::LQ_DEBRIS);
		bp_layer_interface.MapObjectToBroadPhaseLayer(Layers::SENSOR, BroadPhaseLayers::SENSOR);

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		// Set layer names
		bp_layer_interface.SetBroadPhaseLayerName(BroadPhaseLayers::NON_MOVING, "NON_MOVING");
		bp_layer_interface.SetBroadPhaseLayerName(BroadPhaseLayers::MOVING, "MOVING");
		bp_layer_interface.SetBroadPhaseLayerName(BroadPhaseLayers::LQ_DEBRIS, "LQ_DEBRIS");
		bp_layer_interface.SetBroadPhaseLayerName(BroadPhaseLayers::SENSOR, "SENSOR");

		// Check layer name interface
		CHECK(strcmp(bp_layer_interface.GetBroadPhaseLayerName(BroadPhaseLayers::NON_MOVING), "NON_MOVING") == 0);
		CHECK(strcmp(bp_layer_interface.GetBroadPhaseLayerName(BroadPhaseLayers::MOVING), "MOVING") == 0);
		CHECK(strcmp(bp_layer_interface.GetBroadPhaseLayerName(BroadPhaseLayers::LQ_DEBRIS), "LQ_DEBRIS") == 0);
		CHECK(strcmp(bp_layer_interface.GetBroadPhaseLayerName(BroadPhaseLayers::SENSOR), "SENSOR") == 0);
	#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

		// Init object vs broad phase layer filter
		ObjectVsBroadPhaseLayerFilterTable obj_vs_bp_filter(bp_layer_interface, BroadPhaseLayers::NUM_LAYERS, obj_vs_obj_filter, Layers::NUM_LAYERS);

		// Check collision pairs
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::NON_MOVING, BroadPhaseLayers::NON_MOVING));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::NON_MOVING, BroadPhaseLayers::MOVING));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::NON_MOVING, BroadPhaseLayers::LQ_DEBRIS));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::NON_MOVING, BroadPhaseLayers::SENSOR));

		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::MOVING, BroadPhaseLayers::NON_MOVING));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::MOVING, BroadPhaseLayers::MOVING));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::MOVING, BroadPhaseLayers::LQ_DEBRIS));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::MOVING, BroadPhaseLayers::SENSOR));

		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::HQ_DEBRIS, BroadPhaseLayers::NON_MOVING));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::HQ_DEBRIS, BroadPhaseLayers::MOVING));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::HQ_DEBRIS, BroadPhaseLayers::LQ_DEBRIS));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::HQ_DEBRIS, BroadPhaseLayers::SENSOR));

		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::LQ_DEBRIS, BroadPhaseLayers::NON_MOVING));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::LQ_DEBRIS, BroadPhaseLayers::MOVING));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::LQ_DEBRIS, BroadPhaseLayers::LQ_DEBRIS));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::LQ_DEBRIS, BroadPhaseLayers::SENSOR));

		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::SENSOR, BroadPhaseLayers::NON_MOVING));
		CHECK(obj_vs_bp_filter.ShouldCollide(Layers::SENSOR, BroadPhaseLayers::MOVING));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::SENSOR, BroadPhaseLayers::LQ_DEBRIS));
		CHECK(!obj_vs_bp_filter.ShouldCollide(Layers::SENSOR, BroadPhaseLayers::SENSOR));
	}

	TEST_CASE("ObjectLayerPairFilterTableTest2")
	{
		const int n = 10;

		std::pair<ObjectLayer, ObjectLayer> pairs[] = {
			{ ObjectLayer(0), ObjectLayer(0) },
			{ ObjectLayer(9), ObjectLayer(9) },
			{ ObjectLayer(1), ObjectLayer(3) },
			{ ObjectLayer(3), ObjectLayer(1) },
			{ ObjectLayer(5), ObjectLayer(7) },
			{ ObjectLayer(7), ObjectLayer(5) }
		};

		for (auto &p : pairs)
		{
			ObjectLayerPairFilterTable obj_vs_obj_filter(n);
			obj_vs_obj_filter.EnableCollision(p.first, p.second);

			for (ObjectLayer i = 0; i < n; ++i)
				for (ObjectLayer j = 0; j < n; ++j)
				{
					bool should_collide = (i == p.first && j == p.second) || (i == p.second && j == p.first);
					CHECK(obj_vs_obj_filter.ShouldCollide(i, j) == should_collide);
				}
		}
	}
}

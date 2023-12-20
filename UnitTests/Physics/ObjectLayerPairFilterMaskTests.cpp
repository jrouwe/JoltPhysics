// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "Layers.h"
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceMask.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterMask.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterMask.h>

TEST_SUITE("ObjectLayerPairFilterMaskTests")
{
	// Some example layers
	static constexpr uint32 FilterDefault = 1;
	static constexpr uint32 FilterStatic = 2;
	static constexpr uint32 FilterDebris = 4;
	static constexpr uint32 FilterSensor = 8;
	static constexpr uint32 FilterAll = FilterDefault | FilterStatic | FilterDebris | FilterSensor;

	TEST_CASE("ObjectLayerPairFilterMaskTest")
	{
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

		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault), BroadPhaseLayer(0)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault), BroadPhaseLayer(1)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterDefault), BroadPhaseLayer(3)));

		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic), BroadPhaseLayer(0)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic), BroadPhaseLayer(1)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterStatic), BroadPhaseLayer(3)));

		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterSensor), BroadPhaseLayer(0)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterSensor), BroadPhaseLayer(1)));
		CHECK(!bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterSensor), BroadPhaseLayer(2)));
		CHECK(bp_filter.ShouldCollide(ObjectLayerPairFilterMask::sGetObjectLayer(FilterSensor), BroadPhaseLayer(3)));
	}
}

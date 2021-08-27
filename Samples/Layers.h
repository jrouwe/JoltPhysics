// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/ObjectLayer.h>
#include <Physics/Collision/BroadPhase/BroadPhaseLayer.h>

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers
{
	static constexpr uint8 UNUSED1 = 0; // 3 unused values so that broadphase layers values don't match with object layer values (for testing purposes)
	static constexpr uint8 UNUSED2 = 1;
	static constexpr uint8 UNUSED3 = 2;
	static constexpr uint8 NON_MOVING = 3;
	static constexpr uint8 MOVING = 4;
	static constexpr uint8 NUM_LAYERS = 5;
};

/// Function that determines if two object layers can collide
inline bool ObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	switch (inObject1)
	{
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
		return false;
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING;
	case Layers::MOVING:
		return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING;
	default:
		JPH_ASSERT(false);
		return false;
	}
};

/// Broadphase layers
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr BroadPhaseLayer UNUSED(2);
};

/// Function that determines if two broadphase layers can collide
inline bool BroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
		return false;			
	default:
		JPH_ASSERT(false);
		return false;
	}
}

/// Create mapping table from layer to broadphase layer
inline ObjectToBroadPhaseLayer GetObjectToBroadPhaseLayer()
{
	ObjectToBroadPhaseLayer object_to_broadphase;
	object_to_broadphase.resize(Layers::NUM_LAYERS);
	object_to_broadphase[Layers::UNUSED1] = BroadPhaseLayers::UNUSED;
	object_to_broadphase[Layers::UNUSED2] = BroadPhaseLayers::UNUSED;
	object_to_broadphase[Layers::UNUSED3] = BroadPhaseLayers::UNUSED;
	object_to_broadphase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	object_to_broadphase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	return object_to_broadphase;
}

/// Get name of broadphase layer for debugging purposes
inline const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer)
{
	switch ((BroadPhaseLayer::Type)inLayer)
	{
	case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
	case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
	case (BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:		return "UNUSED";
	default:													JPH_ASSERT(false); return "INVALID";
	}
}
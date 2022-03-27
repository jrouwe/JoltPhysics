// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers
{
	static constexpr uint8 UNUSED1 = 0; // 5 unused values so that broadphase layers values don't match with object layer values (for testing purposes)
	static constexpr uint8 UNUSED2 = 1;
	static constexpr uint8 UNUSED3 = 2;
	static constexpr uint8 UNUSED4 = 3;
	static constexpr uint8 UNUSED5 = 4;
	static constexpr uint8 NON_MOVING = 5;
	static constexpr uint8 MOVING = 6;
	static constexpr uint8 HQ_DEBRIS = 7; // High quality debris collides with MOVING and NON_MOVING but not with any debris
	static constexpr uint8 LQ_DEBRIS = 8; // Low quality debris only collides with NON_MOVING
	static constexpr uint8 NUM_LAYERS = 9;
};

/// Function that determines if two object layers can collide
inline bool ObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	switch (inObject1)
	{
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
	case Layers::UNUSED4:
	case Layers::UNUSED5:
		return false;
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING || inObject2 == Layers::HQ_DEBRIS || inObject2 == Layers::LQ_DEBRIS;
	case Layers::MOVING:
		return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING || inObject2 == Layers::HQ_DEBRIS;
	case Layers::HQ_DEBRIS:
		return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING;
	case Layers::LQ_DEBRIS:
		return inObject2 == Layers::NON_MOVING;
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
	static constexpr BroadPhaseLayer LQ_DEBRIS(2);
	static constexpr BroadPhaseLayer UNUSED(3);
	static constexpr uint NUM_LAYERS(4);
};

/// BroadPhaseLayerInterface implementation
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
									BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::UNUSED1] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED2] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED3] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED4] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED5] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		mObjectToBroadPhase[Layers::HQ_DEBRIS] = BroadPhaseLayers::MOVING; // HQ_DEBRIS is also in the MOVING layer as an example on how to map multiple layers onto the same broadphase layer
		mObjectToBroadPhase[Layers::LQ_DEBRIS] = BroadPhaseLayers::LQ_DEBRIS;
	}

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::LQ_DEBRIS:	return "LQ_DEBRIS";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:		return "UNUSED";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Function that determines if two broadphase layers can collide
inline bool BroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
	case Layers::HQ_DEBRIS:
		return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::LQ_DEBRIS:
		return inLayer2 == BroadPhaseLayers::NON_MOVING;
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
	case Layers::UNUSED4:
	case Layers::UNUSED5:
		return false;			
	default:
		JPH_ASSERT(false);
		return false;
	}
}

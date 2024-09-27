// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers
{
	static constexpr ObjectLayer UNUSED1 = 0; // 5 unused values so that broadphase layers values don't match with object layer values (for testing purposes)
	static constexpr ObjectLayer UNUSED2 = 1;
	static constexpr ObjectLayer UNUSED3 = 2;
	static constexpr ObjectLayer UNUSED4 = 3;
	static constexpr ObjectLayer UNUSED5 = 4;
	static constexpr ObjectLayer NON_MOVING = 5;
	static constexpr ObjectLayer MOVING = 6;
	static constexpr ObjectLayer MOVING2 = 7; // Another moving layer that acts as MOVING but doesn't collide with MOVING
	static constexpr ObjectLayer HQ_DEBRIS = 8; // High quality debris collides with MOVING and NON_MOVING but not with any debris
	static constexpr ObjectLayer LQ_DEBRIS = 9; // Low quality debris only collides with NON_MOVING
	static constexpr ObjectLayer SENSOR = 10; // Sensors only collide with MOVING objects
	static constexpr ObjectLayer NUM_LAYERS = 11;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
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
			return inObject2 == Layers::MOVING || inObject2 == Layers::MOVING2 || inObject2 == Layers::HQ_DEBRIS || inObject2 == Layers::LQ_DEBRIS;
		case Layers::MOVING:
			return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING || inObject2 == Layers::HQ_DEBRIS || inObject2 == Layers::SENSOR;
		case Layers::MOVING2:
			return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING2 || inObject2 == Layers::HQ_DEBRIS || inObject2 == Layers::SENSOR;
		case Layers::HQ_DEBRIS:
			return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING || inObject2 == Layers::MOVING2;
		case Layers::LQ_DEBRIS:
			return inObject2 == Layers::NON_MOVING;
		case Layers::SENSOR:
			return inObject2 == Layers::MOVING || inObject2 == Layers::MOVING2;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

/// Broadphase layers
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr BroadPhaseLayer MOVING2(2);
	static constexpr BroadPhaseLayer LQ_DEBRIS(3);
	static constexpr BroadPhaseLayer UNUSED(4);
	static constexpr BroadPhaseLayer SENSOR(5);
	static constexpr uint NUM_LAYERS(6);
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
		mObjectToBroadPhase[Layers::MOVING2] = BroadPhaseLayers::MOVING2;
		mObjectToBroadPhase[Layers::HQ_DEBRIS] = BroadPhaseLayers::MOVING; // HQ_DEBRIS is also in the MOVING layer as an example on how to map multiple layers onto the same broadphase layer
		mObjectToBroadPhase[Layers::LQ_DEBRIS] = BroadPhaseLayers::LQ_DEBRIS;
		mObjectToBroadPhase[Layers::SENSOR] = BroadPhaseLayers::SENSOR;
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
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING2:		return "MOVING2";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::LQ_DEBRIS:	return "LQ_DEBRIS";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:		return "UNUSED";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::SENSOR:		return "SENSOR";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool					ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
		case Layers::HQ_DEBRIS:
			return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::SENSOR;
		case Layers::MOVING2:
			return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING2 || inLayer2 == BroadPhaseLayers::SENSOR;
		case Layers::LQ_DEBRIS:
			return inLayer2 == BroadPhaseLayers::NON_MOVING;
		case Layers::SENSOR:
			return inLayer2 == BroadPhaseLayers::MOVING;
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
};

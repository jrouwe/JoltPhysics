// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/RTTI.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, Skeleton)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, SkeletalAnimation)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, CompoundShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, StaticCompoundShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, MutableCompoundShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, TriangleShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, SphereShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, BoxShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, CapsuleShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, TaperedCapsuleShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, CylinderShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, ScaledShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, MeshShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, ConvexHullShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, HeightFieldShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, RotatedTranslatedShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, OffsetCenterOfMassShapeSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, RagdollSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PointConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, SixDOFConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, SliderConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, SwingTwistConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, DistanceConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, HingeConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, FixedConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, ConeConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PathConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PathConstraintPath)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PathConstraintPathHermite)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, VehicleConstraintSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, WheeledVehicleControllerSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, MotorSettings)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PhysicsScene)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PhysicsMaterial)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, PhysicsMaterialSimple)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, GroupFilter)
JPH_DECLARE_RTTI_WITH_NAMESPACE_FOR_FACTORY(JPH, GroupFilterTable)

JPH_NAMESPACE_BEGIN

void RegisterTypes()
{
	JPH_ASSERT(Factory::sInstance != nullptr, "Need to create a factory first!");

	// Register base classes first so that we can specialize them later
	CompoundShape::sRegister();
	ConvexShape::sRegister();

	// Register compounds before others so that we can specialize them later (register them in reverse order of collision complexity)
	MutableCompoundShape::sRegister();
	StaticCompoundShape::sRegister();

	// Leaf classes
	TriangleShape::sRegister();
	SphereShape::sRegister();
	BoxShape::sRegister();
	CapsuleShape::sRegister();
	TaperedCapsuleShape::sRegister();
	CylinderShape::sRegister();
	MeshShape::sRegister();
	ConvexHullShape::sRegister();
	HeightFieldShape::sRegister();

	// Register these last because their collision functions are simple so we want to execute them first (register them in reverse order of collision complexity)
	RotatedTranslatedShape::sRegister();
	OffsetCenterOfMassShape::sRegister();
	ScaledShape::sRegister();

	// Create list of all types
	const RTTI *types[] = {
		JPH_RTTI(SkeletalAnimation),
		JPH_RTTI(Skeleton),
		JPH_RTTI(CompoundShapeSettings),
		JPH_RTTI(StaticCompoundShapeSettings),
		JPH_RTTI(MutableCompoundShapeSettings),
		JPH_RTTI(TriangleShapeSettings),
		JPH_RTTI(SphereShapeSettings),
		JPH_RTTI(BoxShapeSettings),
		JPH_RTTI(CapsuleShapeSettings),
		JPH_RTTI(TaperedCapsuleShapeSettings),
		JPH_RTTI(CylinderShapeSettings),
		JPH_RTTI(ScaledShapeSettings),
		JPH_RTTI(MeshShapeSettings),
		JPH_RTTI(ConvexHullShapeSettings),
		JPH_RTTI(HeightFieldShapeSettings),
		JPH_RTTI(RotatedTranslatedShapeSettings),
		JPH_RTTI(OffsetCenterOfMassShapeSettings),
		JPH_RTTI(RagdollSettings),
		JPH_RTTI(PointConstraintSettings),
		JPH_RTTI(SixDOFConstraintSettings),
		JPH_RTTI(SliderConstraintSettings),
		JPH_RTTI(SwingTwistConstraintSettings),
		JPH_RTTI(DistanceConstraintSettings),
		JPH_RTTI(HingeConstraintSettings),
		JPH_RTTI(FixedConstraintSettings),
		JPH_RTTI(ConeConstraintSettings),
		JPH_RTTI(PathConstraintSettings),
		JPH_RTTI(VehicleConstraintSettings),
		JPH_RTTI(WheeledVehicleControllerSettings),
		JPH_RTTI(PathConstraintPath),
		JPH_RTTI(PathConstraintPathHermite),
		JPH_RTTI(MotorSettings),
		JPH_RTTI(PhysicsScene),
		JPH_RTTI(PhysicsMaterial),
		JPH_RTTI(PhysicsMaterialSimple),
		JPH_RTTI(GroupFilter),
		JPH_RTTI(GroupFilterTable)
	};

	// Register them all
	Factory::sInstance->Register(types, (uint)size(types));
}

JPH_NAMESPACE_END

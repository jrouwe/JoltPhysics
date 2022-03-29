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

	Factory::sInstance.Register(JPH_RTTI(SkeletalAnimation));
	Factory::sInstance.Register(JPH_RTTI(Skeleton));
	Factory::sInstance.Register(JPH_RTTI(CompoundShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(StaticCompoundShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(MutableCompoundShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(TriangleShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(SphereShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(BoxShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(CapsuleShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(TaperedCapsuleShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(CylinderShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(ScaledShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(MeshShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(ConvexHullShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(HeightFieldShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(RotatedTranslatedShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(OffsetCenterOfMassShapeSettings));
	Factory::sInstance.Register(JPH_RTTI(RagdollSettings));
	Factory::sInstance.Register(JPH_RTTI(PointConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(SixDOFConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(SliderConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(SwingTwistConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(DistanceConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(HingeConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(FixedConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(ConeConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(PathConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(VehicleConstraintSettings));
	Factory::sInstance.Register(JPH_RTTI(WheeledVehicleControllerSettings));
	Factory::sInstance.Register(JPH_RTTI(PathConstraintPath));
	Factory::sInstance.Register(JPH_RTTI(PathConstraintPathHermite));
	Factory::sInstance.Register(JPH_RTTI(MotorSettings));
	Factory::sInstance.Register(JPH_RTTI(PhysicsScene));
	Factory::sInstance.Register(JPH_RTTI(PhysicsMaterial));
	Factory::sInstance.Register(JPH_RTTI(PhysicsMaterialSimple));
	Factory::sInstance.Register(JPH_RTTI(GroupFilter));
	Factory::sInstance.Register(JPH_RTTI(GroupFilterTable));
}

JPH_NAMESPACE_END

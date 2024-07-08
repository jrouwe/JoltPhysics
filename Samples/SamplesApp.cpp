// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <SamplesApp.h>
#include <Application/EntryPoint.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/Geometry/OrientedBox.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Jolt/Physics/Collision/AABoxCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/NarrowPhaseStats.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Constraints/PulleyConstraint.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Utils/Log.h>
#include <Utils/ShapeCreator.h>
#include <Utils/CustomMemoryHook.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

//-----------------------------------------------------------------------------
// RTTI definitions
//-----------------------------------------------------------------------------

struct TestNameAndRTTI
{
	const char *		mName;
	const RTTI *		mRTTI;
};

struct TestCategory
{
	const char *		mName;
	TestNameAndRTTI *	mTests;
	size_t				mNumTests;
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SimpleTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, StackTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, WallTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PyramidTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, IslandTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, FunnelTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, FrictionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, FrictionPerTriangleTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConveyorBeltTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, GravityFactorTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, RestitutionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, DampingTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, KinematicTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ContactManifoldTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ManifoldReductionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CenterOfMassTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, HeavyOnLightTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, HighSpeedTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ChangeMotionQualityTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ChangeMotionTypeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ChangeShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ChangeObjectLayerTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadSaveBinaryTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BigVsSmallTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ActiveEdgesTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, EnhancedInternalEdgeRemovalTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, MultithreadedTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ContactListenerTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ModifyMassTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ActivateDuringUpdateTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SensorTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, DynamicMeshTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, TwoDFunnelTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, AllowedDOFsTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ShapeFilterTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, GyroscopicForceTest)
#ifdef JPH_OBJECT_STREAM
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadSaveSceneTest)
#endif // JPH_OBJECT_STREAM

static TestNameAndRTTI sGeneralTests[] =
{
	{ "Simple",								JPH_RTTI(SimpleTest) },
	{ "Stack",								JPH_RTTI(StackTest) },
	{ "Wall",								JPH_RTTI(WallTest) },
	{ "Pyramid",							JPH_RTTI(PyramidTest) },
	{ "Island",								JPH_RTTI(IslandTest) },
	{ "Funnel",								JPH_RTTI(FunnelTest) },
	{ "2D Funnel",							JPH_RTTI(TwoDFunnelTest) },
	{ "Friction",							JPH_RTTI(FrictionTest) },
	{ "Friction (Per Triangle)",			JPH_RTTI(FrictionPerTriangleTest) },
	{ "Conveyor Belt",						JPH_RTTI(ConveyorBeltTest) },
	{ "Gravity Factor",						JPH_RTTI(GravityFactorTest) },
	{ "Restitution",						JPH_RTTI(RestitutionTest) },
	{ "Damping",							JPH_RTTI(DampingTest) },
	{ "Kinematic",							JPH_RTTI(KinematicTest) },
	{ "Contact Manifold",					JPH_RTTI(ContactManifoldTest) },
	{ "Manifold Reduction",					JPH_RTTI(ManifoldReductionTest) },
	{ "Center Of Mass",						JPH_RTTI(CenterOfMassTest) },
	{ "Heavy On Light",						JPH_RTTI(HeavyOnLightTest) },
	{ "High Speed",							JPH_RTTI(HighSpeedTest) },
	{ "Change Motion Quality",				JPH_RTTI(ChangeMotionQualityTest) },
	{ "Change Motion Type",					JPH_RTTI(ChangeMotionTypeTest) },
	{ "Change Shape",						JPH_RTTI(ChangeShapeTest) },
	{ "Change Object Layer",				JPH_RTTI(ChangeObjectLayerTest) },
#ifdef JPH_OBJECT_STREAM
	{ "Load/Save Scene",					JPH_RTTI(LoadSaveSceneTest) },
#endif // JPH_OBJECT_STREAM
	{ "Load/Save Binary",					JPH_RTTI(LoadSaveBinaryTest) },
	{ "Big vs Small",						JPH_RTTI(BigVsSmallTest) },
	{ "Active Edges",						JPH_RTTI(ActiveEdgesTest) },
	{ "Enhanced Internal Edge Removal",		JPH_RTTI(EnhancedInternalEdgeRemovalTest) },
	{ "Multithreaded",						JPH_RTTI(MultithreadedTest) },
	{ "Contact Listener",					JPH_RTTI(ContactListenerTest) },
	{ "Modify Mass",						JPH_RTTI(ModifyMassTest) },
	{ "Activate During Update",				JPH_RTTI(ActivateDuringUpdateTest) },
	{ "Sensor",								JPH_RTTI(SensorTest) },
	{ "Dynamic Mesh",						JPH_RTTI(DynamicMeshTest) },
	{ "Allowed Degrees of Freedom",			JPH_RTTI(AllowedDOFsTest) },
	{ "Shape Filter",						JPH_RTTI(ShapeFilterTest) },
	{ "Gyroscopic Force",					JPH_RTTI(GyroscopicForceTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, DistanceConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, FixedConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SwingTwistConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SixDOFConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, HingeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PoweredHingeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PointConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SliderConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PoweredSliderConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SpringTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConstraintSingularityTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConstraintPriorityTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PoweredSwingTwistConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SwingTwistConstraintFrictionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PathConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, RackAndPinionConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, GearConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PulleyConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConstraintVsCOMChangeTest)

static TestNameAndRTTI sConstraintTests[] =
{
	{ "Point Constraint",					JPH_RTTI(PointConstraintTest) },
	{ "Distance Constraint",				JPH_RTTI(DistanceConstraintTest) },
	{ "Hinge Constraint",					JPH_RTTI(HingeConstraintTest) },
	{ "Powered Hinge Constraint",			JPH_RTTI(PoweredHingeConstraintTest) },
	{ "Slider Constraint",					JPH_RTTI(SliderConstraintTest) },
	{ "Powered Slider Constraint",			JPH_RTTI(PoweredSliderConstraintTest) },
	{ "Fixed Constraint",					JPH_RTTI(FixedConstraintTest) },
	{ "Cone Constraint",					JPH_RTTI(ConeConstraintTest) },
	{ "Swing Twist Constraint",				JPH_RTTI(SwingTwistConstraintTest) },
	{ "Powered Swing Twist Constraint",		JPH_RTTI(PoweredSwingTwistConstraintTest) },
	{ "Swing Twist Constraint Friction",	JPH_RTTI(SwingTwistConstraintFrictionTest) },
	{ "Six DOF Constraint",					JPH_RTTI(SixDOFConstraintTest) },
	{ "Path Constraint",					JPH_RTTI(PathConstraintTest) },
	{ "Rack And Pinion Constraint",			JPH_RTTI(RackAndPinionConstraintTest) },
	{ "Gear Constraint",					JPH_RTTI(GearConstraintTest) },
	{ "Pulley Constraint",					JPH_RTTI(PulleyConstraintTest) },
	{ "Spring",								JPH_RTTI(SpringTest) },
	{ "Constraint Singularity",				JPH_RTTI(ConstraintSingularityTest) },
	{ "Constraint vs Center Of Mass Change",JPH_RTTI(ConstraintVsCOMChangeTest) },
	{ "Constraint Priority",				JPH_RTTI(ConstraintPriorityTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BoxShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SphereShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, TaperedCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CylinderShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, StaticCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, MutableCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, TriangleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConvexHullShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, MeshShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, HeightFieldShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, DeformedHeightFieldShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, RotatedTranslatedShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, OffsetCenterOfMassShapeTest)

static TestNameAndRTTI sShapeTests[] =
{
	{ "Sphere Shape",						JPH_RTTI(SphereShapeTest) },
	{ "Box Shape",							JPH_RTTI(BoxShapeTest) },
	{ "Capsule Shape",						JPH_RTTI(CapsuleShapeTest) },
	{ "Tapered Capsule Shape",				JPH_RTTI(TaperedCapsuleShapeTest) },
	{ "Cylinder Shape",						JPH_RTTI(CylinderShapeTest) },
	{ "Convex Hull Shape",					JPH_RTTI(ConvexHullShapeTest) },
	{ "Mesh Shape",							JPH_RTTI(MeshShapeTest) },
	{ "Height Field Shape",					JPH_RTTI(HeightFieldShapeTest) },
	{ "Deformed Height Field Shape",		JPH_RTTI(DeformedHeightFieldShapeTest) },
	{ "Static Compound Shape",				JPH_RTTI(StaticCompoundShapeTest) },
	{ "Mutable Compound Shape",				JPH_RTTI(MutableCompoundShapeTest) },
	{ "Triangle Shape",						JPH_RTTI(TriangleShapeTest) },
	{ "Rotated Translated Shape",			JPH_RTTI(RotatedTranslatedShapeTest) },
	{ "Offset Center Of Mass Shape",		JPH_RTTI(OffsetCenterOfMassShapeTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledSphereShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledBoxShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledTaperedCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledCylinderShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledConvexHullShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledMeshShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledHeightFieldShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledStaticCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledMutableCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledTriangleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ScaledOffsetCenterOfMassShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, DynamicScaledShape)

static TestNameAndRTTI sScaledShapeTests[] =
{
	{ "Sphere Shape",						JPH_RTTI(ScaledSphereShapeTest) },
	{ "Box Shape",							JPH_RTTI(ScaledBoxShapeTest) },
	{ "Capsule Shape",						JPH_RTTI(ScaledCapsuleShapeTest) },
	{ "Tapered Capsule Shape",				JPH_RTTI(ScaledTaperedCapsuleShapeTest) },
	{ "Cylinder Shape",						JPH_RTTI(ScaledCylinderShapeTest) },
	{ "Convex Hull Shape",					JPH_RTTI(ScaledConvexHullShapeTest) },
	{ "Mesh Shape",							JPH_RTTI(ScaledMeshShapeTest) },
	{ "Height Field Shape",					JPH_RTTI(ScaledHeightFieldShapeTest) },
	{ "Static Compound Shape",				JPH_RTTI(ScaledStaticCompoundShapeTest) },
	{ "Mutable Compound Shape",				JPH_RTTI(ScaledMutableCompoundShapeTest) },
	{ "Triangle Shape",						JPH_RTTI(ScaledTriangleShapeTest) },
	{ "Offset Center Of Mass Shape",		JPH_RTTI(ScaledOffsetCenterOfMassShapeTest) },
	{ "Dynamic Scaled Shape",				JPH_RTTI(DynamicScaledShape) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CreateRigTest)
#ifdef JPH_OBJECT_STREAM
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, KinematicRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, PoweredRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, RigPileTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadSaveRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadSaveBinaryRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SkeletonMapperTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BigWorldTest)
#endif // JPH_OBJECT_STREAM

static TestNameAndRTTI sRigTests[] =
{
	{ "Create Rig",							JPH_RTTI(CreateRigTest) },
#ifdef JPH_OBJECT_STREAM
	{ "Load Rig",							JPH_RTTI(LoadRigTest) },
	{ "Load / Save Rig",					JPH_RTTI(LoadSaveRigTest) },
	{ "Load / Save Binary Rig",				JPH_RTTI(LoadSaveBinaryRigTest) },
	{ "Kinematic Rig",						JPH_RTTI(KinematicRigTest) },
	{ "Powered Rig",						JPH_RTTI(PoweredRigTest) },
	{ "Skeleton Mapper",					JPH_RTTI(SkeletonMapperTest) },
	{ "Rig Pile",							JPH_RTTI(RigPileTest) },
	{ "Big World",							JPH_RTTI(BigWorldTest) }
#endif // JPH_OBJECT_STREAM
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CharacterTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CharacterVirtualTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CharacterSpaceShipTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CharacterPlanetTest)

static TestNameAndRTTI sCharacterTests[] =
{
	{ "Character",							JPH_RTTI(CharacterTest) },
	{ "Character Virtual",					JPH_RTTI(CharacterVirtualTest) },
	{ "Character Virtual vs Space Ship",	JPH_RTTI(CharacterSpaceShipTest) },
	{ "Character Virtual vs Planet",		JPH_RTTI(CharacterPlanetTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, WaterShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BoatTest)

static TestNameAndRTTI sWaterTests[] =
{
	{ "Shapes",								JPH_RTTI(WaterShapeTest) },
	{ "Boat",								JPH_RTTI(BoatTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, VehicleSixDOFTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, VehicleConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, MotorcycleTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, TankTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, VehicleStressTest)

static TestNameAndRTTI sVehicleTests[] =
{
	{ "Car (VehicleConstraint)",			JPH_RTTI(VehicleConstraintTest) },
	{ "Motorcycle (VehicleConstraint)",		JPH_RTTI(MotorcycleTest) },
	{ "Tank (VehicleConstraint)",			JPH_RTTI(TankTest) },
	{ "Car (SixDOFConstraint)",				JPH_RTTI(VehicleSixDOFTest) },
	{ "Vehicle Stress Test",				JPH_RTTI(VehicleStressTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyShapesTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyFrictionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyRestitutionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyPressureTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyGravityFactorTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyForceTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyKinematicTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyUpdatePositionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyStressTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyVsFastMovingTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyVertexRadiusTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyContactListenerTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyCustomUpdateTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyLRAConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodyBendConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, SoftBodySkinnedConstraintTest)

static TestNameAndRTTI sSoftBodyTests[] =
{
	{ "Soft Body vs Shapes",			JPH_RTTI(SoftBodyShapesTest) },
	{ "Soft Body vs Fast Moving",		JPH_RTTI(SoftBodyVsFastMovingTest) },
	{ "Soft Body Friction",				JPH_RTTI(SoftBodyFrictionTest) },
	{ "Soft Body Restitution",			JPH_RTTI(SoftBodyRestitutionTest) },
	{ "Soft Body Pressure",				JPH_RTTI(SoftBodyPressureTest) },
	{ "Soft Body Gravity Factor",		JPH_RTTI(SoftBodyGravityFactorTest) },
	{ "Soft Body Force",				JPH_RTTI(SoftBodyForceTest) },
	{ "Soft Body Kinematic",			JPH_RTTI(SoftBodyKinematicTest) },
	{ "Soft Body Update Position",		JPH_RTTI(SoftBodyUpdatePositionTest) },
	{ "Soft Body Stress Test",			JPH_RTTI(SoftBodyStressTest) },
	{ "Soft Body Vertex Radius Test",	JPH_RTTI(SoftBodyVertexRadiusTest) },
	{ "Soft Body Contact Listener",		JPH_RTTI(SoftBodyContactListenerTest) },
	{ "Soft Body Custom Update",		JPH_RTTI(SoftBodyCustomUpdateTest) },
	{ "Soft Body LRA Constraint",		JPH_RTTI(SoftBodyLRAConstraintTest) },
	{ "Soft Body Bend Constraint",		JPH_RTTI(SoftBodyBendConstraintTest) },
	{ "Soft Body Skinned Constraint",	JPH_RTTI(SoftBodySkinnedConstraintTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BroadPhaseCastRayTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, BroadPhaseInsertionTest)

static TestNameAndRTTI sBroadPhaseTests[] =
{
	{ "Cast Ray",							JPH_RTTI(BroadPhaseCastRayTest) },
	{ "Insertion",							JPH_RTTI(BroadPhaseInsertionTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, InteractivePairsTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, EPATest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ClosestPointTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConvexHullTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, ConvexHullShrinkTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, RandomRayTest)
JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, CapsuleVsBoxTest)

static TestNameAndRTTI sConvexCollisionTests[] =
{
	{ "Interactive Pairs",					JPH_RTTI(InteractivePairsTest) },
	{ "EPA Test",							JPH_RTTI(EPATest) },
	{ "Closest Point",						JPH_RTTI(ClosestPointTest) },
	{ "Convex Hull",						JPH_RTTI(ConvexHullTest) },
	{ "Convex Hull Shrink",					JPH_RTTI(ConvexHullShrinkTest) },
	{ "Random Ray",							JPH_RTTI(RandomRayTest) },
	{ "Capsule Vs Box",						JPH_RTTI(CapsuleVsBoxTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(JPH_NO_EXPORT, LoadSnapshotTest)

static TestNameAndRTTI sTools[] =
{
	{ "Load Snapshot",						JPH_RTTI(LoadSnapshotTest) },
};

static TestCategory sAllCategories[] =
{
	{ "General", sGeneralTests, size(sGeneralTests) },
	{ "Shapes", sShapeTests, size(sShapeTests) },
	{ "Scaled Shapes", sScaledShapeTests, size(sScaledShapeTests) },
	{ "Constraints", sConstraintTests, size(sConstraintTests) },
	{ "Rig", sRigTests, size(sRigTests) },
	{ "Character", sCharacterTests, size(sCharacterTests) },
	{ "Water", sWaterTests, size(sWaterTests) },
	{ "Vehicle", sVehicleTests, size(sVehicleTests) },
	{ "Soft Body", sSoftBodyTests, size(sSoftBodyTests) },
	{ "Broad Phase", sBroadPhaseTests, size(sBroadPhaseTests) },
	{ "Convex Collision", sConvexCollisionTests, size(sConvexCollisionTests) },
	{ "Tools", sTools, size(sTools) }
};

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
static constexpr uint cNumBodies = 10240;
static constexpr uint cNumBodyMutexes = 0; // Autodetect
static constexpr uint cMaxBodyPairs = 65536;
static constexpr uint cMaxContactConstraints = 20480;

SamplesApp::SamplesApp()
{
	// Limit the render frequency to our simulation frequency so we don't play back the simulation too fast
	// Note that if the simulation frequency > vsync frequency the simulation will slow down as we want
	// to visualize every simulation step. When the simulation frequency is lower than the vsync frequency
	// we will not render a new frame every frame as we want to show the result of the sim and not an interpolated version.
	SetRenderFrequency(mUpdateFrequency);

	// Allocate temp memory
#ifdef JPH_DISABLE_TEMP_ALLOCATOR
	mTempAllocator = new TempAllocatorMalloc();
#else
	mTempAllocator = new TempAllocatorImpl(32 * 1024 * 1024);
#endif

	// Create job system
	mJobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, mMaxConcurrentJobs - 1);

	// Create single threaded job system for validating
	mJobSystemValidating = new JobSystemSingleThreaded(cMaxPhysicsJobs);

	{
		// Disable allocation checking
		DisableCustomMemoryHook dcmh;

		// Create UI
		UIElement *main_menu = mDebugUI->CreateMenu();
		mDebugUI->CreateTextButton(main_menu, "Select Test", [this]() {
			UIElement *tests = mDebugUI->CreateMenu();
			for (TestCategory &c : sAllCategories)
			{
				mDebugUI->CreateTextButton(tests, c.mName, [this, &c]() {
					UIElement *category = mDebugUI->CreateMenu();
					for (uint j = 0; j < c.mNumTests; ++j)
						mDebugUI->CreateTextButton(category, c.mTests[j].mName, [this, &c, j]() { StartTest(c.mTests[j].mRTTI); });
					mDebugUI->ShowMenu(category);
				});
			}
			mDebugUI->ShowMenu(tests);
		});
		mTestSettingsButton = mDebugUI->CreateTextButton(main_menu, "Test Settings", [this](){
			UIElement *test_settings = mDebugUI->CreateMenu();
			mTest->CreateSettingsMenu(mDebugUI, test_settings);
			mDebugUI->ShowMenu(test_settings);
		});
		mDebugUI->CreateTextButton(main_menu, "Restart Test (R)", [this]() { StartTest(mTestClass); });
		mDebugUI->CreateTextButton(main_menu, "Run All Tests", [this]() { RunAllTests(); });
		mNextTestButton = mDebugUI->CreateTextButton(main_menu, "Next Test (N)", [this]() { NextTest(); });
		mNextTestButton->SetDisabled(true);
		mDebugUI->CreateTextButton(main_menu, "Take Snapshot", [this]() { TakeSnapshot(); });
		mDebugUI->CreateTextButton(main_menu, "Take And Reload Snapshot", [this]() { TakeAndReloadSnapshot(); });
		mDebugUI->CreateTextButton(main_menu, "Physics Settings", [this]() {
			UIElement *phys_settings = mDebugUI->CreateMenu();
			mDebugUI->CreateSlider(phys_settings, "Max Concurrent Jobs", float(mMaxConcurrentJobs), 1, float(thread::hardware_concurrency()), 1, [this](float inValue) { mMaxConcurrentJobs = (int)inValue; });
			mDebugUI->CreateSlider(phys_settings, "Gravity (m/s^2)", -mPhysicsSystem->GetGravity().GetY(), 0.0f, 20.0f, 1.0f, [this](float inValue) { mPhysicsSystem->SetGravity(Vec3(0, -inValue, 0)); });
			mDebugUI->CreateSlider(phys_settings, "Update Frequency (Hz)", mUpdateFrequency, 7.5f, 300.0f, 2.5f, [this](float inValue) { mUpdateFrequency = inValue; SetRenderFrequency(mUpdateFrequency); });
			mDebugUI->CreateSlider(phys_settings, "Num Collision Steps", float(mCollisionSteps), 1.0f, 4.0f, 1.0f, [this](float inValue) { mCollisionSteps = int(inValue); });
			mDebugUI->CreateSlider(phys_settings, "Num Velocity Steps", float(mPhysicsSettings.mNumVelocitySteps), 0, 30, 1, [this](float inValue) { mPhysicsSettings.mNumVelocitySteps = int(round(inValue)); mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateSlider(phys_settings, "Num Position Steps", float(mPhysicsSettings.mNumPositionSteps), 0, 30, 1, [this](float inValue) { mPhysicsSettings.mNumPositionSteps = int(round(inValue)); mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateSlider(phys_settings, "Baumgarte Stabilization Factor", mPhysicsSettings.mBaumgarte, 0.01f, 1.0f, 0.05f, [this](float inValue) { mPhysicsSettings.mBaumgarte = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateSlider(phys_settings, "Speculative Contact Distance (m)", mPhysicsSettings.mSpeculativeContactDistance, 0.0f, 0.1f, 0.005f, [this](float inValue) { mPhysicsSettings.mSpeculativeContactDistance = inValue; });
			mDebugUI->CreateSlider(phys_settings, "Penetration Slop (m)", mPhysicsSettings.mPenetrationSlop, 0.0f, 0.1f, 0.005f, [this](float inValue) { mPhysicsSettings.mPenetrationSlop = inValue; });
			mDebugUI->CreateSlider(phys_settings, "Linear Cast Threshold", mPhysicsSettings.mLinearCastThreshold, 0.0f, 1.0f, 0.05f, [this](float inValue) { mPhysicsSettings.mLinearCastThreshold = inValue; });
			mDebugUI->CreateSlider(phys_settings, "Min Velocity For Restitution (m/s)", mPhysicsSettings.mMinVelocityForRestitution, 0.0f, 10.0f, 0.1f, [this](float inValue) { mPhysicsSettings.mMinVelocityForRestitution = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateSlider(phys_settings, "Time Before Sleep (s)", mPhysicsSettings.mTimeBeforeSleep, 0.1f, 1.0f, 0.1f, [this](float inValue) { mPhysicsSettings.mTimeBeforeSleep = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateSlider(phys_settings, "Point Velocity Sleep Threshold (m/s)", mPhysicsSettings.mPointVelocitySleepThreshold, 0.01f, 1.0f, 0.01f, [this](float inValue) { mPhysicsSettings.mPointVelocitySleepThreshold = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		#if defined(_DEBUG) && !defined(JPH_DISABLE_CUSTOM_ALLOCATOR) && !defined(JPH_COMPILER_MINGW)
			mDebugUI->CreateCheckBox(phys_settings, "Enable Checking Memory Hook", IsCustomMemoryHookEnabled(), [](UICheckBox::EState inState) { EnableCustomMemoryHook(inState == UICheckBox::STATE_CHECKED); });
		#endif
			mDebugUI->CreateCheckBox(phys_settings, "Deterministic Simulation", mPhysicsSettings.mDeterministicSimulation, [this](UICheckBox::EState inState) { mPhysicsSettings.mDeterministicSimulation = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Constraint Warm Starting", mPhysicsSettings.mConstraintWarmStart, [this](UICheckBox::EState inState) { mPhysicsSettings.mConstraintWarmStart = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Use Body Pair Contact Cache", mPhysicsSettings.mUseBodyPairContactCache, [this](UICheckBox::EState inState) { mPhysicsSettings.mUseBodyPairContactCache = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Contact Manifold Reduction", mPhysicsSettings.mUseManifoldReduction, [this](UICheckBox::EState inState) { mPhysicsSettings.mUseManifoldReduction = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Use Large Island Splitter", mPhysicsSettings.mUseLargeIslandSplitter, [this](UICheckBox::EState inState) { mPhysicsSettings.mUseLargeIslandSplitter = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Allow Sleeping", mPhysicsSettings.mAllowSleeping, [this](UICheckBox::EState inState) { mPhysicsSettings.mAllowSleeping = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Check Active Triangle Edges", mPhysicsSettings.mCheckActiveEdges, [this](UICheckBox::EState inState) { mPhysicsSettings.mCheckActiveEdges = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
			mDebugUI->CreateCheckBox(phys_settings, "Record State For Playback", mRecordState, [this](UICheckBox::EState inState) { mRecordState = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(phys_settings, "Check Determinism", mCheckDeterminism, [this](UICheckBox::EState inState) { mCheckDeterminism = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(phys_settings, "Install Contact Listener", mInstallContactListener, [this](UICheckBox::EState inState) { mInstallContactListener = inState == UICheckBox::STATE_CHECKED; StartTest(mTestClass); });
			mDebugUI->ShowMenu(phys_settings);
		});
	#ifdef JPH_DEBUG_RENDERER
		mDebugUI->CreateTextButton(main_menu, "Drawing Options", [this]() {
			UIElement *drawing_options = mDebugUI->CreateMenu();
			mDebugUI->CreateCheckBox(drawing_options, "Draw Shapes (H)", mBodyDrawSettings.mDrawShape, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawShape = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Shapes Wireframe (Alt+W)", mBodyDrawSettings.mDrawShapeWireframe, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawShapeWireframe = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateComboBox(drawing_options, "Draw Shape Color", { "Instance", "Shape Type", "Motion Type", "Sleep", "Island", "Material" }, (int)mBodyDrawSettings.mDrawShapeColor, [this](int inItem) { mBodyDrawSettings.mDrawShapeColor = (BodyManager::EShapeColor)inItem; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw GetSupport + Cvx Radius (Shift+H)", mBodyDrawSettings.mDrawGetSupportFunction, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawGetSupportFunction = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Shapes Using GetTrianglesStart/Next (Alt+H)", mDrawGetTriangles, [this](UICheckBox::EState inState) { mDrawGetTriangles = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw GetSupport Direction", mBodyDrawSettings.mDrawSupportDirection, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSupportDirection = inState == UICheckBox::STATE_CHECKED; mBodyDrawSettings.mDrawGetSupportFunction |= mBodyDrawSettings.mDrawSupportDirection; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw GetSupportingFace (Shift+F)", mBodyDrawSettings.mDrawGetSupportingFace, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawGetSupportingFace = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Constraints (C)", mDrawConstraints, [this](UICheckBox::EState inState) { mDrawConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Constraint Limits (L)", mDrawConstraintLimits, [this](UICheckBox::EState inState) { mDrawConstraintLimits = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Constraint Reference Frame", mDrawConstraintReferenceFrame, [this](UICheckBox::EState inState) { mDrawConstraintReferenceFrame = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Contact Point (1)", ContactConstraintManager::sDrawContactPoint, [](UICheckBox::EState inState) { ContactConstraintManager::sDrawContactPoint = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Supporting Faces (2)", ContactConstraintManager::sDrawSupportingFaces, [](UICheckBox::EState inState) { ContactConstraintManager::sDrawSupportingFaces = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Contact Point Reduction (3)", ContactConstraintManager::sDrawContactPointReduction, [](UICheckBox::EState inState) { ContactConstraintManager::sDrawContactPointReduction = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Contact Manifolds (M)", ContactConstraintManager::sDrawContactManifolds, [](UICheckBox::EState inState) { ContactConstraintManager::sDrawContactManifolds = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Motion Quality Linear Cast", PhysicsSystem::sDrawMotionQualityLinearCast, [](UICheckBox::EState inState) { PhysicsSystem::sDrawMotionQualityLinearCast = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Bounding Boxes", mBodyDrawSettings.mDrawBoundingBox, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawBoundingBox = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Physics System Bounds", mDrawPhysicsSystemBounds, [this](UICheckBox::EState inState) { mDrawPhysicsSystemBounds = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Center of Mass Transforms", mBodyDrawSettings.mDrawCenterOfMassTransform, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawCenterOfMassTransform = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw World Transforms", mBodyDrawSettings.mDrawWorldTransform, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawWorldTransform = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Velocity", mBodyDrawSettings.mDrawVelocity, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawVelocity = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Sleep Stats", mBodyDrawSettings.mDrawSleepStats, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSleepStats = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Mass and Inertia (I)", mBodyDrawSettings.mDrawMassAndInertia, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawMassAndInertia = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Joints", mPoseDrawSettings.mDrawJoints, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJoints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Joint Orientations", mPoseDrawSettings.mDrawJointOrientations, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJointOrientations = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Joint Names", mPoseDrawSettings.mDrawJointNames, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJointNames = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Convex Hull Shape Face Outlines", ConvexHullShape::sDrawFaceOutlines, [](UICheckBox::EState inState) { ConvexHullShape::sDrawFaceOutlines = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Mesh Shape Triangle Groups", MeshShape::sDrawTriangleGroups, [](UICheckBox::EState inState) { MeshShape::sDrawTriangleGroups = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Mesh Shape Triangle Outlines", MeshShape::sDrawTriangleOutlines, [](UICheckBox::EState inState) { MeshShape::sDrawTriangleOutlines = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Height Field Shape Triangle Outlines", HeightFieldShape::sDrawTriangleOutlines, [](UICheckBox::EState inState) { HeightFieldShape::sDrawTriangleOutlines = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Submerged Volumes", Shape::sDrawSubmergedVolumes, [](UICheckBox::EState inState) { Shape::sDrawSubmergedVolumes = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Character Virtual Constraints", CharacterVirtual::sDrawConstraints, [](UICheckBox::EState inState) { CharacterVirtual::sDrawConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Character Virtual Walk Stairs", CharacterVirtual::sDrawWalkStairs, [](UICheckBox::EState inState) { CharacterVirtual::sDrawWalkStairs = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Character Virtual Stick To Floor", CharacterVirtual::sDrawStickToFloor, [](UICheckBox::EState inState) { CharacterVirtual::sDrawStickToFloor = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Vertices", mBodyDrawSettings.mDrawSoftBodyVertices, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyVertices = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Vertex Velocities", mBodyDrawSettings.mDrawSoftBodyVertexVelocities, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyVertexVelocities = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Edge Constraints", mBodyDrawSettings.mDrawSoftBodyEdgeConstraints, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyEdgeConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Bend Constraints", mBodyDrawSettings.mDrawSoftBodyBendConstraints, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyBendConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Volume Constraints", mBodyDrawSettings.mDrawSoftBodyVolumeConstraints, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyVolumeConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Skin Constraints", mBodyDrawSettings.mDrawSoftBodySkinConstraints, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodySkinConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body LRA Constraints", mBodyDrawSettings.mDrawSoftBodyLRAConstraints, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyLRAConstraints = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(drawing_options, "Draw Soft Body Predicted Bounds", mBodyDrawSettings.mDrawSoftBodyPredictedBounds, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSoftBodyPredictedBounds = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateComboBox(drawing_options, "Draw Soft Body Constraint Color", { "Constraint Type", "Constraint Group", "Constraint Order" }, (int)mBodyDrawSettings.mDrawSoftBodyConstraintColor, [this](int inItem) { mBodyDrawSettings.mDrawSoftBodyConstraintColor = (ESoftBodyConstraintColor)inItem; });
			mDebugUI->ShowMenu(drawing_options);
		});
	#endif // JPH_DEBUG_RENDERER
		mDebugUI->CreateTextButton(main_menu, "Mouse Probe", [this]() {
			UIElement *probe_options = mDebugUI->CreateMenu();
			mDebugUI->CreateComboBox(probe_options, "Mode", { "Pick", "Ray", "RayCollector", "CollidePoint", "CollideShape", "CastShape", "CollideSoftBody", "TransfShape", "GetTriangles", "BP Ray", "BP Box", "BP Sphere", "BP Point", "BP OBox", "BP Cast Box" }, (int)mProbeMode, [this](int inItem) { mProbeMode = (EProbeMode)inItem; });
			mDebugUI->CreateComboBox(probe_options, "Shape", { "Sphere", "Box", "ConvexHull", "Capsule", "TaperedCapsule", "Cylinder", "Triangle", "RotatedTranslated", "StaticCompound", "StaticCompound2", "MutableCompound", "Mesh" }, (int)mProbeShape, [this](int inItem) { mProbeShape = (EProbeShape)inItem; });
			mDebugUI->CreateCheckBox(probe_options, "Scale Shape", mScaleShape, [this](UICheckBox::EState inState) { mScaleShape = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateSlider(probe_options, "Scale X", mShapeScale.GetX(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetX(inValue); });
			mDebugUI->CreateSlider(probe_options, "Scale Y", mShapeScale.GetY(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetY(inValue); });
			mDebugUI->CreateSlider(probe_options, "Scale Z", mShapeScale.GetZ(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetZ(inValue); });
			mDebugUI->CreateComboBox(probe_options, "Back Face Cull", { "On", "Off" }, (int)mBackFaceMode, [this](int inItem) { mBackFaceMode = (EBackFaceMode)inItem; });
			mDebugUI->CreateComboBox(probe_options, "Active Edge Mode", { "Only Active", "All" }, (int)mActiveEdgeMode, [this](int inItem) { mActiveEdgeMode = (EActiveEdgeMode)inItem; });
			mDebugUI->CreateComboBox(probe_options, "Collect Faces Mode", { "Collect Faces", "No Faces" }, (int)mCollectFacesMode, [this](int inItem) { mCollectFacesMode = (ECollectFacesMode)inItem; });
			mDebugUI->CreateSlider(probe_options, "Max Separation Distance", mMaxSeparationDistance, 0.0f, 5.0f, 0.1f, [this](float inValue) { mMaxSeparationDistance = inValue; });
			mDebugUI->CreateCheckBox(probe_options, "Treat Convex As Solid", mTreatConvexAsSolid, [this](UICheckBox::EState inState) { mTreatConvexAsSolid = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(probe_options, "Return Deepest Point", mReturnDeepestPoint, [this](UICheckBox::EState inState) { mReturnDeepestPoint = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(probe_options, "Shrunken Shape + Convex Radius", mUseShrunkenShapeAndConvexRadius, [this](UICheckBox::EState inState) { mUseShrunkenShapeAndConvexRadius = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateCheckBox(probe_options, "Draw Supporting Face", mDrawSupportingFace, [this](UICheckBox::EState inState) { mDrawSupportingFace = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateSlider(probe_options, "Max Hits", float(mMaxHits), 0, 10, 1, [this](float inValue) { mMaxHits = (int)inValue; });
			mDebugUI->ShowMenu(probe_options);
		});
		mDebugUI->CreateTextButton(main_menu, "Shoot Object", [this]() {
			UIElement *shoot_options = mDebugUI->CreateMenu();
			mDebugUI->CreateTextButton(shoot_options, "Shoot Object (B)", [this]() { ShootObject(); });
			mDebugUI->CreateSlider(shoot_options, "Initial Velocity", mShootObjectVelocity, 0.0f, 500.0f, 10.0f, [this](float inValue) { mShootObjectVelocity = inValue; });
			mDebugUI->CreateComboBox(shoot_options, "Shape", { "Sphere", "ConvexHull", "Thin Bar", "Soft Body Cube" }, (int)mShootObjectShape, [this](int inItem) { mShootObjectShape = (EShootObjectShape)inItem; });
			mDebugUI->CreateComboBox(shoot_options, "Motion Quality", { "Discrete", "LinearCast" }, (int)mShootObjectMotionQuality, [this](int inItem) { mShootObjectMotionQuality = (EMotionQuality)inItem; });
			mDebugUI->CreateSlider(shoot_options, "Friction", mShootObjectFriction, 0.0f, 1.0f, 0.05f, [this](float inValue) { mShootObjectFriction = inValue; });
			mDebugUI->CreateSlider(shoot_options, "Restitution", mShootObjectRestitution, 0.0f, 1.0f, 0.05f, [this](float inValue) { mShootObjectRestitution = inValue; });
			mDebugUI->CreateCheckBox(shoot_options, "Scale Shape", mShootObjectScaleShape, [this](UICheckBox::EState inState) { mShootObjectScaleShape = inState == UICheckBox::STATE_CHECKED; });
			mDebugUI->CreateSlider(shoot_options, "Scale X", mShootObjectShapeScale.GetX(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShootObjectShapeScale.SetX(inValue); });
			mDebugUI->CreateSlider(shoot_options, "Scale Y", mShootObjectShapeScale.GetY(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShootObjectShapeScale.SetY(inValue); });
			mDebugUI->CreateSlider(shoot_options, "Scale Z", mShootObjectShapeScale.GetZ(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShootObjectShapeScale.SetZ(inValue); });
			mDebugUI->ShowMenu(shoot_options);
		});
		mDebugUI->CreateTextButton(main_menu, "Help", [this](){
			UIElement *help = mDebugUI->CreateMenu();
			mDebugUI->CreateStaticText(help,
				"ESC: Back to previous menu.\n"
				"WASD + Mouse: Fly around. Hold Shift to speed up, Ctrl to slow down.\n"
				"Space: Hold to pick up and drag a physics object under the crosshair.\n"
				"P: Pause / unpause simulation.\n"
				"O: Single step the simulation.\n"
				",: Step back (only when Physics Settings / Record State for Playback is on).\n"
				".: Step forward (only when Physics Settings / Record State for Playback is on).\n"
				"Shift + ,: Play reverse (only when Physics Settings / Record State for Playback is on).\n"
				"Shift + .: Replay forward (only when Physics Settings / Record State for Playback is on).\n"
				"T: Dump frame timing information to profile_*.html (when JPH_PROFILE_ENABLED defined)."
			);
			mDebugUI->ShowMenu(help);
		});
		mDebugUI->ShowMenu(main_menu);
	}

	// Get test name from commandline
	String cmd_line = ToLower(GetCommandLineA());
	Array<String> args;
	StringToVector(cmd_line, args, " ");
	if (args.size() == 2)
	{
		String cmd = args[1];
		if (cmd == "alltests")
		{
			// Run all tests
			mCheckDeterminism = true;
			mExitAfterRunningTests = true;
			RunAllTests();
		}
		else
		{
			// Search for the test
			const RTTI *test = JPH_RTTI(CreateRigTest);
			for (TestCategory &c : sAllCategories)
				for (uint i = 0; i < c.mNumTests; ++i)
				{
					TestNameAndRTTI &t = c.mTests[i];
					String test_name = ToLower(t.mRTTI->GetName());
					if (test_name == cmd)
					{
						test = t.mRTTI;
						break;
					}
				}

			// Construct test
			StartTest(test);
		}
	}
	else
	{
		// Otherwise start default test
		StartTest(JPH_RTTI(CreateRigTest));
	}
}

SamplesApp::~SamplesApp()
{
	// Clean up
	delete mTest;
	delete mContactListener;
	delete mPhysicsSystem;
	delete mJobSystemValidating;
	delete mJobSystem;
	delete mTempAllocator;
}

void SamplesApp::StartTest(const RTTI *inRTTI)
{
	// Pop active menus, we might be in the settings menu for the test which will be dangling after restarting the test
	mDebugUI->BackToMain();

	// Store old gravity
	Vec3 old_gravity = mPhysicsSystem != nullptr? mPhysicsSystem->GetGravity() : Vec3(0, -9.81f, 0);

	// Discard old test
	delete mTest;
	delete mContactListener;
	delete mPhysicsSystem;

	// Create physics system
	mPhysicsSystem = new PhysicsSystem();
	mPhysicsSystem->Init(cNumBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, mBroadPhaseLayerInterface, mObjectVsBroadPhaseLayerFilter, mObjectVsObjectLayerFilter);
	mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings);

	// Restore gravity
	mPhysicsSystem->SetGravity(old_gravity);

	// Reset dragging
	mDragAnchor = nullptr;
	mDragBody = BodyID();
	mDragConstraint = nullptr;
	mDragVertexIndex = ~uint(0);
	mDragVertexPreviousInvMass = 0.0f;
	mDragFraction = 0.0f;

	// Reset playback state
	mPlaybackFrames.clear();
	mPlaybackMode = EPlaybackMode::Play;
	mCurrentPlaybackFrame = -1;

	// Set new test
	mTestClass = inRTTI;
	mTest = static_cast<Test *>(inRTTI->CreateObject());
	mTest->SetPhysicsSystem(mPhysicsSystem);
	mTest->SetJobSystem(mJobSystem);
	mTest->SetDebugRenderer(mDebugRenderer);
	mTest->SetTempAllocator(mTempAllocator);
	if (mInstallContactListener)
	{
		mContactListener = new ContactListenerImpl;
		mContactListener->SetNextListener(mTest->GetContactListener());
		mPhysicsSystem->SetContactListener(mContactListener);
	}
	else
	{
		mContactListener = nullptr;
		mPhysicsSystem->SetContactListener(mTest->GetContactListener());
	}
	mTest->Initialize();

	// Optimize the broadphase to make the first update fast
	mPhysicsSystem->OptimizeBroadPhase();

	// Make the world render relative to offset specified by test
	mRenderer->SetBaseOffset(mTest->GetDrawOffset());

	// Reset the camera to the original position
	ResetCamera();

	// Start paused
	Pause(true);
	SingleStep();

	// Check if test has settings menu
	mTestSettingsButton->SetDisabled(!mTest->HasSettingsMenu());
}

void SamplesApp::RunAllTests()
{
	mTestsToRun.clear();

	for (const TestCategory &c : sAllCategories)
		for (uint i = 0; i < c.mNumTests; ++i)
		{
			TestNameAndRTTI &t = c.mTests[i];
			mTestsToRun.push_back(t.mRTTI);
		}

	NextTest();
}

bool SamplesApp::NextTest()
{
	if (mTestsToRun.empty())
	{
		if (mExitAfterRunningTests)
			return false; // Exit the application now
		else
			MessageBoxA(nullptr, "Test run complete!", "Complete", MB_OK);
	}
	else
	{
		// Start the timer for 10 seconds
		mTestTimeLeft = 10.0f;

		// Take next test
		const RTTI *rtti = mTestsToRun.front();
		mTestsToRun.erase(mTestsToRun.begin());

		// Start it
		StartTest(rtti);

		// Unpause
		Pause(false);
	}

	mNextTestButton->SetDisabled(mTestsToRun.empty());

	return true;
}

bool SamplesApp::CheckNextTest()
{
	if (mTestTimeLeft >= 0.0f)
	{
		// Update status string
		if (!mStatusString.empty())
			mStatusString += "\n";
		mStatusString += StringFormat("%s: Next test in %.1fs", mTestClass->GetName(), (double)mTestTimeLeft);

		// Use physics time
		mTestTimeLeft -= 1.0f / mUpdateFrequency;

		// If time's up then go to the next test
		if (mTestTimeLeft < 0.0f)
			return NextTest();
	}

	return true;
}

void SamplesApp::TakeSnapshot()
{
	// Convert physics system to scene
	Ref<PhysicsScene> scene = new PhysicsScene();
	scene->FromPhysicsSystem(mPhysicsSystem);

	// Save scene
	ofstream stream("snapshot.bin", ofstream::out | ofstream::trunc | ofstream::binary);
	StreamOutWrapper wrapper(stream);
	if (stream.is_open())
		scene->SaveBinaryState(wrapper, true, true);
}

void SamplesApp::TakeAndReloadSnapshot()
{
	TakeSnapshot();

	StartTest(JPH_RTTI(LoadSnapshotTest));
}

RefConst<Shape> SamplesApp::CreateProbeShape()
{
	RefConst<Shape> shape;
	switch (mProbeShape)
	{
	case EProbeShape::Sphere:
		shape = new SphereShape(0.2f);
		break;

	case EProbeShape::Box:
		shape = new BoxShape(Vec3(0.1f, 0.2f, 0.3f));
		break;

	case EProbeShape::ConvexHull:
		{
			// Create tetrahedron
			Array<Vec3> tetrahedron;
			tetrahedron.push_back(Vec3::sZero());
			tetrahedron.push_back(Vec3(0.2f, 0, 0.4f));
			tetrahedron.push_back(Vec3(0.4f, 0, 0));
			tetrahedron.push_back(Vec3(0.2f, -0.2f, 1.0f));
			shape = ConvexHullShapeSettings(tetrahedron, 0.01f).Create().Get();
		}
		break;

	case EProbeShape::Capsule:
		shape = new CapsuleShape(0.2f, 0.1f);
		break;

	case EProbeShape::TaperedCapsule:
		shape = TaperedCapsuleShapeSettings(0.2f, 0.1f, 0.2f).Create().Get();
		break;

	case EProbeShape::Cylinder:
		shape = new CylinderShape(0.2f, 0.1f);
		break;

	case EProbeShape::Triangle:
		shape = new TriangleShape(Vec3(0.1f, 0.9f, 0.3f), Vec3(-0.9f, -0.5f, 0.2f), Vec3(0.7f, -0.3f, -0.1f));
		break;

	case EProbeShape::RotatedTranslated:
		shape = new RotatedTranslatedShape(Vec3(0.1f, 0.2f, 0.3f), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), new BoxShape(Vec3(0.1f, 0.2f, 0.3f)));
		break;

	case EProbeShape::StaticCompound:
		{
			Array<Vec3> tetrahedron;
			tetrahedron.push_back(Vec3::sZero());
			tetrahedron.push_back(Vec3(-0.2f, 0, 0.4f));
			tetrahedron.push_back(Vec3(0, 0.2f, 0));
			tetrahedron.push_back(Vec3(0.2f, 0, 0.4f));
			RefConst<Shape> convex = ConvexHullShapeSettings(tetrahedron, 0.01f).Create().Get();
			StaticCompoundShapeSettings compound_settings;
			compound_settings.AddShape(Vec3(-0.5f, 0, 0), Quat::sIdentity(), convex);
			compound_settings.AddShape(Vec3(0.5f, 0, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), convex);
			shape = compound_settings.Create().Get();
		}
		break;

	case EProbeShape::StaticCompound2:
		{
			Ref<StaticCompoundShapeSettings> compound = new StaticCompoundShapeSettings();
			compound->AddShape(Vec3(0, 0.5f, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new BoxShape(Vec3(0.5f, 0.15f, 0.1f)));
			compound->AddShape(Vec3(0.5f, 0, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new CylinderShape(0.5f, 0.1f));
			compound->AddShape(Vec3(0, 0, 0.5f), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), new TaperedCapsuleShapeSettings(0.5f, 0.15f, 0.1f));
			StaticCompoundShapeSettings compound2;
			compound2.AddShape(Vec3(0, 0, 0), Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), compound);
			compound2.AddShape(Vec3(0, -0.4f, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), -0.75f * JPH_PI), compound);
			shape = compound2.Create().Get();
		}
		break;

	case EProbeShape::MutableCompound:
		{
			Array<Vec3> tetrahedron;
			tetrahedron.push_back(Vec3::sZero());
			tetrahedron.push_back(Vec3(-0.2f, 0, 0.4f));
			tetrahedron.push_back(Vec3(0, 0.2f, 0));
			tetrahedron.push_back(Vec3(0.2f, 0, 0.4f));
			RefConst<Shape> convex = ConvexHullShapeSettings(tetrahedron, 0.01f).Create().Get();
			MutableCompoundShapeSettings compound_settings;
			compound_settings.AddShape(Vec3(-0.5f, 0, 0), Quat::sIdentity(), convex);
			compound_settings.AddShape(Vec3(0.5f, 0, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), convex);
			shape = compound_settings.Create().Get();
		}
		break;

	case EProbeShape::Mesh:
		shape = ShapeCreator::CreateTorusMesh(2.0f, 0.25f);
		break;
	}

	JPH_ASSERT(shape != nullptr);

	// Scale the shape
	Vec3 scale = mScaleShape? shape->MakeScaleValid(mShapeScale) : Vec3::sReplicate(1.0f);
	JPH_ASSERT(shape->IsValidScale(scale)); // Double check the MakeScaleValid function
	if (!ScaleHelpers::IsNotScaled(scale))
		shape = new ScaledShape(shape, scale);

	return shape;
}

RefConst<Shape> SamplesApp::CreateShootObjectShape()
{
	// Get the scale
	Vec3 scale = mShootObjectScaleShape? mShootObjectShapeScale : Vec3::sReplicate(1.0f);

	// Make it minimally -0.1 or 0.1 depending on the sign
	Vec3 clamped_value = Vec3::sSelect(Vec3::sReplicate(-0.1f), Vec3::sReplicate(0.1f), Vec3::sGreaterOrEqual(scale, Vec3::sZero()));
	scale = Vec3::sSelect(scale, clamped_value, Vec3::sLess(scale.Abs(), Vec3::sReplicate(0.1f)));

	RefConst<Shape> shape;

	switch (mShootObjectShape)
	{
	case EShootObjectShape::Sphere:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
		shape = new SphereShape(GetWorldScale());
		break;

	case EShootObjectShape::ConvexHull:
		{
			Array<Vec3> vertices = {
				Vec3(-0.044661f, 0.001230f, 0.003877f),
				Vec3(-0.024743f, -0.042562f, 0.003877f),
				Vec3(-0.012336f, -0.021073f, 0.048484f),
				Vec3(0.016066f, 0.028121f, -0.049904f),
				Vec3(-0.023734f, 0.043275f, -0.024153f),
				Vec3(0.020812f, 0.036341f, -0.019530f),
				Vec3(0.012495f, 0.021936f, 0.045288f),
				Vec3(0.026750f, 0.001230f, 0.049273f),
				Vec3(0.045495f, 0.001230f, -0.022077f),
				Vec3(0.022193f, -0.036274f, -0.021126f),
				Vec3(0.022781f, -0.037291f, 0.029558f),
				Vec3(0.014691f, -0.023280f, 0.052897f),
				Vec3(-0.012187f, -0.020815f, -0.040214f),
				Vec3(0.000541f, 0.001230f, -0.056224f),
				Vec3(-0.039882f, 0.001230f, -0.019461f),
				Vec3(0.000541f, 0.001230f, 0.056022f),
				Vec3(-0.020614f, -0.035411f, -0.020551f),
				Vec3(-0.019485f, 0.035916f, 0.027001f),
				Vec3(-0.023968f, 0.043680f, 0.003877f),
				Vec3(-0.020051f, 0.001230f, 0.039543f),
				Vec3(0.026213f, 0.001230f, -0.040589f),
				Vec3(-0.010797f, 0.020868f, 0.043152f),
				Vec3(-0.012378f, 0.023607f, -0.040876f)
			};

			// This shape was created at 0.2 world scale, rescale it to the current world scale
			float vert_scale = GetWorldScale() / 0.2f;
			for (Vec3 &v : vertices)
				v *= vert_scale;

			shape = ConvexHullShapeSettings(vertices).Create().Get();
		}
		break;

	case EShootObjectShape::ThinBar:
		shape = BoxShapeSettings(Vec3(0.05f, 0.8f, 0.03f), 0.015f).Create().Get();
		break;

	case EShootObjectShape::SoftBodyCube:
		JPH_ASSERT(false);
		break;
	}

	// Scale shape if needed
	if (scale != Vec3::sReplicate(1.0f))
		shape = new ScaledShape(shape, scale);

	return shape;
}

void SamplesApp::ShootObject()
{
	if (mShootObjectShape != EShootObjectShape::SoftBodyCube)
	{
		// Configure body
		BodyCreationSettings creation_settings(CreateShootObjectShape(), GetCamera().mPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		creation_settings.mMotionQuality = mShootObjectMotionQuality;
		creation_settings.mFriction = mShootObjectFriction;
		creation_settings.mRestitution = mShootObjectRestitution;
		creation_settings.mLinearVelocity = mShootObjectVelocity * GetCamera().mForward;

		// Create body
		mPhysicsSystem->GetBodyInterface().CreateAndAddBody(creation_settings, EActivation::Activate);
	}
	else
	{
		Ref<SoftBodySharedSettings> shared_settings = SoftBodyCreator::CreateCube(5, 0.5f * GetWorldScale());
		for (SoftBodySharedSettings::Vertex &v : shared_settings->mVertices)
		{
			v.mInvMass = 0.025f;
			(mShootObjectVelocity * GetCamera().mForward).StoreFloat3(&v.mVelocity);
		}

		// Confgure soft body
		SoftBodyCreationSettings creation_settings(shared_settings, GetCamera().mPos, Quat::sIdentity(), Layers::MOVING);
		creation_settings.mFriction = mShootObjectFriction;
		creation_settings.mRestitution = mShootObjectRestitution;

		// Create body
		mPhysicsSystem->GetBodyInterface().CreateAndAddSoftBody(creation_settings, EActivation::Activate);
	}
}

bool SamplesApp::CastProbe(float inProbeLength, float &outFraction, RVec3 &outPosition, BodyID &outID)
{
	// Determine start and direction of the probe
	const CameraState &camera = GetCamera();
	RVec3 start = camera.mPos;
	Vec3 direction = inProbeLength * camera.mForward;

	// Define a base offset that is halfway the probe to test getting the collision results relative to some offset.
	// Note that this is not necessarily the best choice for a base offset, but we want something that's not zero
	// and not the start of the collision test either to ensure that we'll see errors in the algorithm.
	RVec3 base_offset = start + 0.5f * direction;

	// Clear output
	outPosition = start + direction;
	outFraction = 1.0f;
	outID = BodyID();

	bool had_hit = false;

	switch (mProbeMode)
	{
	case EProbeMode::Pick:
		{
			// Create ray
			RRayCast ray { start, direction };

			// Cast ray
			RayCastResult hit;
			had_hit = mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), SpecifiedObjectLayerFilter(Layers::MOVING));

			// Fill in results
			outPosition = ray.GetPointOnRay(hit.mFraction);
			outFraction = hit.mFraction;
			outID = hit.mBodyID;

			if (had_hit)
				mDebugRenderer->DrawMarker(outPosition, Color::sYellow, 0.1f);
			else
				mDebugRenderer->DrawMarker(camera.mPos + 0.1f * camera.mForward, Color::sRed, 0.001f);
		}
		break;

	case EProbeMode::Ray:
		{
			// Create ray
			RRayCast ray { start, direction };

			// Cast ray
			RayCastResult hit;
			had_hit = mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit);

			// Fill in results
			outPosition = ray.GetPointOnRay(hit.mFraction);
			outFraction = hit.mFraction;
			outID = hit.mBodyID;

			// Draw results
			if (had_hit)
			{
				BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
				if (lock.Succeeded())
				{
					const Body &hit_body = lock.GetBody();

					// Draw hit
					Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
					mDebugRenderer->DrawLine(start, outPosition, color);
					mDebugRenderer->DrawLine(outPosition, start + direction, Color::sRed);

					// Draw material
					const PhysicsMaterial *material2 = hit_body.GetShape()->GetMaterial(hit.mSubShapeID2);
					mDebugRenderer->DrawText3D(outPosition, material2->GetDebugName());

					// Draw normal
					Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, outPosition);
					mDebugRenderer->DrawArrow(outPosition, outPosition + normal, color, 0.01f);

					// Draw perpendicular axis to indicate hit position
					Vec3 perp1 = normal.GetNormalizedPerpendicular();
					Vec3 perp2 = normal.Cross(perp1);
					mDebugRenderer->DrawLine(outPosition - 0.1f * perp1, outPosition + 0.1f * perp1, color);
					mDebugRenderer->DrawLine(outPosition - 0.1f * perp2, outPosition + 0.1f * perp2, color);

					// Get and draw the result of GetSupportingFace
					if (mDrawSupportingFace)
					{
						Shape::SupportingFace face;
						hit_body.GetTransformedShape().GetSupportingFace(hit.mSubShapeID2, -normal, base_offset, face);
						mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), face, Color::sWhite, 0.01f);
					}
				}
			}
			else
			{
				mDebugRenderer->DrawMarker(outPosition, Color::sRed, 0.1f);
			}
		}
		break;

	case EProbeMode::RayCollector:
		{
			// Create ray
			RRayCast ray { start, direction };

			// Create settings
			RayCastSettings settings;
			settings.mBackFaceMode = mBackFaceMode;
			settings.mTreatConvexAsSolid = mTreatConvexAsSolid;

			// Cast ray
			Array<RayCastResult> hits;
			if (mMaxHits == 0)
			{
				AnyHitCollisionCollector<CastRayCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else if (mMaxHits == 1)
			{
				ClosestHitCollisionCollector<CastRayCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else
			{
				AllHitCollisionCollector<CastRayCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, settings, collector);
				collector.Sort();
				hits.insert(hits.end(), collector.mHits.begin(), collector.mHits.end());
				if ((int)hits.size() > mMaxHits)
					hits.resize(mMaxHits);
			}

			had_hit = !hits.empty();
			if (had_hit)
			{
				// Fill in results
				RayCastResult &first_hit = hits.front();
				outPosition = ray.GetPointOnRay(first_hit.mFraction);
				outFraction = first_hit.mFraction;
				outID = first_hit.mBodyID;

				// Draw results
				RVec3 prev_position = start;
				bool c = false;
				for (const RayCastResult &hit : hits)
				{
					// Draw line
					RVec3 position = ray.GetPointOnRay(hit.mFraction);
					mDebugRenderer->DrawLine(prev_position, position, c? Color::sGrey : Color::sWhite);
					c = !c;
					prev_position = position;

					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw material
						const PhysicsMaterial *material2 = hit_body.GetShape()->GetMaterial(hit.mSubShapeID2);
						mDebugRenderer->DrawText3D(position, material2->GetDebugName());

						// Draw normal
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, position);
						mDebugRenderer->DrawArrow(position, position + normal, color, 0.01f);

						// Draw perpendicular axis to indicate hit position
						Vec3 perp1 = normal.GetNormalizedPerpendicular();
						Vec3 perp2 = normal.Cross(perp1);
						mDebugRenderer->DrawLine(position - 0.1f * perp1, position + 0.1f * perp1, color);
						mDebugRenderer->DrawLine(position - 0.1f * perp2, position + 0.1f * perp2, color);

						// Get and draw the result of GetSupportingFace
						if (mDrawSupportingFace)
						{
							Shape::SupportingFace face;
							hit_body.GetTransformedShape().GetSupportingFace(hit.mSubShapeID2, -normal, base_offset, face);
							mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), face, Color::sWhite, 0.01f);
						}
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(ray.GetPointOnRay(hits.back().mFraction), start + direction, Color::sRed);
			}
			else
			{
				// Draw 'miss'
				mDebugRenderer->DrawLine(start, start + direction, Color::sRed);
				mDebugRenderer->DrawMarker(start + direction, Color::sRed, 0.1f);
			}
		}
		break;

	case EProbeMode::CollidePoint:
		{
			// Create point
			const float fraction = 0.1f;
			RVec3 point = start + fraction * direction;

			// Collide point
			AllHitCollisionCollector<CollidePointCollector> collector;
			mPhysicsSystem->GetNarrowPhaseQuery().CollidePoint(point, collector);

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				for (const CollidePointResult &hit : collector.mHits)
				{
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawMarker(point, had_hit? Color::sGreen : Color::sRed, 0.1f);
		}
		break;

	case EProbeMode::CollideShape:
		{
			// Create shape cast
			RefConst<Shape> shape = CreateProbeShape();
			Mat44 rotation = Mat44::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Mat44::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI);
			Mat44 com = Mat44::sTranslation(shape->GetCenterOfMass());
			RMat44 shape_transform(RMat44::sTranslation(start + 5.0f * camera.mForward) * rotation * com);

			// Create settings
			CollideShapeSettings settings;
			settings.mActiveEdgeMode = mActiveEdgeMode;
			settings.mBackFaceMode = mBackFaceMode;
			settings.mCollectFacesMode = mCollectFacesMode;
			settings.mMaxSeparationDistance = mMaxSeparationDistance;

			Array<CollideShapeResult> hits;
			if (mMaxHits == 0)
			{
				AnyHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, base_offset, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else if (mMaxHits == 1)
			{
				ClosestHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, base_offset, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else
			{
				AllHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, base_offset, collector);
				collector.Sort();
				hits.insert(hits.end(), collector.mHits.begin(), collector.mHits.end());
				if ((int)hits.size() > mMaxHits)
					hits.resize(mMaxHits);
			}

			had_hit = !hits.empty();
			if (had_hit)
			{
				// Draw results
				for (const CollideShapeResult &hit : hits)
				{
					// Draw 'hit'
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID2);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw contact
						RVec3 contact_position1 = base_offset + hit.mContactPointOn1;
						RVec3 contact_position2 = base_offset + hit.mContactPointOn2;
						mDebugRenderer->DrawMarker(contact_position1, Color::sGreen, 0.1f);
						mDebugRenderer->DrawMarker(contact_position2, Color::sRed, 0.1f);

						Vec3 pen_axis = hit.mPenetrationAxis;
						float pen_axis_len = pen_axis.Length();
						if (pen_axis_len > 0.0f)
						{
							pen_axis /= pen_axis_len;

							// Draw penetration axis with length of the penetration
							mDebugRenderer->DrawArrow(contact_position2, contact_position2 + pen_axis * hit.mPenetrationDepth, Color::sYellow, 0.01f);

							// Draw normal (flipped so it points towards body 1)
							mDebugRenderer->DrawArrow(contact_position2, contact_position2 - pen_axis, Color::sOrange, 0.01f);
						}

						// Draw material
						const PhysicsMaterial *material2 = hit_body.GetShape()->GetMaterial(hit.mSubShapeID2);
						mDebugRenderer->DrawText3D(contact_position2, material2->GetDebugName());

						// Draw faces
						mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), hit.mShape1Face, Color::sYellow, 0.01f);
						mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), hit.mShape2Face, Color::sRed, 0.01f);
					}
				}
			}

		#ifdef JPH_DEBUG_RENDERER
			// Draw shape
			shape->Draw(mDebugRenderer, shape_transform, Vec3::sReplicate(1.0f), had_hit? Color::sGreen : Color::sGrey, false, false);
		#endif // JPH_DEBUG_RENDERER
		}
		break;

	case EProbeMode::CastShape:
		{
			// Create shape cast
			RefConst<Shape> shape = CreateProbeShape();
			Mat44 rotation = Mat44::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Mat44::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI);
			RShapeCast shape_cast = RShapeCast::sFromWorldTransform(shape, Vec3::sReplicate(1.0f), RMat44::sTranslation(start) * rotation, direction);

			// Settings
			ShapeCastSettings settings;
			settings.mUseShrunkenShapeAndConvexRadius = mUseShrunkenShapeAndConvexRadius;
			settings.mActiveEdgeMode = mActiveEdgeMode;
			settings.mBackFaceModeTriangles = mBackFaceMode;
			settings.mBackFaceModeConvex = mBackFaceMode;
			settings.mReturnDeepestPoint = mReturnDeepestPoint;
			settings.mCollectFacesMode = mCollectFacesMode;

			// Cast shape
			Array<ShapeCastResult> hits;
			if (mMaxHits == 0)
			{
				AnyHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, base_offset, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else if (mMaxHits == 1)
			{
				ClosestHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, base_offset, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else
			{
				AllHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, base_offset, collector);
				collector.Sort();
				hits.insert(hits.end(), collector.mHits.begin(), collector.mHits.end());
				if ((int)hits.size() > mMaxHits)
					hits.resize(mMaxHits);
			}

			had_hit = !hits.empty();
			if (had_hit)
			{
				// Fill in results
				ShapeCastResult &first_hit = hits.front();
				outPosition = shape_cast.GetPointOnRay(first_hit.mFraction);
				outFraction = first_hit.mFraction;
				outID = first_hit.mBodyID2;

				// Draw results
				RVec3 prev_position = start;
				bool c = false;
				for (const ShapeCastResult &hit : hits)
				{
					// Draw line
					RVec3 position = shape_cast.GetPointOnRay(hit.mFraction);
					mDebugRenderer->DrawLine(prev_position, position, c? Color::sGrey : Color::sWhite);
					c = !c;
					prev_position = position;

					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID2);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw shape
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
					#ifdef JPH_DEBUG_RENDERER
						shape_cast.mShape->Draw(mDebugRenderer, shape_cast.mCenterOfMassStart.PostTranslated(hit.mFraction * shape_cast.mDirection), Vec3::sReplicate(1.0f), color, false, false);
					#endif // JPH_DEBUG_RENDERER

						// Draw normal
						RVec3 contact_position1 = base_offset + hit.mContactPointOn1;
						RVec3 contact_position2 = base_offset + hit.mContactPointOn2;
						Vec3 normal = hit.mPenetrationAxis.Normalized();
						mDebugRenderer->DrawArrow(contact_position2, contact_position2 - normal, color, 0.01f); // Flip to make it point towards the cast body

						// Contact position 1
						mDebugRenderer->DrawMarker(contact_position1, Color::sGreen, 0.1f);

						// Draw perpendicular axis to indicate contact position 2
						Vec3 perp1 = normal.GetNormalizedPerpendicular();
						Vec3 perp2 = normal.Cross(perp1);
						mDebugRenderer->DrawLine(contact_position2 - 0.1f * perp1, contact_position2 + 0.1f * perp1, color);
						mDebugRenderer->DrawLine(contact_position2 - 0.1f * perp2, contact_position2 + 0.1f * perp2, color);

						// Draw material
						const PhysicsMaterial *material2 = hit_body.GetShape()->GetMaterial(hit.mSubShapeID2);
						mDebugRenderer->DrawText3D(position, material2->GetDebugName());

						// Draw faces
						mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), hit.mShape1Face, Color::sYellow, 0.01f);
						mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(base_offset), hit.mShape2Face, Color::sRed, 0.01f);
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(shape_cast.GetPointOnRay(hits.back().mFraction), start + direction, Color::sRed);
			}
			else
			{
				// Draw 'miss'
				mDebugRenderer->DrawLine(start, start + direction, Color::sRed);
			#ifdef JPH_DEBUG_RENDERER
				shape_cast.mShape->Draw(mDebugRenderer, shape_cast.mCenterOfMassStart.PostTranslated(shape_cast.mDirection), Vec3::sReplicate(1.0f), Color::sRed, false, false);
			#endif // JPH_DEBUG_RENDERER
			}
		}
		break;

	case EProbeMode::CollideSoftBody:
		{
			// Create a soft body vertex
			const float fraction = 0.2f;
			const float max_distance = 10.0f;
			SoftBodyVertex vertex;
			vertex.mInvMass = 1.0f;
			vertex.mPosition = fraction * direction;
			vertex.mVelocity = 10.0f * direction;
			vertex.mCollidingShapeIndex = -1;
			vertex.mLargestPenetration = -FLT_MAX;

			// Get shapes in a large radius around the start position
			AABox box(Vec3(start + vertex.mPosition), max_distance);
			AllHitCollisionCollector<TransformedShapeCollector> collector;
			mPhysicsSystem->GetNarrowPhaseQuery().CollectTransformedShapes(box, collector);

			// Closest point found using CollideShape, position relative to 'start'
			Vec3 closest_point = vertex.mPosition;
			float closest_point_penetration = 0;

			// Test against each shape
			for (const TransformedShape &ts : collector.mHits)
			{
				int colliding_shape_index = int(&ts - collector.mHits.data());
				ts.mShape->CollideSoftBodyVertices((RMat44::sTranslation(-start) * ts.GetCenterOfMassTransform()).ToMat44(), ts.GetShapeScale(), &vertex, 1, 1.0f / 60.0f, Vec3::sZero(), colliding_shape_index);
				if (vertex.mCollidingShapeIndex == colliding_shape_index)
				{
					// To draw a plane, we need a point but CollideSoftBodyVertices doesn't provide one, so we use CollideShape with a tiny sphere to get the closest point and then project that onto the plane to draw the plane
					SphereShape point_sphere(1.0e-6f);
					point_sphere.SetEmbedded();
					CollideShapeSettings settings;
					settings.mMaxSeparationDistance = sqrt(3.0f) * max_distance; // Box is extended in all directions by max_distance
					ClosestHitCollisionCollector<CollideShapeCollector> collide_shape_collector;
					ts.CollideShape(&point_sphere, Vec3::sReplicate(1.0f), RMat44::sTranslation(start + vertex.mPosition), settings, start, collide_shape_collector);
					if (collide_shape_collector.HadHit())
					{
						closest_point = collide_shape_collector.mHit.mContactPointOn2;
						closest_point_penetration = collide_shape_collector.mHit.mPenetrationDepth;
					}
				}
			}

			// Draw test point
			mDebugRenderer->DrawMarker(start + vertex.mPosition, Color::sYellow, 0.1f);
			mDebugRenderer->DrawMarker(start + closest_point, Color::sRed, 0.1f);

			// Draw collision plane
			if (vertex.mCollidingShapeIndex != -1)
			{
				RVec3 plane_point = start + vertex.mPosition - vertex.mCollisionPlane.GetNormal() * vertex.mCollisionPlane.SignedDistance(vertex.mPosition);
				mDebugRenderer->DrawPlane(plane_point, vertex.mCollisionPlane.GetNormal(), Color::sGreen, 2.0f);

				if (abs(closest_point_penetration - vertex.mLargestPenetration) > 0.001f)
					mDebugRenderer->DrawText3D(plane_point, StringFormat("Pen %f (exp %f)", (double)vertex.mLargestPenetration, (double)closest_point_penetration));
				else
					mDebugRenderer->DrawText3D(plane_point, StringFormat("Pen %f", (double)vertex.mLargestPenetration));
			}
		}
		break;

	case EProbeMode::TransformedShape:
		{
			// Create box
			const float fraction = 0.2f;
			RVec3 center = start + fraction * direction;
			Vec3 half_extent = 0.5f * mShapeScale;
			AABox box(center - half_extent, center + half_extent);

			// Get shapes
			AllHitCollisionCollector<TransformedShapeCollector> collector;
			mPhysicsSystem->GetNarrowPhaseQuery().CollectTransformedShapes(box, collector);

			// Draw results
			for (const TransformedShape &ts : collector.mHits)
				mDebugRenderer->DrawWireBox(RMat44::sRotationTranslation(ts.mShapeRotation, ts.mShapePositionCOM) * Mat44::sScale(ts.GetShapeScale()), ts.mShape->GetLocalBounds(), Color::sYellow);

			// Draw test location
			mDebugRenderer->DrawWireBox(box, !collector.mHits.empty()? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::GetTriangles:
		{
			// Create box
			const float fraction = 0.2f;
			RVec3 center = start + fraction * direction;
			Vec3 half_extent = 2.0f * mShapeScale;
			AABox box(center - half_extent, center + half_extent);

			// Get shapes
			AllHitCollisionCollector<TransformedShapeCollector> collector;
			mPhysicsSystem->GetNarrowPhaseQuery().CollectTransformedShapes(box, collector);

			// Loop over shapes
			had_hit = false;
			for (const TransformedShape &ts : collector.mHits)
			{
				const int cMaxTriangles = 32;
				Float3 vertices[cMaxTriangles * 3];
				const PhysicsMaterial *materials[cMaxTriangles];

				// Start iterating triangles
				Shape::GetTrianglesContext ctx;
				ts.GetTrianglesStart(ctx, box, base_offset);
				for (;;)
				{
					// Fetch next triangles
					int count = ts.GetTrianglesNext(ctx, cMaxTriangles, vertices, materials);
					if (count == 0)
						break;

					// Draw triangles
					const PhysicsMaterial **m = materials;
					for (Float3 *v = vertices, *v_end = vertices + 3 * count; v < v_end; v += 3, ++m)
					{
						RVec3 v1 = base_offset + Vec3(v[0]), v2 = base_offset + Vec3(v[1]), v3 = base_offset + Vec3(v[2]);
						RVec3 triangle_center = (v1 + v2 + v3) / 3.0f;
						Vec3 triangle_normal = Vec3(v2 - v1).Cross(Vec3(v3 - v1)).Normalized();
						mDebugRenderer->DrawWireTriangle(v1, v2, v3, (*m)->GetDebugColor());
						mDebugRenderer->DrawArrow(triangle_center, triangle_center + triangle_normal, Color::sGreen, 0.01f);
					}

					had_hit = true;
				}
			}

			// Draw test location
			mDebugRenderer->DrawWireBox(box, had_hit? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::BroadPhaseRay:
		{
			// Create ray
			RayCast ray { Vec3(start), direction };

			// Cast ray
			AllHitCollisionCollector<RayCastBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			collector.Sort();

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				RVec3 prev_position = start;
				bool c = false;
				for (const BroadPhaseCastResult &hit : collector.mHits)
				{
					// Draw line
					RVec3 position = start + hit.mFraction * direction;
					Color cast_color = c? Color::sGrey : Color::sWhite;
					mDebugRenderer->DrawLine(prev_position, position, cast_color);
					mDebugRenderer->DrawMarker(position, cast_color, 0.1f);
					c = !c;
					prev_position = position;

					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(start + collector.mHits.back().mFraction * direction, start + direction, Color::sRed);
			}
			else
			{
				// Draw 'miss'
				mDebugRenderer->DrawLine(start, start + direction, Color::sRed);
				mDebugRenderer->DrawMarker(start + direction, Color::sRed, 0.1f);
			}
		}
		break;

	case EProbeMode::BroadPhaseBox:
		{
			// Create box
			const float fraction = 0.2f;
			RVec3 center = start + fraction * direction;
			Vec3 half_extent = 2.0f * mShapeScale;
			AABox box(center - half_extent, center + half_extent);

			// Collide box
			AllHitCollisionCollector<CollideShapeBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CollideAABox(box, collector);

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				for (const BodyID &hit : collector.mHits)
				{
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawWireBox(box, had_hit? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::BroadPhaseSphere:
		{
			// Create sphere
			const float fraction = 0.2f;
			const float radius = mShapeScale.Length() * 2.0f;
			Vec3 point(start + fraction * direction);

			// Collide sphere
			AllHitCollisionCollector<CollideShapeBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CollideSphere(point, radius, collector);

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				for (const BodyID &hit : collector.mHits)
				{
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawWireSphere(RVec3(point), radius, had_hit? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::BroadPhasePoint:
		{
			// Create point
			const float fraction = 0.1f;
			Vec3 point(start + fraction * direction);

			// Collide point
			AllHitCollisionCollector<CollideShapeBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CollidePoint(point, collector);

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				for (const BodyID &hit : collector.mHits)
				{
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawMarker(RVec3(point), had_hit? Color::sGreen : Color::sRed, 0.1f);
		}
		break;

	case EProbeMode::BroadPhaseOrientedBox:
		{
			// Create box
			const float fraction = 0.2f;
			Vec3 center(start + fraction * direction);
			Vec3 half_extent = 2.0f * mShapeScale;
			OrientedBox box(Mat44::sRotationTranslation(Quat::sRotation(Vec3::sAxisZ(), 0.2f * JPH_PI) * Quat::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI), center), half_extent);

			// Collide box
			AllHitCollisionCollector<CollideShapeBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CollideOrientedBox(box, collector);

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				for (const BodyID &hit : collector.mHits)
				{
					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawWireBox(box, had_hit? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::BroadPhaseCastBox:
		{
			// Create box
			Vec3 half_extent = 2.0f * mShapeScale;
			AABox box(start - half_extent, start + half_extent);
			AABoxCast box_cast { box, direction };

			// Cast box
			AllHitCollisionCollector<CastShapeBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CastAABox(box_cast, collector);
			collector.Sort();

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				RVec3 prev_position = start;
				bool c = false;
				for (const BroadPhaseCastResult &hit : collector.mHits)
				{
					// Draw line
					RVec3 position = start + hit.mFraction * direction;
					Color cast_color = c? Color::sGrey : Color::sWhite;
					mDebugRenderer->DrawLine(prev_position, position, cast_color);
					mDebugRenderer->DrawWireBox(RMat44::sTranslation(position), AABox(-half_extent, half_extent), cast_color);
					c = !c;
					prev_position = position;

					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetCenterOfMassTransform(), hit_body.GetShape()->GetLocalBounds(), color);
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(start + collector.mHits.back().mFraction * direction, start + direction, Color::sRed);
			}
			else
			{
				// Draw 'miss'
				mDebugRenderer->DrawLine(start, start + direction, Color::sRed);
				mDebugRenderer->DrawWireBox(RMat44::sTranslation(start + direction), AABox(-half_extent, half_extent), Color::sRed);
			}
		}
		break;
	}

	return had_hit;
}

void SamplesApp::UpdateDebug(float inDeltaTime)
{
	JPH_PROFILE_FUNCTION();

	const float cDragRayLength = 40.0f;

	BodyInterface &bi = mPhysicsSystem->GetBodyInterface();

	// Handle keyboard input for which simulation needs to be running
	if (mKeyboard->IsKeyPressedAndTriggered(DIK_B, mWasShootKeyPressed))
		ShootObject();

	// Allow the user to drag rigid/soft bodies around
	if (mDragConstraint == nullptr && mDragVertexIndex == ~uint(0))
	{
		// Not dragging yet
		RVec3 hit_position;
		if (CastProbe(cDragRayLength, mDragFraction, hit_position, mDragBody))
		{
			// If key is pressed create constraint to start dragging
			if (mKeyboard->IsKeyPressed(DIK_SPACE))
			{
				// Target body must be dynamic
				BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), mDragBody);
				if (lock.Succeeded())
				{
					Body &drag_body = lock.GetBody();
					if (drag_body.IsSoftBody())
					{
						SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(drag_body.GetMotionProperties());

						// Find closest vertex
						Vec3 local_hit_position = Vec3(drag_body.GetInverseCenterOfMassTransform() * hit_position);
						float closest_dist_sq = FLT_MAX;
						for (SoftBodyVertex &v : mp->GetVertices())
						{
							float dist_sq = (v.mPosition - local_hit_position).LengthSq();
							if (dist_sq < closest_dist_sq)
							{
								closest_dist_sq = dist_sq;
								mDragVertexIndex = uint(&v - mp->GetVertices().data());
							}
						}

						// Make the vertex kinematic
						SoftBodyVertex &v = mp->GetVertex(mDragVertexIndex);
						mDragVertexPreviousInvMass = v.mInvMass;
						v.mInvMass = 0.0f;
					}
					else if (drag_body.IsDynamic())
					{
						// Create constraint to drag body
						DistanceConstraintSettings settings;
						settings.mPoint1 = settings.mPoint2 = hit_position;
						settings.mLimitsSpringSettings.mFrequency = 2.0f / GetWorldScale();
						settings.mLimitsSpringSettings.mDamping = 1.0f;

						// Construct fixed body for the mouse constraint
						// Note that we don't add it to the world since we don't want anything to collide with it, we just
						// need an anchor for a constraint
						Body *drag_anchor = bi.CreateBody(BodyCreationSettings(new SphereShape(0.01f), hit_position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
						mDragAnchor = drag_anchor;

						// Construct constraint that connects the drag anchor with the body that we want to drag
						mDragConstraint = settings.Create(*drag_anchor, drag_body);
						mPhysicsSystem->AddConstraint(mDragConstraint);
					}
				}
			}
		}
	}
	else
	{
		if (!mKeyboard->IsKeyPressed(DIK_SPACE))
		{
			// If key released, destroy constraint
			if (mDragConstraint != nullptr)
			{
				mPhysicsSystem->RemoveConstraint(mDragConstraint);
				mDragConstraint = nullptr;
			}

			// Destroy drag anchor
			if (mDragAnchor != nullptr)
			{
				bi.DestroyBody(mDragAnchor->GetID());
				mDragAnchor = nullptr;
			}

			// Release dragged vertex
			if (mDragVertexIndex != ~uint(0))
			{
				// Restore vertex mass
				BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), mDragBody);
				if (lock.Succeeded())
				{
					Body &body = lock.GetBody();
					JPH_ASSERT(body.IsSoftBody());
					SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());
					mp->GetVertex(mDragVertexIndex).mInvMass = mDragVertexPreviousInvMass;
				}
				mDragVertexIndex = ~uint(0);
				mDragVertexPreviousInvMass = 0;
			}

			// Forget the drag body
			mDragBody = BodyID();
		}
		else
		{
			// Else drag the body to the new position
			RVec3 new_pos = GetCamera().mPos + cDragRayLength * mDragFraction * GetCamera().mForward;

			switch (bi.GetBodyType(mDragBody))
			{
			case EBodyType::RigidBody:
				bi.SetPositionAndRotation(mDragAnchor->GetID(), new_pos, Quat::sIdentity(), EActivation::DontActivate);
				break;

			case EBodyType::SoftBody:
				{
					BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), mDragBody);
					if (lock.Succeeded())
					{
						Body &body = lock.GetBody();
						SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(body.GetMotionProperties());
						SoftBodyVertex &v = mp->GetVertex(mDragVertexIndex);
						v.mVelocity = body.GetRotation().Conjugated() * Vec3(new_pos - body.GetCenterOfMassTransform() * v.mPosition) / inDeltaTime;
					}
				}
				break;
			}

			// Activate other body
			bi.ActivateBody(mDragBody);
		}
	}
}

bool SamplesApp::UpdateFrame(float inDeltaTime)
{
	// Reinitialize the job system if the concurrency setting changed
	if (mMaxConcurrentJobs != mJobSystem->GetMaxConcurrency())
		static_cast<JobSystemThreadPool *>(mJobSystem)->SetNumThreads(mMaxConcurrentJobs - 1);

	// Restart the test if the test requests this
	if (mTest->NeedsRestart())
	{
		StartTest(mTestClass);
		return true;
	}

	// Get the status string
	mStatusString = mTest->GetStatusString();

	// Select the next test if automatic testing times out
	if (!CheckNextTest())
		return false;

	// Handle keyboard input
	bool shift = mKeyboard->IsKeyPressed(DIK_LSHIFT) || mKeyboard->IsKeyPressed(DIK_RSHIFT);
#ifdef JPH_DEBUG_RENDERER
	bool alt = mKeyboard->IsKeyPressed(DIK_LALT) || mKeyboard->IsKeyPressed(DIK_RALT);
#endif // JPH_DEBUG_RENDERER
	for (int key = mKeyboard->GetFirstKey(); key != 0; key = mKeyboard->GetNextKey())
		switch (key)
		{
		case DIK_R:
			StartTest(mTestClass);
			return true;

		case DIK_N:
			if (!mTestsToRun.empty())
				NextTest();
			break;

	#ifdef JPH_DEBUG_RENDERER
		case DIK_H:
			if (shift)
				mBodyDrawSettings.mDrawGetSupportFunction = !mBodyDrawSettings.mDrawGetSupportFunction;
			else if (alt)
				mDrawGetTriangles = !mDrawGetTriangles;
			else
				mBodyDrawSettings.mDrawShape = !mBodyDrawSettings.mDrawShape;
			break;

		case DIK_F:
			if (shift)
				mBodyDrawSettings.mDrawGetSupportingFace = !mBodyDrawSettings.mDrawGetSupportingFace;
			break;

		case DIK_I:
			mBodyDrawSettings.mDrawMassAndInertia = !mBodyDrawSettings.mDrawMassAndInertia;
			break;

		case DIK_1:
			ContactConstraintManager::sDrawContactPoint = !ContactConstraintManager::sDrawContactPoint;
			break;

		case DIK_2:
			ContactConstraintManager::sDrawSupportingFaces = !ContactConstraintManager::sDrawSupportingFaces;
			break;

		case DIK_3:
			ContactConstraintManager::sDrawContactPointReduction = !ContactConstraintManager::sDrawContactPointReduction;
			break;

		case DIK_C:
			mDrawConstraints = !mDrawConstraints;
			break;

		case DIK_L:
			mDrawConstraintLimits = !mDrawConstraintLimits;
			break;

		case DIK_M:
			ContactConstraintManager::sDrawContactManifolds = !ContactConstraintManager::sDrawContactManifolds;
			break;

		case DIK_W:
			if (alt)
				mBodyDrawSettings.mDrawShapeWireframe = !mBodyDrawSettings.mDrawShapeWireframe;
			break;
	#endif // JPH_DEBUG_RENDERER

		case DIK_COMMA:
			// Back stepping
			if (mPlaybackFrames.size() > 1)
			{
				if (mPlaybackMode == EPlaybackMode::Play)
				{
					JPH_ASSERT(mCurrentPlaybackFrame == -1);
					mCurrentPlaybackFrame = (int)mPlaybackFrames.size() - 1;
				}
				mPlaybackMode = shift? EPlaybackMode::Rewind : EPlaybackMode::StepBack;
			}
			break;

		case DIK_PERIOD:
			// Forward stepping
			if (mPlaybackMode != EPlaybackMode::Play)
			{
				JPH_ASSERT(mCurrentPlaybackFrame >= 0);
				mPlaybackMode = shift? EPlaybackMode::FastForward : EPlaybackMode::StepForward;
			}
			break;
		}

	// Stop recording if record state is turned off
	if (!mRecordState)
	{
		mPlaybackFrames.clear();
		mPlaybackMode = EPlaybackMode::Play;
		mCurrentPlaybackFrame = -1;
	}

	// Determine if we need to check deterministic simulation
	bool check_determinism = mCheckDeterminism && mTest->IsDeterministic();

	// Check if we've in replay mode
	if (mPlaybackMode != EPlaybackMode::Play)
	{
		JPH_PROFILE("RestoreState");

		// We're in replay mode
		JPH_ASSERT(mCurrentPlaybackFrame >= 0);

		// Ensure the simulation is paused
		Pause(true);

		// Always restore state when not paused, the debug drawing will be cleared
		bool restore_state = inDeltaTime > 0.0f;

		// Advance to the next frame when single stepping or unpausing
		switch (mPlaybackMode)
		{
		case EPlaybackMode::StepBack:
			mPlaybackMode = EPlaybackMode::Stop;
			[[fallthrough]];

		case EPlaybackMode::Rewind:
			if (mCurrentPlaybackFrame > 0)
			{
				mCurrentPlaybackFrame--;
				restore_state = true;
			}
			break;

		case EPlaybackMode::StepForward:
			mPlaybackMode = EPlaybackMode::Stop;
			[[fallthrough]];

		case EPlaybackMode::FastForward:
			if (mCurrentPlaybackFrame < (int)mPlaybackFrames.size() - 1)
			{
				mCurrentPlaybackFrame++;
				restore_state = true;
			}
			break;

		case EPlaybackMode::Stop:
		case EPlaybackMode::Play:
			// Satisfy compiler
			break;
		}

		// If the replay frame changed we need to update state
		if (restore_state)
		{
			// Clear existing debug stuff so we can render this restored frame
			// (if we're paused, we will otherwise not clear the debugging stuff)
			ClearDebugRenderer();

			// Restore state to what it was during that time
			PlayBackFrame &frame = mPlaybackFrames[mCurrentPlaybackFrame];
			RestoreState(frame.mState);

			// Also restore input back to what it was at the time
			frame.mInputState.Rewind();
			mTest->RestoreInputState(frame.mInputState);

			// Physics world is drawn using debug lines, when not paused
			// Draw state prior to step so that debug lines are created from the same state
			// (the constraints are solved on the current state and then the world is stepped)
			DrawPhysics();

			// Step the world (with fixed frequency)
			StepPhysics(mJobSystem);

		#ifdef JPH_DEBUG_RENDERER
			// Draw any contacts that were collected through the contact listener
			if (mContactListener)
				mContactListener->DrawState();
		#endif // JPH_DEBUG_RENDERER

			// Validate that update result is the same as the previously recorded state
			if (check_determinism && mCurrentPlaybackFrame < (int)mPlaybackFrames.size() - 1)
				ValidateState(mPlaybackFrames[mCurrentPlaybackFrame + 1].mState);
		}

		// On the last frame go back to play mode
		if (mCurrentPlaybackFrame >= (int)mPlaybackFrames.size() - 1)
		{
			mPlaybackMode = EPlaybackMode::Play;
			mCurrentPlaybackFrame = -1;
		}

		// On the first frame go to stop mode
		if (mCurrentPlaybackFrame == 0)
			mPlaybackMode = EPlaybackMode::Stop;
	}
	else
	{
		// Normal update
		JPH_ASSERT(mCurrentPlaybackFrame == -1);

		if (inDeltaTime > 0.0f)
		{
			// Debugging functionality like shooting a ball and dragging objects
			UpdateDebug(inDeltaTime);

			{
				// Process input, this is done once and before we save the state so that we can save the input state
				JPH_PROFILE("ProcessInput");
				Test::ProcessInputParams handle_input;
				handle_input.mDeltaTime = 1.0f / mUpdateFrequency;
				handle_input.mKeyboard = mKeyboard;
				handle_input.mCameraState = GetCamera();
				mTest->ProcessInput(handle_input);
			}

			if (mRecordState || check_determinism)
			{
				// Record the state prior to the step
				mPlaybackFrames.push_back(PlayBackFrame());
				SaveState(mPlaybackFrames.back().mState);

				// Save input too
				mTest->SaveInputState(mPlaybackFrames.back().mInputState);
			}

			// Physics world is drawn using debug lines, when not paused
			// Draw state prior to step so that debug lines are created from the same state
			// (the constraints are solved on the current state and then the world is stepped)
			DrawPhysics();

			// Update the physics world
			StepPhysics(mJobSystem);

		#ifdef JPH_DEBUG_RENDERER
			// Draw any contacts that were collected through the contact listener
			if (mContactListener)
				mContactListener->DrawState();
		#endif // JPH_DEBUG_RENDERER

			if (check_determinism)
			{
				// Save the current state
				StateRecorderImpl post_step_state;
				SaveState(post_step_state);

				// Restore to the previous state
				PlayBackFrame &frame = mPlaybackFrames.back();
				RestoreState(frame.mState);

				// Also restore input back to what it was at the time
				frame.mInputState.Rewind();
				mTest->RestoreInputState(frame.mInputState);

				// Step again
				StepPhysics(mJobSystemValidating);

				// Validate that the result is the same
				ValidateState(post_step_state);
			}
		}
	}

	return true;
}

void SamplesApp::DrawPhysics()
{
#ifdef JPH_DEBUG_RENDERER
	mPhysicsSystem->DrawBodies(mBodyDrawSettings, mDebugRenderer);

	if (mDrawConstraints)
		mPhysicsSystem->DrawConstraints(mDebugRenderer);

	if (mDrawConstraintLimits)
		mPhysicsSystem->DrawConstraintLimits(mDebugRenderer);

	if (mDrawConstraintReferenceFrame)
		mPhysicsSystem->DrawConstraintReferenceFrame(mDebugRenderer);

	if (mDrawPhysicsSystemBounds)
		mDebugRenderer->DrawWireBox(mPhysicsSystem->GetBounds(), Color::sGreen);
#endif // JPH_DEBUG_RENDERER

	// This map collects the shapes that we used this frame
	ShapeToGeometryMap shape_to_geometry;

#ifdef JPH_DEBUG_RENDERER
	if (mDrawGetTriangles)
#endif // JPH_DEBUG_RENDERER
	{
		JPH_PROFILE("DrawGetTriangles");

		// Iterate through all active bodies
		BodyIDVector bodies;
		mPhysicsSystem->GetBodies(bodies);
		const BodyLockInterface &bli = mPhysicsSystem->GetBodyLockInterface();
		for (BodyID b : bodies)
		{
			// Get the body
			BodyLockRead lock(bli, b);
			if (lock.SucceededAndIsInBroadPhase())
			{
				// Collect all leaf shapes for the body and their transforms
				const Body &body = lock.GetBody();
				AllHitCollisionCollector<TransformedShapeCollector> collector;
				body.GetTransformedShape().CollectTransformedShapes(body.GetWorldSpaceBounds(), collector);

				// Draw all leaf shapes
				for (const TransformedShape &transformed_shape : collector.mHits)
				{
					DebugRenderer::GeometryRef geometry;

					// Find geometry from previous frame
					ShapeToGeometryMap::iterator map_iterator = mShapeToGeometry.find(transformed_shape.mShape);
					if (map_iterator != mShapeToGeometry.end())
						geometry = map_iterator->second;

					if (geometry == nullptr)
					{
						// Find geometry from this frame
						map_iterator = shape_to_geometry.find(transformed_shape.mShape);
						if (map_iterator != shape_to_geometry.end())
							geometry = map_iterator->second;
					}

					if (geometry == nullptr)
					{
						// Geometry not cached
						Array<DebugRenderer::Triangle> triangles;

						// Start iterating all triangles of the shape
						Shape::GetTrianglesContext context;
						transformed_shape.mShape->GetTrianglesStart(context, AABox::sBiggest(), Vec3::sZero(), Quat::sIdentity(), Vec3::sReplicate(1.0f));
						for (;;)
						{
							// Get the next batch of vertices
							constexpr int cMaxTriangles = 1000;
							Float3 vertices[3 * cMaxTriangles];
							int triangle_count = transformed_shape.mShape->GetTrianglesNext(context, cMaxTriangles, vertices);
							if (triangle_count == 0)
								break;

							// Allocate space for triangles
							size_t output_index = triangles.size();
							triangles.resize(triangles.size() + triangle_count);
							DebugRenderer::Triangle *triangle = &triangles[output_index];

							// Convert to a renderable triangle
							for (int vertex = 0, vertex_max = 3 * triangle_count; vertex < vertex_max; vertex += 3, ++triangle)
							{
								// Get the vertices
								Vec3 v1(vertices[vertex + 0]);
								Vec3 v2(vertices[vertex + 1]);
								Vec3 v3(vertices[vertex + 2]);

								// Calculate the normal
								Float3 normal;
								(v2 - v1).Cross(v3 - v1).NormalizedOr(Vec3::sZero()).StoreFloat3(&normal);

								v1.StoreFloat3(&triangle->mV[0].mPosition);
								triangle->mV[0].mNormal = normal;
								triangle->mV[0].mColor = Color::sWhite;
								triangle->mV[0].mUV = Float2(0, 0);

								v2.StoreFloat3(&triangle->mV[1].mPosition);
								triangle->mV[1].mNormal = normal;
								triangle->mV[1].mColor = Color::sWhite;
								triangle->mV[1].mUV = Float2(0, 0);

								v3.StoreFloat3(&triangle->mV[2].mPosition);
								triangle->mV[2].mNormal = normal;
								triangle->mV[2].mColor = Color::sWhite;
								triangle->mV[2].mUV = Float2(0, 0);
							}
						}

						// Convert to geometry
						geometry = new DebugRenderer::Geometry(mDebugRenderer->CreateTriangleBatch(triangles), transformed_shape.mShape->GetLocalBounds());
					}

					// Ensure that we cache the geometry for next frame
					// Don't cache soft bodies as their shape changes every frame
					if (!body.IsSoftBody())
						shape_to_geometry[transformed_shape.mShape] = geometry;

					// Determine color
					Color color;
					switch (body.GetMotionType())
					{
					case EMotionType::Static:
						color = Color::sGrey;
						break;

					case EMotionType::Kinematic:
						color = Color::sGreen;
						break;

					case EMotionType::Dynamic:
						color = Color::sGetDistinctColor(body.GetID().GetIndex());
						break;

					default:
						JPH_ASSERT(false);
						color = Color::sBlack;
						break;
					}

					// Draw the geometry
					Vec3 scale = transformed_shape.GetShapeScale();
					bool inside_out = ScaleHelpers::IsInsideOut(scale);
					RMat44 matrix = transformed_shape.GetCenterOfMassTransform().PreScaled(scale);
					mDebugRenderer->DrawGeometry(matrix, color, geometry, inside_out? DebugRenderer::ECullMode::CullFrontFace : DebugRenderer::ECullMode::CullBackFace, DebugRenderer::ECastShadow::On, body.IsSensor()? DebugRenderer::EDrawMode::Wireframe : DebugRenderer::EDrawMode::Solid);
				}
			}
		}
	}

	// Replace the map with the newly created map so that shapes that we don't draw / were removed are released
	mShapeToGeometry = std::move(shape_to_geometry);
}

void SamplesApp::StepPhysics(JobSystem *inJobSystem)
{
	float delta_time = 1.0f / mUpdateFrequency;

	{
		// Pre update
		JPH_PROFILE("PrePhysicsUpdate");
		Test::PreUpdateParams pre_update;
		pre_update.mDeltaTime = delta_time;
		pre_update.mCameraState = GetCamera();
	#ifdef JPH_DEBUG_RENDERER
		pre_update.mPoseDrawSettings = &mPoseDrawSettings;
	#endif // JPH_DEBUG_RENDERER
		mTest->PrePhysicsUpdate(pre_update);
	}

	// Remember start time
	chrono::high_resolution_clock::time_point clock_start = chrono::high_resolution_clock::now();

	// Step the world (with fixed frequency)
	mPhysicsSystem->Update(delta_time, mCollisionSteps, mTempAllocator, inJobSystem);
#ifndef JPH_DISABLE_TEMP_ALLOCATOR
	JPH_ASSERT(static_cast<TempAllocatorImpl *>(mTempAllocator)->IsEmpty());
#endif // JPH_DISABLE_TEMP_ALLOCATOR

	// Accumulate time
	chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
	chrono::microseconds duration = chrono::duration_cast<chrono::microseconds>(clock_end - clock_start);
	mTotalTime += duration;
	mStepNumber++;

	// Print timing information
	constexpr uint cNumSteps = 60;
	if (mStepNumber % cNumSteps == 0)
	{
		Trace("Timing: %u, %llu", mStepNumber / cNumSteps, static_cast<unsigned long long>(mTotalTime.count()) / cNumSteps);
		mTotalTime = chrono::microseconds(0);
	}

#ifdef JPH_TRACK_BROADPHASE_STATS
	if (mStepNumber % 600 == 0)
		mPhysicsSystem->ReportBroadphaseStats();
#endif // JPH_TRACK_BROADPHASE_STATS

#ifdef JPH_TRACK_NARROWPHASE_STATS
	if (mStepNumber % 600 == 0)
		NarrowPhaseStat::sReportStats();
#endif // JPH_TRACK_NARROWPHASE_STATS

	{
		// Post update
		JPH_PROFILE("PostPhysicsUpdate");
		mTest->PostPhysicsUpdate(delta_time);
	}
}

void SamplesApp::SaveState(StateRecorderImpl &inStream)
{
	mTest->SaveState(inStream);

	if (mContactListener)
		mContactListener->SaveState(inStream);

	mPhysicsSystem->SaveState(inStream);
}

void SamplesApp::RestoreState(StateRecorderImpl &inStream)
{
	inStream.Rewind();

	// Restore the state of the test first, this is needed because the test can make changes to
	// the state of bodies that is not tracked by the PhysicsSystem::SaveState.
	// E.g. in the ChangeShapeTest the shape is restored here, which needs to be done first
	// because changing the shape changes Body::mPosition when the center of mass changes.
	mTest->RestoreState(inStream);

	if (mContactListener)
		mContactListener->RestoreState(inStream);

	if (!mPhysicsSystem->RestoreState(inStream))
		FatalError("Failed to restore physics state");
}

void SamplesApp::ValidateState(StateRecorderImpl &inExpectedState)
{
	// Save state
	StateRecorderImpl current_state;
	SaveState(current_state);

	// Compare state with expected state
	if (!current_state.IsEqual(inExpectedState))
	{
		// Mark this stream to break whenever it detects a memory change during reading
		inExpectedState.SetValidating(true);

		// Restore state. Anything that changes indicates a problem with the deterministic simulation.
		RestoreState(inExpectedState);

		// Turn change detection off again
		inExpectedState.SetValidating(false);
	}
}

void SamplesApp::GetInitialCamera(CameraState &ioState) const
{
	// Default if the test doesn't override it
	ioState.mPos = GetWorldScale() * RVec3(30, 10, 30);
	ioState.mForward = -Vec3(ioState.mPos).Normalized();
	ioState.mFarPlane = 1000.0f;

	mTest->GetInitialCamera(ioState);
}

RMat44 SamplesApp::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	return mTest->GetCameraPivot(inCameraHeading, inCameraPitch);
}

float SamplesApp::GetWorldScale() const
{
	return mTest != nullptr? mTest->GetWorldScale() : 1.0f;
}

ENTRY_POINT(SamplesApp, RegisterCustomMemoryHook)

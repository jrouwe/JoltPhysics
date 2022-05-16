// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <SamplesApp.h>
#include <Application/EntryPoint.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Geometry/OrientedBox.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
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
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/NarrowPhaseStats.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

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

JPH_DECLARE_RTTI_FOR_FACTORY(SimpleTest)
JPH_DECLARE_RTTI_FOR_FACTORY(StackTest)
JPH_DECLARE_RTTI_FOR_FACTORY(WallTest)
JPH_DECLARE_RTTI_FOR_FACTORY(IslandTest)
JPH_DECLARE_RTTI_FOR_FACTORY(FunnelTest)
JPH_DECLARE_RTTI_FOR_FACTORY(FrictionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(FrictionPerTriangleTest)
JPH_DECLARE_RTTI_FOR_FACTORY(GravityFactorTest)
JPH_DECLARE_RTTI_FOR_FACTORY(RestitutionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(DampingTest)
JPH_DECLARE_RTTI_FOR_FACTORY(KinematicTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ContactManifoldTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ManifoldReductionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(CenterOfMassTest)
JPH_DECLARE_RTTI_FOR_FACTORY(HeavyOnLightTest)
JPH_DECLARE_RTTI_FOR_FACTORY(HighSpeedTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ChangeMotionTypeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ChangeShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ChangeObjectLayerTest)
JPH_DECLARE_RTTI_FOR_FACTORY(LoadSaveSceneTest)
JPH_DECLARE_RTTI_FOR_FACTORY(LoadSaveBinaryTest)
JPH_DECLARE_RTTI_FOR_FACTORY(BigVsSmallTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ActiveEdgesTest)
JPH_DECLARE_RTTI_FOR_FACTORY(MultithreadedTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ContactListenerTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ActivateDuringUpdateTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SensorTest)

static TestNameAndRTTI sGeneralTests[] =
{
	{ "Simple",								JPH_RTTI(SimpleTest) },
	{ "Stack",								JPH_RTTI(StackTest) },
	{ "Wall",								JPH_RTTI(WallTest) },
	{ "Island",								JPH_RTTI(IslandTest) },
	{ "Funnel",								JPH_RTTI(FunnelTest) },
	{ "Friction",							JPH_RTTI(FrictionTest) },
	{ "Friction (Per Triangle)",			JPH_RTTI(FrictionPerTriangleTest) },
	{ "Gravity Factor",						JPH_RTTI(GravityFactorTest) },
	{ "Restitution",						JPH_RTTI(RestitutionTest) },
	{ "Damping",							JPH_RTTI(DampingTest) },
	{ "Kinematic",							JPH_RTTI(KinematicTest) },
	{ "Contact Manifold",					JPH_RTTI(ContactManifoldTest) },
	{ "Manifold Reduction",					JPH_RTTI(ManifoldReductionTest) },
	{ "Center Of Mass",						JPH_RTTI(CenterOfMassTest) },
	{ "Heavy On Light",						JPH_RTTI(HeavyOnLightTest) },
	{ "High Speed",							JPH_RTTI(HighSpeedTest) },
	{ "Change Motion Type",					JPH_RTTI(ChangeMotionTypeTest) },
	{ "Change Shape",						JPH_RTTI(ChangeShapeTest) },
	{ "Change Object Layer",				JPH_RTTI(ChangeObjectLayerTest) },
	{ "Load/Save Scene",					JPH_RTTI(LoadSaveSceneTest) },
	{ "Load/Save Binary",					JPH_RTTI(LoadSaveBinaryTest) },
	{ "Big vs Small",						JPH_RTTI(BigVsSmallTest) },
	{ "Active Edges",						JPH_RTTI(ActiveEdgesTest) },
	{ "Multithreaded",						JPH_RTTI(MultithreadedTest) },
	{ "Contact Listener",					JPH_RTTI(ContactListenerTest) },
	{ "Activate During Update",				JPH_RTTI(ActivateDuringUpdateTest) },
	{ "Sensor",								JPH_RTTI(SensorTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(DistanceConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(FixedConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ConeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SwingTwistConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SixDOFConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(HingeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PoweredHingeConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PointConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SliderConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PoweredSliderConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SpringTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ConstraintSingularityTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PoweredSwingTwistConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SwingTwistConstraintFrictionTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PathConstraintTest)

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
	{ "Spring",								JPH_RTTI(SpringTest) },
	{ "Constraint Singularity",				JPH_RTTI(ConstraintSingularityTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(BoxShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(SphereShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(TaperedCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(CapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(CylinderShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(StaticCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(MutableCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(TriangleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ConvexHullShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(MeshShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(HeightFieldShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(RotatedTranslatedShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(OffsetCenterOfMassShapeTest)

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
	{ "Static Compound Shape",				JPH_RTTI(StaticCompoundShapeTest) },
	{ "Mutable Compound Shape",				JPH_RTTI(MutableCompoundShapeTest) },
	{ "Triangle Shape",						JPH_RTTI(TriangleShapeTest) },
	{ "Rotated Translated Shape",			JPH_RTTI(RotatedTranslatedShapeTest) },
	{ "Offset Center Of Mass Shape",		JPH_RTTI(OffsetCenterOfMassShapeTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(ScaledSphereShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledBoxShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledTaperedCapsuleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledCylinderShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledConvexHullShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledMeshShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledHeightFieldShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledStaticCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledMutableCompoundShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledTriangleShapeTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ScaledOffsetCenterOfMassShapeTest)

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
	{ "Offset Center Of Mass Shape",		JPH_RTTI(ScaledOffsetCenterOfMassShapeTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(CreateRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(LoadRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(KinematicRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(PoweredRigTest)
JPH_DECLARE_RTTI_FOR_FACTORY(RigPileTest)
JPH_DECLARE_RTTI_FOR_FACTORY(LoadSaveBinaryRigTest)

static TestNameAndRTTI sRigTests[] =
{
	{ "Create Rig",							JPH_RTTI(CreateRigTest) },
	{ "Load Rig",							JPH_RTTI(LoadRigTest) },
	{ "Load / Save Binary Rig",				JPH_RTTI(LoadSaveBinaryRigTest) },
	{ "Kinematic Rig",						JPH_RTTI(KinematicRigTest) },
	{ "Powered Rig",						JPH_RTTI(PoweredRigTest) },
	{ "Rig Pile",							JPH_RTTI(RigPileTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(CharacterTest)
JPH_DECLARE_RTTI_FOR_FACTORY(CharacterVirtualTest)

static TestNameAndRTTI sCharacterTests[] =
{
	{ "Character",							JPH_RTTI(CharacterTest) },
	{ "Character Virtual",					JPH_RTTI(CharacterVirtualTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(WaterShapeTest)

static TestNameAndRTTI sWaterTests[] =
{
	{ "Shapes",								JPH_RTTI(WaterShapeTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(VehicleSixDOFTest)
JPH_DECLARE_RTTI_FOR_FACTORY(VehicleConstraintTest)
JPH_DECLARE_RTTI_FOR_FACTORY(TankTest)

static TestNameAndRTTI sVehicleTests[] =
{
	{ "Car (VehicleConstraint)",			JPH_RTTI(VehicleConstraintTest) },
	{ "Tank (VehicleConstraint)",			JPH_RTTI(TankTest) },
	{ "Car (SixDOFConstraint)",				JPH_RTTI(VehicleSixDOFTest) },
};

JPH_DECLARE_RTTI_FOR_FACTORY(BroadPhaseCastRayTest)
JPH_DECLARE_RTTI_FOR_FACTORY(BroadPhaseInsertionTest)

static TestNameAndRTTI sBroadPhaseTests[] =
{
	{ "Cast Ray",							JPH_RTTI(BroadPhaseCastRayTest) },
	{ "Insertion",							JPH_RTTI(BroadPhaseInsertionTest) }
};

JPH_DECLARE_RTTI_FOR_FACTORY(InteractivePairsTest)
JPH_DECLARE_RTTI_FOR_FACTORY(EPATest)
JPH_DECLARE_RTTI_FOR_FACTORY(ClosestPointTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ConvexHullTest)
JPH_DECLARE_RTTI_FOR_FACTORY(ConvexHullShrinkTest)
JPH_DECLARE_RTTI_FOR_FACTORY(RandomRayTest)
JPH_DECLARE_RTTI_FOR_FACTORY(CapsuleVsBoxTest)

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

JPH_DECLARE_RTTI_FOR_FACTORY(LoadSnapshotTest)

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
static constexpr uint cMaxContactConstraints = 10240;

SamplesApp::SamplesApp()
{
	// Allocate temp memory
#ifdef JPH_DISABLE_TEMP_ALLOCATOR
	mTempAllocator = new TempAllocatorMalloc();
#else
	mTempAllocator = new TempAllocatorImpl(16 * 1024 * 1024);
#endif

	// Create job system
	mJobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, mMaxConcurrentJobs - 1);

	// Create job system without extra threads for validating
	mJobSystemValidating = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, 0);

	// Create UI
	UIElement *main_menu = mDebugUI->CreateMenu();
	mDebugUI->CreateTextButton(main_menu, "Select Test", [this]() { 
		UIElement *tests = mDebugUI->CreateMenu();
		for (TestCategory &c : sAllCategories)
		{
			mDebugUI->CreateTextButton(tests, c.mName, [=]() { 
				UIElement *category = mDebugUI->CreateMenu();
				for (uint j = 0; j < c.mNumTests; ++j)
					mDebugUI->CreateTextButton(category, c.mTests[j].mName, [=]() { StartTest(c.mTests[j].mRTTI); });
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
		mDebugUI->CreateSlider(phys_settings, "Update Frequency (Hz)", mUpdateFrequency, 7.5f, 120.0f, 2.5f, [this](float inValue) { mUpdateFrequency = inValue; });
		mDebugUI->CreateSlider(phys_settings, "Num Collision Steps", float(mCollisionSteps), 1.0f, 4.0f, 1.0f, [this](float inValue) { mCollisionSteps = int(inValue); });
		mDebugUI->CreateSlider(phys_settings, "Num Integration Sub Steps", float(mIntegrationSubSteps), 1.0f, 4.0f, 1.0f, [this](float inValue) { mIntegrationSubSteps = int(inValue); });
		mDebugUI->CreateSlider(phys_settings, "Num Velocity Steps", float(mPhysicsSettings.mNumVelocitySteps), 0, 30, 1, [this](float inValue) { mPhysicsSettings.mNumVelocitySteps = int(round(inValue)); mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateSlider(phys_settings, "Num Position Steps", float(mPhysicsSettings.mNumPositionSteps), 0, 30, 1, [this](float inValue) { mPhysicsSettings.mNumPositionSteps = int(round(inValue)); mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateSlider(phys_settings, "Baumgarte Stabilization Factor", mPhysicsSettings.mBaumgarte, 0.01f, 1.0f, 0.05f, [this](float inValue) { mPhysicsSettings.mBaumgarte = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateSlider(phys_settings, "Speculative Contact Distance (m)", mPhysicsSettings.mSpeculativeContactDistance, 0.0f, 0.1f, 0.005f, [this](float inValue) { mPhysicsSettings.mSpeculativeContactDistance = inValue; });
		mDebugUI->CreateSlider(phys_settings, "Penetration Slop (m)", mPhysicsSettings.mPenetrationSlop, 0.0f, 0.1f, 0.005f, [this](float inValue) { mPhysicsSettings.mPenetrationSlop = inValue; });
		mDebugUI->CreateSlider(phys_settings, "Min Velocity For Restitution (m/s)", mPhysicsSettings.mMinVelocityForRestitution, 0.0f, 10.0f, 0.1f, [this](float inValue) { mPhysicsSettings.mMinVelocityForRestitution = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateSlider(phys_settings, "Time Before Sleep (s)", mPhysicsSettings.mTimeBeforeSleep, 0.1f, 1.0f, 0.1f, [this](float inValue) { mPhysicsSettings.mTimeBeforeSleep = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateSlider(phys_settings, "Point Velocity Sleep Threshold (m/s)", mPhysicsSettings.mPointVelocitySleepThreshold, 0.01f, 1.0f, 0.01f, [this](float inValue) { mPhysicsSettings.mPointVelocitySleepThreshold = inValue; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateCheckBox(phys_settings, "Constraint Warm Starting", mPhysicsSettings.mConstraintWarmStart, [this](UICheckBox::EState inState) { mPhysicsSettings.mConstraintWarmStart = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateCheckBox(phys_settings, "Use Body Pair Contact Cache", mPhysicsSettings.mUseBodyPairContactCache, [this](UICheckBox::EState inState) { mPhysicsSettings.mUseBodyPairContactCache = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
		mDebugUI->CreateCheckBox(phys_settings, "Contact Manifold Reduction", mPhysicsSettings.mUseManifoldReduction, [this](UICheckBox::EState inState) { mPhysicsSettings.mUseManifoldReduction = inState == UICheckBox::STATE_CHECKED; mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings); });
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
		mDebugUI->CreateCheckBox(drawing_options, "Draw Center of Mass Transforms", mBodyDrawSettings.mDrawCenterOfMassTransform, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawCenterOfMassTransform = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw World Transforms", mBodyDrawSettings.mDrawWorldTransform, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawWorldTransform = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Velocity", mBodyDrawSettings.mDrawVelocity, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawVelocity = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Sleep Stats", mBodyDrawSettings.mDrawSleepStats, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawSleepStats = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Mass and Inertia (I)", mBodyDrawSettings.mDrawMassAndInertia, [this](UICheckBox::EState inState) { mBodyDrawSettings.mDrawMassAndInertia = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Joints", mPoseDrawSettings.mDrawJoints, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJoints = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Joint Orientations", mPoseDrawSettings.mDrawJointOrientations, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJointOrientations = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Joint Names", mPoseDrawSettings.mDrawJointNames, [this](UICheckBox::EState inState) { mPoseDrawSettings.mDrawJointNames = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Mesh Shape Triangle Groups", MeshShape::sDrawTriangleGroups, [](UICheckBox::EState inState) { MeshShape::sDrawTriangleGroups = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Mesh Shape Triangle Outlines", MeshShape::sDrawTriangleOutlines, [](UICheckBox::EState inState) { MeshShape::sDrawTriangleOutlines = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Height Field Shape Triangle Outlines", HeightFieldShape::sDrawTriangleOutlines, [](UICheckBox::EState inState) { HeightFieldShape::sDrawTriangleOutlines = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Submerged Volumes", Shape::sDrawSubmergedVolumes, [](UICheckBox::EState inState) { Shape::sDrawSubmergedVolumes = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Character Virtual Constraints", CharacterVirtual::sDrawConstraints, [](UICheckBox::EState inState) { CharacterVirtual::sDrawConstraints = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(drawing_options, "Draw Character Virtual Walk Stairs", CharacterVirtual::sDrawWalkStairs, [](UICheckBox::EState inState) { CharacterVirtual::sDrawWalkStairs = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->ShowMenu(drawing_options);
	});
#endif // JPH_DEBUG_RENDERER
	mDebugUI->CreateTextButton(main_menu, "Mouse Probe", [this]() { 
		UIElement *probe_options = mDebugUI->CreateMenu();
		mDebugUI->CreateComboBox(probe_options, "Mode", { "Pick", "Ray", "RayCollector", "CollidePoint", "CollideShape", "CastShape", "TransfShape", "GetTriangles", "BP Ray", "BP Box", "BP Sphere", "BP Point", "BP OBox", "BP Cast Box" }, (int)mProbeMode, [this](int inItem) { mProbeMode = (EProbeMode)inItem; });
		mDebugUI->CreateComboBox(probe_options, "Shape", { "Sphere", "Box", "ConvexHull", "Capsule", "TaperedCapsule", "Cylinder", "Triangle", "StaticCompound", "StaticCompound2", "MutableCompound" }, (int)mProbeShape, [=](int inItem) { mProbeShape = (EProbeShape)inItem; });
		mDebugUI->CreateCheckBox(probe_options, "Scale Shape", mScaleShape, [this](UICheckBox::EState inState) { mScaleShape = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateSlider(probe_options, "Scale X", mShapeScale.GetX(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetX(inValue); });
		mDebugUI->CreateSlider(probe_options, "Scale Y", mShapeScale.GetY(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetY(inValue); });
		mDebugUI->CreateSlider(probe_options, "Scale Z", mShapeScale.GetZ(), -5.0f, 5.0f, 0.1f, [this](float inValue) { mShapeScale.SetZ(inValue); });
		mDebugUI->CreateComboBox(probe_options, "Back Face Cull", { "On", "Off" }, (int)mBackFaceMode, [=](int inItem) { mBackFaceMode = (EBackFaceMode)inItem; });
		mDebugUI->CreateComboBox(probe_options, "Active Edge Mode", { "Only Active", "All" }, (int)mActiveEdgeMode, [=](int inItem) { mActiveEdgeMode = (EActiveEdgeMode)inItem; });
		mDebugUI->CreateComboBox(probe_options, "Collect Faces Mode", { "Collect Faces", "No Faces" }, (int)mCollectFacesMode, [=](int inItem) { mCollectFacesMode = (ECollectFacesMode)inItem; });
		mDebugUI->CreateSlider(probe_options, "Max Separation Distance", mMaxSeparationDistance, 0.0f, 5.0f, 0.1f, [this](float inValue) { mMaxSeparationDistance = inValue; });
		mDebugUI->CreateCheckBox(probe_options, "Treat Convex As Solid", mTreatConvexAsSolid, [this](UICheckBox::EState inState) { mTreatConvexAsSolid = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(probe_options, "Return Deepest Point", mReturnDeepestPoint, [this](UICheckBox::EState inState) { mReturnDeepestPoint = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateCheckBox(probe_options, "Shrunken Shape + Convex Radius", mUseShrunkenShapeAndConvexRadius, [this](UICheckBox::EState inState) { mUseShrunkenShapeAndConvexRadius = inState == UICheckBox::STATE_CHECKED; });
		mDebugUI->CreateSlider(probe_options, "Max Hits", float(mMaxHits), 0, 10, 1, [this](float inValue) { mMaxHits = (int)inValue; });
		mDebugUI->ShowMenu(probe_options);
	});
	mDebugUI->CreateTextButton(main_menu, "Shoot Object", [this]() { 
		UIElement *shoot_options = mDebugUI->CreateMenu();
		mDebugUI->CreateTextButton(shoot_options, "Shoot Object (B)", [=]() { ShootObject(); });
		mDebugUI->CreateSlider(shoot_options, "Initial Velocity", mShootObjectVelocity, 0.0f, 500.0f, 10.0f, [this](float inValue) { mShootObjectVelocity = inValue; });
		mDebugUI->CreateComboBox(shoot_options, "Shape", { "Sphere", "ConvexHull", "Thin Bar" }, (int)mShootObjectShape, [=](int inItem) { mShootObjectShape = (EShootObjectShape)inItem; });
		mDebugUI->CreateComboBox(shoot_options, "Motion Quality", { "Discrete", "LinearCast" }, (int)mShootObjectMotionQuality, [=](int inItem) { mShootObjectMotionQuality = (EMotionQuality)inItem; });
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

	// Get test name from commandline
	string cmd_line = ToLower(GetCommandLineA());
	vector<string> args;
	StringToVector(cmd_line, args, " ");
	if (args.size() == 2)
	{
		string cmd = args[1];
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
			const RTTI *test = JPH_RTTI(LoadRigTest);
			for (TestCategory &c : sAllCategories)
				for (uint i = 0; i < c.mNumTests; ++i)
				{
					TestNameAndRTTI &t = c.mTests[i];
					string test_name = ToLower(t.mRTTI->GetName());
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
	mPhysicsSystem->Init(cNumBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, mBroadPhaseLayerInterface, BroadPhaseCanCollide, ObjectCanCollide);
	mPhysicsSystem->SetPhysicsSettings(mPhysicsSettings);

	// Restore gravity
	mPhysicsSystem->SetGravity(old_gravity);

	// Reset dragging
	mDragAnchor = nullptr;
	mDragConstraint = nullptr;

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
		mStatusString = StringFormat("%s: Next test in %.1fs", mTestClass->GetName(), (double)mTestTimeLeft);

		// Use physics time
		mTestTimeLeft -= 1.0f / mUpdateFrequency;

		// If time's up then go to the next test
		if (mTestTimeLeft < 0.0f)
			return NextTest();
	}
	else
		mStatusString.clear();

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
	// Get the scale
	Vec3 scale = mScaleShape? mShapeScale : Vec3::sReplicate(1.0f);

	// Make it minimally -0.1 or 0.1 depending on the sign
	Vec3 clamped_value = Vec3::sSelect(Vec3::sReplicate(-0.1f), Vec3::sReplicate(0.1f), Vec3::sGreaterOrEqual(scale, Vec3::sZero()));
	scale = Vec3::sSelect(scale, clamped_value, Vec3::sLess(scale.Abs(), Vec3::sReplicate(0.1f)));

	RefConst<Shape> shape;
	switch (mProbeShape)
	{
	case EProbeShape::Sphere:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
		shape = new SphereShape(0.2f);
		break;

	case EProbeShape::Box:
		shape = new BoxShape(Vec3(0.1f, 0.2f, 0.3f));
		break;

	case EProbeShape::ConvexHull:
		{
			// Create tetrahedron
			vector<Vec3> tetrahedron;
			tetrahedron.push_back(Vec3::sZero());
			tetrahedron.push_back(Vec3(0.2f, 0, 0.4f));
			tetrahedron.push_back(Vec3(0.4f, 0, 0));
			tetrahedron.push_back(Vec3(0.2f, -0.2f, 1.0f));
			shape = ConvexHullShapeSettings(tetrahedron, 0.01f).Create().Get();
		}
		break;

	case EProbeShape::Capsule:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
		shape = new CapsuleShape(0.2f, 0.1f);
		break;

	case EProbeShape::TaperedCapsule:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
		shape = TaperedCapsuleShapeSettings(0.2f, 0.1f, 0.2f).Create().Get();
		break;

	case EProbeShape::Cylinder:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>(); // Scale X must be same as Z
		shape = new CylinderShape(0.2f, 0.1f);
		break;

	case EProbeShape::Triangle:
		scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
		shape = new TriangleShape(Vec3(0.1f, 0.9f, 0.3f), Vec3(-0.9f, -0.5f, 0.2f), Vec3(0.7f, -0.3f, -0.1f));
		break;

	case EProbeShape::StaticCompound:
		{
			vector<Vec3> tetrahedron;
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
			scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
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
			vector<Vec3> tetrahedron;
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
	}

	JPH_ASSERT(shape != nullptr);

	// Scale the shape
	if (scale != Vec3::sReplicate(1.0f))
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
			vector<Vec3> vertices = {
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
	}

	// Scale shape if needed
	if (scale != Vec3::sReplicate(1.0f))
		shape = new ScaledShape(shape, scale);

	return shape;
}

void SamplesApp::ShootObject()
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

bool SamplesApp::CastProbe(float inProbeLength, float &outFraction, Vec3 &outPosition, BodyID &outID)
{
	const CameraState &camera = GetCamera();
	Vec3 start = camera.mPos;
	Vec3 direction = inProbeLength * camera.mForward;

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
			RayCast ray { start, direction };

			// Cast ray
			RayCastResult hit;
			had_hit = mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), SpecifiedObjectLayerFilter(Layers::MOVING));

			// Fill in results
			outPosition = start + hit.mFraction * direction;
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
			RayCast ray { start, direction };

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
			RayCast ray { start, direction };

			// Create settings
			RayCastSettings settings;
			settings.mBackFaceMode = mBackFaceMode;
			settings.mTreatConvexAsSolid = mTreatConvexAsSolid;

			// Cast ray
			vector<RayCastResult> hits;
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
				outPosition = start + first_hit.mFraction * direction;
				outFraction = first_hit.mFraction;
				outID = first_hit.mBodyID;
	
				// Draw results
				Vec3 prev_position = start;
				bool c = false;
				for (const RayCastResult &hit : hits)
				{
					// Draw line
					Vec3 position = ray.GetPointOnRay(hit.mFraction);
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
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(start + hits.back().mFraction * direction, start + direction, Color::sRed);
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
			Vec3 point = start + fraction * direction;

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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawMarker(start + fraction * direction, had_hit? Color::sGreen : Color::sRed, 0.1f);
		}
		break;

	case EProbeMode::CollideShape:
		{
			// Create shape cast
			RefConst<Shape> shape = CreateProbeShape();
			Mat44 rotation = Mat44::sRotation(Vec3::sAxisX(), 0.1f * JPH_PI) * Mat44::sRotation(Vec3::sAxisY(), 0.2f * JPH_PI);
			Mat44 com = Mat44::sTranslation(shape->GetCenterOfMass());
			Mat44 shape_transform = Mat44::sTranslation(start + 5.0f * camera.mForward) * rotation * com;

			// Create settings
			CollideShapeSettings settings;
			settings.mActiveEdgeMode = mActiveEdgeMode;
			settings.mBackFaceMode = mBackFaceMode;
			settings.mCollectFacesMode = mCollectFacesMode;
			settings.mMaxSeparationDistance = mMaxSeparationDistance;

			vector<CollideShapeResult> hits;
			if (mMaxHits == 0)
			{
				AnyHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else if (mMaxHits == 1)
			{
				ClosestHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else
			{
				AllHitCollisionCollector<CollideShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CollideShape(shape, Vec3::sReplicate(1.0f), shape_transform, settings, collector);
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
						mDebugRenderer->DrawMarker(hit.mContactPointOn1, Color::sGreen, 0.1f);
						mDebugRenderer->DrawMarker(hit.mContactPointOn2, Color::sRed, 0.1f);

						Vec3 pen_axis = hit.mPenetrationAxis;
						float pen_axis_len = pen_axis.Length();
						if (pen_axis_len > 0.0f)
						{
							pen_axis /= pen_axis_len;

							// Draw penetration axis with length of the penetration
							mDebugRenderer->DrawArrow(hit.mContactPointOn2, hit.mContactPointOn2 + pen_axis * hit.mPenetrationDepth, Color::sYellow, 0.01f);

							// Draw normal (flipped so it points towards body 1)
							mDebugRenderer->DrawArrow(hit.mContactPointOn2, hit.mContactPointOn2 - pen_axis, Color::sOrange, 0.01f);
						}

						// Draw material
						const PhysicsMaterial *material2 = hit_body.GetShape()->GetMaterial(hit.mSubShapeID2);
						mDebugRenderer->DrawText3D(hit.mContactPointOn2, material2->GetDebugName());

						// Draw faces
						mDebugRenderer->DrawWirePolygon(hit.mShape1Face, Color::sYellow, 0.01f);
						mDebugRenderer->DrawWirePolygon(hit.mShape2Face, Color::sRed, 0.01f);
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
			ShapeCast shape_cast = ShapeCast::sFromWorldTransform(shape, Vec3::sReplicate(1.0f), Mat44::sTranslation(start) * rotation, direction);

			// Settings
			ShapeCastSettings settings;
			settings.mUseShrunkenShapeAndConvexRadius = mUseShrunkenShapeAndConvexRadius;
			settings.mActiveEdgeMode = mActiveEdgeMode;
			settings.mBackFaceModeTriangles = mBackFaceMode;
			settings.mBackFaceModeConvex = mBackFaceMode;
			settings.mReturnDeepestPoint = mReturnDeepestPoint;
			settings.mCollectFacesMode = mCollectFacesMode;

			// Cast shape
			vector<ShapeCastResult> hits;
			if (mMaxHits == 0)
			{
				AnyHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else if (mMaxHits == 1)
			{
				ClosestHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
				if (collector.HadHit())
					hits.push_back(collector.mHit);
			}
			else
			{
				AllHitCollisionCollector<CastShapeCollector> collector;
				mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
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
				outPosition = start + first_hit.mFraction * direction;
				outFraction = first_hit.mFraction;
				outID = first_hit.mBodyID2;

				// Draw results
				Vec3 prev_position = start;
				bool c = false;
				for (const ShapeCastResult &hit : hits)
				{
					// Draw line
					Vec3 position = start + hit.mFraction * direction;
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
						Vec3 contact_position1 = hit.mContactPointOn1;
						Vec3 contact_position2 = hit.mContactPointOn2;
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
						mDebugRenderer->DrawWirePolygon(hit.mShape1Face, Color::sYellow, 0.01f);
						mDebugRenderer->DrawWirePolygon(hit.mShape2Face, Color::sRed, 0.01f);
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(start + hits.back().mFraction * direction, start + direction, Color::sRed);
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

	case EProbeMode::TransformedShape:
		{
			// Create box
			const float fraction = 0.2f;
			Vec3 center = start + fraction * direction;
			Vec3 half_extent = 0.5f * mShapeScale;
			AABox box(center - half_extent, center + half_extent);

			// Get shapes
			AllHitCollisionCollector<TransformedShapeCollector> collector;
			mPhysicsSystem->GetNarrowPhaseQuery().CollectTransformedShapes(box, collector);

			// Draw results
			for (const TransformedShape &ts : collector.mHits)
				mDebugRenderer->DrawWireBox(Mat44::sRotationTranslation(ts.mShapeRotation, ts.mShapePositionCOM) * Mat44::sScale(ts.GetShapeScale()), ts.mShape->GetLocalBounds(), Color::sYellow);

			// Draw test location
			mDebugRenderer->DrawWireBox(box, !collector.mHits.empty()? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::GetTriangles:
		{
			// Create box
			const float fraction = 0.2f;
			Vec3 center = start + fraction * direction;
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
				ts.GetTrianglesStart(ctx, box);
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
						Vec3 v1(v[0]), v2(v[1]), v3(v[2]);
						Vec3 triangle_center = (v1 + v2 + v3) / 3.0f;
						Vec3 triangle_normal = (v2 - v1).Cross(v3 - v1).Normalized();
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
			RayCast ray { start, direction };

			// Cast ray
			AllHitCollisionCollector<RayCastBodyCollector> collector;
			mPhysicsSystem->GetBroadPhaseQuery().CastRay(ray, collector);
			collector.Sort();

			had_hit = !collector.mHits.empty();
			if (had_hit)
			{
				// Draw results
				Vec3 prev_position = start;
				bool c = false;
				for (const BroadPhaseCastResult &hit : collector.mHits)
				{
					// Draw line
					Vec3 position = start + hit.mFraction * direction;
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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
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
			Vec3 center = start + fraction * direction;
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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
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
			Vec3 point = start + fraction * direction;

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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawWireSphere(start + fraction * direction, radius, had_hit? Color::sGreen : Color::sRed);
		}
		break;

	case EProbeMode::BroadPhasePoint:
		{
			// Create point
			const float fraction = 0.1f;
			Vec3 point = start + fraction * direction;

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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
					}
				}
			}

			// Draw test location
			mDebugRenderer->DrawMarker(start + fraction * direction, had_hit? Color::sGreen : Color::sRed, 0.1f);
		}
		break;

	case EProbeMode::BroadPhaseOrientedBox:
		{
			// Create box
			const float fraction = 0.2f;
			Vec3 center = start + fraction * direction;
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
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
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
				Vec3 prev_position = start;
				bool c = false;
				for (const BroadPhaseCastResult &hit : collector.mHits)
				{
					// Draw line
					Vec3 position = start + hit.mFraction * direction;
					Color cast_color = c? Color::sGrey : Color::sWhite;
					mDebugRenderer->DrawLine(prev_position, position, cast_color);
					mDebugRenderer->DrawWireBox(AABox(position - half_extent, position + half_extent), cast_color);
					c = !c;
					prev_position = position;

					BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
					if (lock.Succeeded())
					{
						const Body &hit_body = lock.GetBody();

						// Draw bounding box
						Color color = hit_body.IsDynamic()? Color::sYellow : Color::sOrange;
						mDebugRenderer->DrawWireBox(hit_body.GetWorldSpaceBounds(), color);
					}
				}

				// Draw remainder of line
				mDebugRenderer->DrawLine(start + collector.mHits.back().mFraction * direction, start + direction, Color::sRed);
			}
			else
			{
				// Draw 'miss'
				mDebugRenderer->DrawLine(start, start + direction, Color::sRed);
				mDebugRenderer->DrawWireBox(AABox(start + direction - half_extent, start + direction + half_extent), Color::sRed);
			}
		}
		break;
	}

	return had_hit;
}

void SamplesApp::UpdateDebug()
{
	JPH_PROFILE_FUNCTION();

	const float cDragRayLength = 40.0f;

	BodyInterface &bi = mPhysicsSystem->GetBodyInterface();
			
	// Handle keyboard input for which simulation needs to be running
	for (int key = mKeyboard->GetFirstKey(); key != 0; key = mKeyboard->GetNextKey())
		switch (key)
		{
		case DIK_B:
			ShootObject();
			break;
		}

	// Allow the user to drag rigid bodies around
	if (mDragConstraint == nullptr)
	{
		// Not dragging yet
		Vec3 hit_position;
		float hit_fraction;
		if (CastProbe(cDragRayLength, hit_fraction, hit_position, mDragBody))
		{
			// If key is pressed create constraint to start dragging
			if (mKeyboard->IsKeyPressed(DIK_SPACE))
			{
				// Target body must be dynamic
				BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), mDragBody);
				if (lock.Succeeded())
				{
					Body &drag_body = lock.GetBody();
					if (drag_body.IsDynamic())
					{
						// Create constraint to drag body
						DistanceConstraintSettings settings;
						settings.mPoint1 = settings.mPoint2 = hit_position;
						settings.mFrequency = 2.0f / GetWorldScale();
						settings.mDamping = 1.0f;

						// Construct fixed body for the mouse constraint
						// Note that we don't add it to the world since we don't want anything to collide with it, we just
						// need an anchor for a constraint
						Body *drag_anchor = bi.CreateBody(BodyCreationSettings(new SphereShape(0.01f), hit_position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
						mDragAnchor = drag_anchor;

						// Construct constraint that connects the drag anchor with the body that we want to drag
						mDragConstraint = settings.Create(*drag_anchor, drag_body);
						mPhysicsSystem->AddConstraint(mDragConstraint);

						mDragFraction = hit_fraction;
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
				mPhysicsSystem->RemoveConstraint(mDragConstraint);
			mDragConstraint = nullptr;

			// Destroy drag anchor
			bi.DestroyBody(mDragAnchor->GetID());
			mDragAnchor = nullptr;

			// Forget the drag body
			mDragBody = BodyID();
		}
		else
		{
			// Else update position of anchor
			bi.SetPositionAndRotation(mDragAnchor->GetID(), GetCamera().mPos + cDragRayLength * mDragFraction * GetCamera().mForward, Quat::sIdentity(), EActivation::DontActivate);

			// Activate other body
			bi.ActivateBody(mDragBody);
		}
	}
}

bool SamplesApp::RenderFrame(float inDeltaTime)
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
			StateRecorderImpl &recorder = mPlaybackFrames[mCurrentPlaybackFrame];
			RestoreState(recorder);
				
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
				ValidateState(mPlaybackFrames[mCurrentPlaybackFrame + 1]);
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
			UpdateDebug();

			if (mRecordState || check_determinism)
			{
				// Record the state prior to the step
				mPlaybackFrames.push_back(StateRecorderImpl());
				SaveState(mPlaybackFrames.back());
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
				RestoreState(mPlaybackFrames.back());

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
						vector<DebugRenderer::Triangle> triangles;

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
					Mat44 matrix = transformed_shape.GetCenterOfMassTransform() * Mat44::sScale(scale);
					mDebugRenderer->DrawGeometry(matrix, color, geometry, inside_out? DebugRenderer::ECullMode::CullFrontFace : DebugRenderer::ECullMode::CullBackFace, DebugRenderer::ECastShadow::On, body.IsSensor()? DebugRenderer::EDrawMode::Wireframe : DebugRenderer::EDrawMode::Solid);
				}
			}
		}
	}

	// Replace the map with the newly created map so that shapes that we don't draw / were removed are released
	mShapeToGeometry = move(shape_to_geometry);
}

void SamplesApp::StepPhysics(JobSystem *inJobSystem)
{
	float delta_time = 1.0f / mUpdateFrequency;

	{
		// Pre update
		JPH_PROFILE("PrePhysicsUpdate");
		Test::PreUpdateParams pre_update;
		pre_update.mDeltaTime = delta_time;
		pre_update.mKeyboard = mKeyboard;
		pre_update.mCameraState = GetCamera();
	#ifdef JPH_DEBUG_RENDERER
		pre_update.mPoseDrawSettings = &mPoseDrawSettings;
	#endif // JPH_DEBUG_RENDERER
		mTest->PrePhysicsUpdate(pre_update);
	}

	// Remember start tick
	uint64 start_tick = GetProcessorTickCount();

	// Step the world (with fixed frequency)
	mPhysicsSystem->Update(delta_time, mCollisionSteps, mIntegrationSubSteps, mTempAllocator, inJobSystem);
#ifndef JPH_DISABLE_TEMP_ALLOCATOR
	JPH_ASSERT(static_cast<TempAllocatorImpl *>(mTempAllocator)->IsEmpty());
#endif // JPH_DISABLE_TEMP_ALLOCATOR

	// Accumulate time
	mTotalTime += GetProcessorTickCount() - start_tick;
	mStepNumber++;

	// Print timing information
	constexpr int cNumSteps = 60;
	if (mStepNumber % cNumSteps == 0)
	{
		double us_per_step = double(mTotalTime / cNumSteps) / double(GetProcessorTicksPerSecond()) * 1.0e6;
		Trace("Timing: %d, %.0f", mStepNumber / cNumSteps, us_per_step);
		mTotalTime = 0;
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
	ioState.mPos = GetWorldScale() * Vec3(30, 10, 30);
	ioState.mForward = -ioState.mPos.Normalized();
	ioState.mFarPlane = 1000.0f;

	mTest->GetInitialCamera(ioState);
}

Mat44 SamplesApp::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{ 
	return mTest->GetCameraPivot(inCameraHeading, inCameraPitch); 
}

float SamplesApp::GetWorldScale() const
{ 
	return mTest != nullptr? mTest->GetWorldScale() : 1.0f; 
}

ENTRY_POINT(SamplesApp)

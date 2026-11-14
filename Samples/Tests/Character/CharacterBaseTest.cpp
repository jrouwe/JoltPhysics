// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>
#include <Renderer/DebugRendererImp.h>
#include <Input/Keyboard.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(CharacterBaseTest)
{
	JPH_ADD_BASE_CLASS(CharacterBaseTest, Test)
}

const char *CharacterBaseTest::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"ObstacleCourse",
	"InitiallyIntersecting",
	"InitiallyIntersecting2",
#ifdef JPH_OBJECT_STREAM
	"Terrain1",
	"Terrain2",
#endif // JPH_OBJECT_STREAM
};

const char *CharacterBaseTest::sSceneName = "ObstacleCourse";

// Scene constants
static const RVec3 cRotatingPosition(-5, 0.15f, 15);
static const Quat cRotatingOrientation = Quat::sIdentity();
static const RVec3 cRotatingWallPosition(5, 1.0f, 25.0f);
static const Quat cRotatingWallOrientation = Quat::sIdentity();
static const RVec3 cRotatingAndTranslatingPosition(-10, 0.15f, 27.5f);
static const Quat cRotatingAndTranslatingOrientation = Quat::sIdentity();
static const RVec3 cSmoothVerticallyMovingPosition(0, 2.0f, 15);
static const Quat cSmoothVerticallyMovingOrientation = Quat::sIdentity();
static const RVec3 cReversingVerticallyMovingPosition(0, 0.15f, 25);
static const Quat cReversingVerticallyMovingOrientation = Quat::sIdentity();
static const RVec3 cHorizontallyMovingPosition(5, 1, 15);
static const Quat cHorizontallyMovingOrientation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
static const RVec3 cConveyorBeltPosition(-10, 0.15f, 15);
static const RVec3 cRampPosition(15, 2.2f, 15);
static const Quat cRampOrientation = Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI);
static const RVec3 cRampBlocksStart = cRampPosition + Vec3(-3.0f, 3.0f, 1.5f);
static const Vec3 cRampBlocksDelta = Vec3(2.0f, 0, 0);
static const float cRampBlocksTime = 5.0f;
static const RVec3 cSmallBumpsPosition(-5.0f, 0, 2.5f);
static const float cSmallBumpHeight = 0.05f;
static const float cSmallBumpWidth = 0.01f;
static const float cSmallBumpDelta = 0.5f;
static const RVec3 cLargeBumpsPosition(-10.0f, 0, 2.5f);
static const float cLargeBumpHeight = 0.3f;
static const float cLargeBumpWidth = 0.1f;
static const float cLargeBumpDelta = 2.0f;
static const RVec3 cStairsPosition(-15.0f, 0, 2.5f);
static const float cStairsStepHeight = 0.3f;
static const RVec3 cMeshStairsPosition(-20.0f, 0, 2.5f);
static const RVec3 cNoStairsPosition(-15.0f, 0, 10.0f);
static const float cNoStairsStepHeight = 0.3f;
static const float cNoStairsStepDelta = 0.05f;
static const RVec3 cMeshNoStairsPosition(-20.0f, 0, 10.0f);
static const RVec3 cMeshWallPosition(-25.0f, 0, -27.0f);
static const float cMeshWallHeight = 3.0f;
static const float cMeshWallWidth = 2.0f;
static const float cMeshWallStepStart = 0.5f;
static const float cMeshWallStepEnd = 4.0f;
static const int cMeshWallSegments = 25;
static const RVec3 cHalfCylinderPosition(5.0f, 0, 8.0f);
static const RVec3 cMeshBoxPosition(30.0f, 1.5f, 5.0f);
static const RVec3 cSensorPosition(30, 0.9f, -5);
static const RVec3 cCornersPosition(-20.0f, 0, -35.0f);
static const RVec3 cMovingCornersPosition(-20.0f, 0, -45.0f);
static const Vec3 cCornersDelta(7.0f, 0, 0);
static const Quat cCornersOrientation = Quat::sRotation(Vec3::sAxisY(), -0.5f * JPH_PI);
static const RVec3 cCharacterPosition(-3.5f, 0, 3.0f);
static const RVec3 cCharacterVirtualPosition(-5.0f, 0, 3.0f);
static const RVec3 cCharacterVirtualWithInnerBodyPosition(-6.5f, 0, 3.0f);
static const Vec3 cCharacterVelocity(0, 0, 2);

CharacterBaseTest::~CharacterBaseTest()
{
	if (mAnimatedCharacter != nullptr)
		mAnimatedCharacter->RemoveFromPhysicsSystem();
}

void CharacterBaseTest::Initialize()
{
	// Create capsule shapes for all stances
	switch (sShapeType)
	{
	case EType::Capsule:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching)).Create().Get();
		mInnerStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cInnerShapeFraction * cCharacterHeightStanding, cInnerShapeFraction * cCharacterRadiusStanding)).Create().Get();
		mInnerCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cInnerShapeFraction * cCharacterHeightCrouching, cInnerShapeFraction * cCharacterRadiusCrouching)).Create().Get();
		break;

	case EType::Cylinder:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CylinderShape(0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding)).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CylinderShape(0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching)).Create().Get();
		mInnerStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CylinderShape(cInnerShapeFraction * (0.5f * cCharacterHeightStanding + cCharacterRadiusStanding), cInnerShapeFraction * cCharacterRadiusStanding)).Create().Get();
		mInnerCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CylinderShape(cInnerShapeFraction * (0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching), cInnerShapeFraction * cCharacterRadiusCrouching)).Create().Get();
		break;

	case EType::Box:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusStanding, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding))).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusCrouching, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching))).Create().Get();
		mInnerStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new BoxShape(cInnerShapeFraction * Vec3(cCharacterRadiusStanding, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding))).Create().Get();
		mInnerCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new BoxShape(cInnerShapeFraction * Vec3(cCharacterRadiusCrouching, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching))).Create().Get();
		break;

	case EType::Compound:
		{
			StaticCompoundShapeSettings standing_compound;
			standing_compound.AddShape(Vec3(-0.3f, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding));
			standing_compound.AddShape(Vec3(0.3f, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusStanding, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding)));
			mStandingShape = standing_compound.Create().Get();

			StaticCompoundShapeSettings crouching_compound;
			crouching_compound.AddShape(Vec3(-0.3f, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching));
			crouching_compound.AddShape(Vec3(0.3f, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusCrouching, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching)));
			mCrouchingShape = crouching_compound.Create().Get();

			StaticCompoundShapeSettings inner_standing_compound;
			inner_standing_compound.AddShape(Vec3(-0.3f, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cInnerShapeFraction * cCharacterHeightStanding, cInnerShapeFraction * cCharacterRadiusStanding));
			inner_standing_compound.AddShape(Vec3(0.3f, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new BoxShape(cInnerShapeFraction * Vec3(cCharacterRadiusStanding, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding)));
			mInnerStandingShape = inner_standing_compound.Create().Get();

			StaticCompoundShapeSettings inner_crouching_compound;
			inner_crouching_compound.AddShape(Vec3(-0.3f, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cInnerShapeFraction * cCharacterHeightCrouching, cInnerShapeFraction * cCharacterRadiusCrouching));
			inner_crouching_compound.AddShape(Vec3(0.3f, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new BoxShape(cInnerShapeFraction * Vec3(cCharacterRadiusCrouching, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching)));
			mInnerCrouchingShape = inner_crouching_compound.Create().Get();
		}
		break;
	}

	if (strcmp(sSceneName, "PerlinMesh") == 0)
	{
		// Default terrain
		CreateMeshTerrain();
	}
	else if (strcmp(sSceneName, "PerlinHeightField") == 0)
	{
		// Default terrain
		CreateHeightFieldTerrain();
	}
	else if (strcmp(sSceneName, "InitiallyIntersecting") == 0)
	{
		CreateFloor();

		// Create a grid of boxes that are initially intersecting with the character
		RefConst<Shape> box = new BoxShape(Vec3(0.1f, 0.1f, 0.1f));
		BodyCreationSettings settings(box, RVec3(0, 0.5f, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		for (int x = 0; x < 4; ++x)
			for (int y = 0; y <= 10; ++y)
				for (int z = 0; z <= 10; ++z)
				{
					settings.mPosition = RVec3(-0.5f + 0.1f * x, 0.1f + 0.1f * y, -0.5f + 0.1f * z);
					mBodyInterface->CreateAndAddBody(settings, EActivation::DontActivate);
				}
	}
	else if (strcmp(sSceneName, "InitiallyIntersecting2") == 0)
	{
		CreateFloor();

		// Create two very steep sloped floors that are initially intersecting with the character
		RefConst<Shape> box = new BoxShape(Vec3(1, 0.1f, 1));
		BodyCreationSettings settings(box, RVec3(0, 0, 0), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(75.0f)), EMotionType::Static, Layers::NON_MOVING);
		mBodyInterface->CreateAndAddBody(settings, EActivation::DontActivate);
		settings.mRotation = Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(-75.0f));
		mBodyInterface->CreateAndAddBody(settings, EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "ObstacleCourse") == 0)
	{
		// Default terrain
		CreateFloor(1000.0f);

		{
			// Create ramps with different inclinations
			Ref<Shape> ramp = RotatedTranslatedShapeSettings(Vec3(0, 0, -2.5f), Quat::sIdentity(), new BoxShape(Vec3(1.0f, 0.05f, 2.5f))).Create().Get();
			for (int angle = 0; angle < 18; ++angle)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp, RVec3(-15.0f + angle * 2.0f, 0, -10.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(10.0f * angle)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Create ramps with different inclinations intersecting with a steep slope
			Ref<Shape> ramp = RotatedTranslatedShapeSettings(Vec3(0, 0, -2.5f), Quat::sIdentity(), new BoxShape(Vec3(1.0f, 0.05f, 2.5f))).Create().Get();
			Ref<Shape> ramp2 = RotatedTranslatedShapeSettings(Vec3(0, 2.0f, 0), Quat::sIdentity(), new BoxShape(Vec3(0.05f, 2.0f, 1.0f))).Create().Get();
			for (int angle = 0; angle < 9; ++angle)
			{
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp, RVec3(-15.0f + angle * 2.0f, 0, -20.0f - angle * 0.1f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(10.0f * angle)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp2, RVec3(-15.0f + angle * 2.0f, 0, -21.0f), Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(20.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			}
		}

		{
			// Create wall consisting of vertical pillars
			// Note: Convex radius 0 because otherwise it will be a bumpy wall
			Ref<Shape> wall = new BoxShape(Vec3(0.1f, 2.5f, 0.1f), 0.0f);
			for (int z = 0; z < 30; ++z)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, RVec3(0.0f, 2.5f, 2.0f + 0.2f * z), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Kinematic blocks to test interacting with moving objects
			Ref<Shape> kinematic = new BoxShape(Vec3(1, 0.15f, 3.0f));
			mRotatingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cRotatingPosition, cRotatingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mRotatingWallBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(3.0f, 1, 0.15f)), cRotatingWallPosition, cRotatingWallOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mRotatingAndTranslatingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cRotatingAndTranslatingPosition, cRotatingAndTranslatingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mSmoothVerticallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cSmoothVerticallyMovingPosition, cSmoothVerticallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mReversingVerticallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cReversingVerticallyMovingPosition, cReversingVerticallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mHorizontallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cHorizontallyMovingPosition, cHorizontallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
		}

		{
			// Conveyor belt (only works with virtual character)
			mConveyorBeltBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(1, 0.15f, 3.0f)), cConveyorBeltPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::Activate);
		}

		{
			// A rolling sphere towards the player
			BodyCreationSettings bcs(new SphereShape(0.2f), RVec3(0.0f, 0.2f, -1.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mLinearVelocity = Vec3(0, 0, 2.0f);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		}

		{
			// Dynamic blocks to test player pushing blocks
			Ref<Shape> block = new BoxShape(Vec3::sReplicate(0.5f));
			for (int y = 0; y < 3; ++y)
			{
				BodyCreationSettings bcs(block, RVec3(5.0f, 0.5f + float(y), 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 10.0f;
				mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
			}
		}

		{
			// Dynamic block on a static step (to test pushing block on stairs)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 0.15f, 0.5f)), RVec3(10.0f, 0.15f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3::sReplicate(0.5f)), RVec3(10.0f, 0.8f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
		}

		{
			// Dynamic spheres to test player pushing stuff you can step on
			float h = 0.0f;
			for (int y = 0; y < 3; ++y)
			{
				float r = 0.4f - 0.1f * y;
				h += r;
				BodyCreationSettings bcs(new SphereShape(r), RVec3(15.0f, h, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				h += r;
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 10.0f;
				mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
			}
		}

		{
			// A seesaw to test character gravity
			BodyID b1 = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, 0.2f, 0.05f)), RVec3(20.0f, 0.2f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3(1.0f, 0.05f, 5.0f)), RVec3(20.0f, 0.45f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			BodyID b2 = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

			// Connect the parts with a hinge
			HingeConstraintSettings hinge;
			hinge.mPoint1 = hinge.mPoint2 = RVec3(20.0f, 0.4f, 0.0f);
			hinge.mHingeAxis1 = hinge.mHingeAxis2 = Vec3::sAxisX();
			mPhysicsSystem->AddConstraint(mBodyInterface->CreateConstraint(&hinge, b1, b2));
		}

		{
			// A board above the character to crouch and jump up against
			float h = 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching + 0.1f;
			for (int x = 0; x < 2; ++x)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, h, 0.05f)), RVec3(25.0f, h, x == 0? -0.95f : 0.95f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3(1.0f, 0.05f, 1.0f)), RVec3(25.0f, 2.0f * h + 0.05f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		}

		{
			// A floating static block
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), RVec3(30.0f, 1.5f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Create ramp
			BodyCreationSettings ramp(new BoxShape(Vec3(4.0f, 0.1f, 3.0f)), cRampPosition, cRampOrientation, EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(ramp, EActivation::DontActivate);

			// Create blocks on ramp
			Ref<Shape> block = new BoxShape(Vec3::sReplicate(0.5f));
			BodyCreationSettings bcs(block, cRampBlocksStart, cRampOrientation, EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			for (int i = 0; i < 4; ++i)
			{
				mRampBlocks.emplace_back(mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate));
				bcs.mPosition += cRampBlocksDelta;
			}
		}

		// Create three funnels with walls that are too steep to climb
		Ref<Shape> funnel = new BoxShape(Vec3(0.1f, 1.0f, 1.0f));
		for (int i = 0; i < 2; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, RVec3(5.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 3; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 2.0f / 3.0f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, RVec3(7.5f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 4; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, RVec3(10.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		// Create mesh with a steep slope and an acute angle with triangles that face downwards
		{
			TriangleList triangles;
			triangles.push_back(Triangle(Vec3(-0.5f, 0, 0), Vec3(0.5f, 0, 0), Vec3(-0.5f, 1.1f, -1)));
			triangles.push_back(Triangle(Vec3(0.5f, 0, 0), Vec3(0.5f, 1.1f, -1), Vec3(-0.5f, 1.1f, -1)));
			triangles.push_back(Triangle(Vec3(-0.5f, 0, 0), Vec3(-0.5f, 0, -1), Vec3(0.5f, 0, 0)));
			triangles.push_back(Triangle(Vec3(0.5f, 0, 0), Vec3(-0.5f, 0, -1), Vec3(0.5f, 0, -1)));

			MeshShapeSettings mesh(triangles);
			mesh.SetEmbedded();
			BodyCreationSettings body(&mesh, RVec3(12.5f, 0.5f, 4.0f), Quat::sRotation(Vec3::sAxisY(), JPH_PI), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(body, EActivation::DontActivate);
		}

		// Create small bumps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cSmallBumpHeight, 0.5f * cSmallBumpWidth), 0.0f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cSmallBumpsPosition + Vec3(0, 0.5f * cSmallBumpHeight, cSmallBumpDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create large bumps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cLargeBumpHeight, 0.5f * cLargeBumpWidth)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 5; ++i)
			{
				step.mPosition = cLargeBumpsPosition + Vec3(0, 0.5f * cLargeBumpHeight, cLargeBumpDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create stairs
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cStairsStepHeight, 0.5f * cStairsStepHeight)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cStairsPosition + Vec3(0, cStairsStepHeight * (0.5f + i), cStairsStepHeight * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// A wall beside the stairs
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 2.0f, 5.0f * cStairsStepHeight)), cStairsPosition + Vec3(-2.5f, 2.0f, 5.0f * cStairsStepHeight), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Create stairs from triangles
		{
			TriangleList triangles;

			float rear_z = 10 * cStairsStepHeight;

			for (int i = 0; i < 10; ++i)
			{
				// Start of step
				Vec3 base(0, cStairsStepHeight * i, cStairsStepHeight * i);

				// Left side
				Vec3 b1 = base + Vec3(2.0f, 0, 0);
				Vec3 s1 = b1 + Vec3(0, cStairsStepHeight, 0);
				Vec3 p1 = s1 + Vec3(0, 0, cStairsStepHeight);

				// Right side
				Vec3 width(-4.0f, 0, 0);
				Vec3 b2 = b1 + width;
				Vec3 s2 = s1 + width;
				Vec3 p2 = p1 + width;

				triangles.push_back(Triangle(s1, b1, s2));
				triangles.push_back(Triangle(b1, b2, s2));
				triangles.push_back(Triangle(s1, p2, p1));
				triangles.push_back(Triangle(s1, s2, p2));

				// Side of stairs
				Vec3 rb2 = b2; rb2.SetZ(rear_z);
				Vec3 rs2 = s2; rs2.SetZ(rear_z);

				triangles.push_back(Triangle(s2, b2, rs2));
				triangles.push_back(Triangle(rs2, b2, rb2));
			}

			MeshShapeSettings mesh(triangles);
			mesh.SetEmbedded();
			BodyCreationSettings mesh_stairs(&mesh, cMeshStairsPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(mesh_stairs, EActivation::DontActivate);
		}

		// A wall to the side and behind the stairs
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 2.0f, 0.25f)), cStairsPosition + Vec3(-7.5f, 2.0f, 10.0f * cStairsStepHeight + 0.25f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Create stairs with too little space between the steps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cNoStairsStepHeight, 0.5f * cNoStairsStepHeight)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cNoStairsPosition + Vec3(0, cNoStairsStepHeight * (0.5f + i), cNoStairsStepDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create stairs with too little space between the steps consisting of triangles
		{
			TriangleList triangles;

			for (int i = 0; i < 10; ++i)
			{
				// Start of step
				Vec3 base(0, cStairsStepHeight * i, cNoStairsStepDelta * i);

				// Left side
				Vec3 b1 = base - Vec3(2.0f, 0, 0);
				Vec3 s1 = b1 + Vec3(0, cStairsStepHeight, 0);
				Vec3 p1 = s1 + Vec3(0, 0, cNoStairsStepDelta);

				// Right side
				Vec3 width(4.0f, 0, 0);
				Vec3 b2 = b1 + width;
				Vec3 s2 = s1 + width;
				Vec3 p2 = p1 + width;

				triangles.push_back(Triangle(s1, s2, b1));
				triangles.push_back(Triangle(b1, s2, b2));
				triangles.push_back(Triangle(s1, p1, p2));
				triangles.push_back(Triangle(s1, p2, s2));
			}

			MeshShapeSettings mesh(triangles);
			mesh.SetEmbedded();
			BodyCreationSettings mesh_stairs(&mesh, cMeshNoStairsPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(mesh_stairs, EActivation::DontActivate);
		}

		// Create mesh with walls at varying angles
		{
			TriangleList triangles;
			Vec3 p1(0.5f * cMeshWallWidth, 0, 0);
			Vec3 h(0, cMeshWallHeight, 0);
			for (int i = 0; i < cMeshWallSegments; ++i)
			{
				float delta = cMeshWallStepStart + i * (cMeshWallStepEnd - cMeshWallStepStart) / (cMeshWallSegments - 1);
				Vec3 p2 = Vec3((i & 1)? 0.5f * cMeshWallWidth : -0.5f * cMeshWallWidth, 0, p1.GetZ() + delta);
				triangles.push_back(Triangle(p1, p1 + h, p2 + h));
				triangles.push_back(Triangle(p1, p2 + h, p2));
				p1 = p2;
			}

			MeshShapeSettings mesh(triangles);
			mesh.SetEmbedded();
			BodyCreationSettings wall(&mesh, cMeshWallPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(wall, EActivation::DontActivate);
		}

		// Create a half cylinder with caps for testing contact point limit
		{
			VertexList vertices;
			IndexedTriangleList triangles;

			// The half cylinder
			const int cPosSegments = 2;
			const int cAngleSegments = 512;
			const float cCylinderLength = 2.0f;
			for (int pos = 0; pos < cPosSegments; ++pos)
				for (int angle = 0; angle < cAngleSegments; ++angle)
				{
					uint32 start = (uint32)vertices.size();

					float radius = cCharacterRadiusStanding + 0.05f;
					float angle_rad = (-0.5f + float(angle) / cAngleSegments) * JPH_PI;
					float s = Sin(angle_rad);
					float c = Cos(angle_rad);
					float x = cCylinderLength * (-0.5f + float(pos) / (cPosSegments - 1));
					float y = angle == 0 || angle == cAngleSegments - 1? 0.5f : (1.0f - c) * radius;
					float z = s * radius;
					vertices.push_back(Float3(x, y, z));

					if (pos > 0 && angle > 0)
					{
						triangles.push_back(IndexedTriangle(start, start - 1, start - cAngleSegments));
						triangles.push_back(IndexedTriangle(start - 1, start - cAngleSegments - 1, start - cAngleSegments));
					}
				}

			// Add end caps
			uint32 end = cAngleSegments * (cPosSegments - 1);
			for (int angle = 0; angle < cAngleSegments - 1; ++angle)
			{
				triangles.push_back(IndexedTriangle(0, angle + 1, angle));
				triangles.push_back(IndexedTriangle(end, end + angle, end + angle + 1));
			}

			MeshShapeSettings mesh(std::move(vertices), std::move(triangles));
			mesh.SetEmbedded();
			BodyCreationSettings mesh_cylinder(&mesh, cHalfCylinderPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(mesh_cylinder, EActivation::DontActivate);
		}

		// Create a box made out of polygons (character should not get stuck behind back facing side)
		{
			VertexList vertices = {
				Float3(-1,  1, -1),
				Float3( 1,  1, -1),
				Float3( 1,  1,  1),
				Float3(-1,  1,  1),
				Float3(-1, -1, -1),
				Float3( 1, -1, -1),
				Float3( 1, -1,  1),
				Float3(-1, -1,  1)
			};

			IndexedTriangleList triangles = {
				IndexedTriangle(0, 3, 2),
				IndexedTriangle(0, 2, 1),
				IndexedTriangle(4, 5, 6),
				IndexedTriangle(4, 6, 7),
				IndexedTriangle(0, 4, 3),
				IndexedTriangle(3, 4, 7),
				IndexedTriangle(2, 6, 5),
				IndexedTriangle(2, 5, 1),
				IndexedTriangle(3, 7, 6),
				IndexedTriangle(3, 6, 2),
				IndexedTriangle(0, 1, 5),
				IndexedTriangle(0, 5, 4)
			};

			MeshShapeSettings mesh(std::move(vertices), std::move(triangles));
			mesh.SetEmbedded();
			BodyCreationSettings box(&mesh, cMeshBoxPosition, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			mBodyInterface->CreateAndAddBody(box, EActivation::DontActivate);
		}

		// Create a sensor
		{
			BodyCreationSettings sensor(new BoxShape(Vec3::sOne()), cSensorPosition, Quat::sIdentity(), EMotionType::Kinematic, Layers::SENSOR);
			sensor.mIsSensor = true;
			mSensorBody = mBodyInterface->CreateAndAddBody(sensor, EActivation::Activate);
		}

		// Create Character
		{
			CharacterSettings settings;
			settings.mLayer = Layers::MOVING;
			settings.mShape = mStandingShape;
			settings.mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
			mAnimatedCharacter = new Character(&settings, cCharacterPosition, Quat::sIdentity(), 0, mPhysicsSystem);
			mAnimatedCharacter->AddToPhysicsSystem();
		}

		// Create CharacterVirtual
		{
			CharacterVirtualSettings settings;
			settings.mShape = mStandingShape;
			settings.mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
			mAnimatedCharacterVirtual = new CharacterVirtual(&settings, cCharacterVirtualPosition, Quat::sIdentity(), 0, mPhysicsSystem);
			mAnimatedCharacterVirtual->SetCharacterVsCharacterCollision(&mCharacterVsCharacterCollision);
			mCharacterVsCharacterCollision.Add(mAnimatedCharacterVirtual);
		}

		// Create CharacterVirtual with inner rigid body
		{
			CharacterVirtualSettings settings;
			settings.mShape = mStandingShape;
			settings.mInnerBodyShape = mInnerStandingShape;
			settings.mSupportingVolume = Plane(Vec3::sAxisY(), -cCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
			mAnimatedCharacterVirtualWithInnerBody = new CharacterVirtual(&settings, cCharacterVirtualWithInnerBodyPosition, Quat::sIdentity(), 0, mPhysicsSystem);
			mAnimatedCharacterVirtualWithInnerBody->SetCharacterVsCharacterCollision(&mCharacterVsCharacterCollision);
			mCharacterVsCharacterCollision.Add(mAnimatedCharacterVirtualWithInnerBody);
		}

		// Create corners and slopes that used to make CharacterVirtual jitter (see also the TestJumpIntoCorner, TestWalkIntoCornerOnSlope and TestWalkUpSlopeIntoWall unit tests).
		// Each configuration is a single shape so that it can be moved as a whole. We create a static and a moving version of each.
		// In local space the character should approach them from +X, the shapes are rotated so that they open towards the player starting position.
		{
			// Add two vertical walls that meet at inApex with inCornerAngle between them, the corner opens towards +X
			auto add_corner_walls = [](TriangleList &ioTriangles, Vec3Arg inApex, float inCornerAngle)
			{
				constexpr float cWallLength = 3.5f;
				Vec3 bottom = inApex - Vec3(0, 1.5f, 0), top = inApex + Vec3(0, 3.0f, 0);
				float half_angle = 0.5f * inCornerAngle;
				Vec3 end = cWallLength * Vec3(Cos(half_angle), 0, Sin(half_angle));
				ioTriangles.push_back(Triangle(bottom, top, end + top));
				ioTriangles.push_back(Triangle(bottom, end + top, end + bottom));
				end = end.FlipSign<1, 1, -1>();
				ioTriangles.push_back(Triangle(bottom, end + top, top));
				ioTriangles.push_back(Triangle(bottom, end + bottom, end + top));
			};

			// Add a box to a compound shape. The center of the top surface of the box is at inTopCenter and the box is rotated by inRotation.
			auto add_slab = [](StaticCompoundShapeSettings &ioCompound, Vec3Arg inTopCenter, QuatArg inRotation, float inHalfLength, float inHalfWidth)
			{
				constexpr float cHalfThickness = 0.15f;
				ioCompound.AddShape(inTopCenter - cHalfThickness * (inRotation * Vec3::sAxisY()), inRotation, new BoxShape(Vec3(inHalfLength, cHalfThickness, inHalfWidth), 0.0f));
			};

			// Create a static and a moving body for a compound shape (plus optional walls), inHeight is the height at which to place the bodies
			int index = 0;
			auto create_bodies = [this, &index](StaticCompoundShapeSettings &ioCompound, const TriangleList &inWalls, float inHeight)
			{
				if (!inWalls.empty())
					ioCompound.AddShape(Vec3::sZero(), Quat::sIdentity(), MeshShapeSettings(inWalls).Create().Get());
				RefConst<Shape> shape = ioCompound.Create().Get();

				Vec3 offset = float(index++) * cCornersDelta + Vec3(0, inHeight, 0);
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(shape, cCornersPosition + offset, cCornersOrientation, EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

				// Create the moving version at the position it will have at time 0
				RVec3 position = cMovingCornersPosition + offset;
				BodyCreationSettings moving(shape, position + Vec3(0, 0, 2.0f), cCornersOrientation, EMotionType::Kinematic, Layers::MOVING);
				moving.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided; // Mesh shapes have no mass properties
				moving.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(Vec3::sReplicate(5.0f), 1000.0f);
				mMovingCorners.push_back({ mBodyInterface->CreateAndAddBody(moving, EActivation::Activate), position });
			};

			// Walk up a 10 degree slope into a 135 degree corner
			{
				float slope = DegreesToRadians(10.0f);
				Quat rotation = Quat::sRotation(Vec3::sAxisZ(), -slope);
				StaticCompoundShapeSettings compound;
				add_slab(compound, rotation * Vec3(1.5f, 0, 0), rotation, 2.5f, 3.0f);
				TriangleList walls;
				add_corner_walls(walls, Vec3::sZero(), DegreesToRadians(135.0f));
				create_bodies(compound, walls, 4.0f * Sin(slope) + 0.5f);
			}

			// Walk up a 10 degree slope into a wall at an angle so that the character slides along the wall
			{
				float slope = DegreesToRadians(10.0f);
				Quat rotation = Quat::sRotation(Vec3::sAxisZ(), -slope);
				StaticCompoundShapeSettings compound;
				add_slab(compound, rotation * Vec3(1.5f, 0, 0), rotation, 2.5f, 3.0f);
				TriangleList walls;
				add_corner_walls(walls, Vec3::sZero(), JPH_PI);
				create_bodies(compound, walls, 4.0f * Sin(slope) + 0.5f);
			}

			// Steep pocket: Walk up a 30 degree ramp that turns into a 70 degree slope inside a 120 degree corner
			{
				float ramp = DegreesToRadians(30.0f), steep = DegreesToRadians(70.0f);
				Quat ramp_rotation = Quat::sRotation(Vec3::sAxisZ(), -ramp);
				Quat steep_rotation = Quat::sRotation(Vec3::sAxisZ(), -steep);
				StaticCompoundShapeSettings compound;
				add_slab(compound, ramp_rotation * Vec3(1.0f, 0, 0), ramp_rotation, 2.0f, 2.5f);
				Vec3 steep_start(-0.3f, Tan(ramp) * 0.3f, 0);
				add_slab(compound, steep_start + steep_rotation * Vec3(-1.2f, 0, 0), steep_rotation, 1.5f, 1.5f);
				TriangleList walls;
				add_corner_walls(walls, Vec3(-1.0f, 0, 0), DegreesToRadians(120.0f));
				create_bodies(compound, walls, 3.0f * Sin(ramp) + 0.5f);
			}

			// Steep V: Walk up a valley formed by a 55 and a 60 degree slope, the valley rises 10 degrees towards -X
			{
				float incline = DegreesToRadians(10.0f);
				Quat tilt = Quat::sRotation(Vec3::sAxisZ(), -incline);
				StaticCompoundShapeSettings compound;
				for (float side_angle : { 55.0f, -60.0f })
				{
					Quat rotation = tilt * Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(side_angle));
					add_slab(compound, rotation * Vec3(0, 0, side_angle > 0.0f? -1.45f : 1.45f), rotation, 3.0f, 1.5f);
				}
				create_bodies(compound, TriangleList(), 3.0f * Sin(incline));
			}

			// Jump into a 130 and a 100 degree corner
			for (float corner_angle : { 130.0f, 100.0f })
			{
				StaticCompoundShapeSettings compound;
				add_slab(compound, Vec3(1.5f, 0, 0), Quat::sIdentity(), 2.5f, 3.0f);
				TriangleList walls;
				add_corner_walls(walls, Vec3::sZero(), DegreesToRadians(corner_angle));
				create_bodies(compound, walls, 0.5f);
			}

			// Gondola: A narrow aisle between two benches made out of multiple segments, with pillars and a console at the far end.
			// The character can walk in from +X.
			{
				// Add a box made out of triangles that face outwards
				auto add_box = [](TriangleList &ioTriangles, Vec3Arg inMin, Vec3Arg inMax)
				{
					for (uint axis = 0; axis < 3; ++axis)
						for (int side = 0; side < 2; ++side)
						{
							// Pick the tangent axes so that u x v points out of the box
							uint u = (axis + 1) % 3, v = (axis + 2) % 3;
							if (side == 0)
								std::swap(u, v);
							Vec3 quad[4];
							for (int i = 0; i < 4; ++i)
							{
								Vec3 p = side == 0? inMin : inMax;
								p.SetComponent(u, i == 1 || i == 2? inMax[u] : inMin[u]);
								p.SetComponent(v, i >= 2? inMax[v] : inMin[v]);
								quad[i] = p;
							}
							ioTriangles.push_back(Triangle(quad[0], quad[1], quad[2]));
							ioTriangles.push_back(Triangle(quad[0], quad[2], quad[3]));
						}
				};

				constexpr float cHalfLength = 3.0f;			// Gondola goes from -cHalfLength to cHalfLength along X
				constexpr float cAisleHalfWidth = 0.5f;		// A bit wider than the character
				constexpr float cBenchDepth = 0.5f;
				constexpr float cBenchHeight = 0.45f;
				constexpr float cBenchStart = -1.8f;		// Benches go from here to cHalfLength along X
				constexpr float cBenchSegmentLength = 1.2f;
				constexpr float cWallThickness = 0.05f;
				constexpr float cWallHeight = 1.0f;
				constexpr float cPillarHalfSize = 0.04f;
				constexpr float cPillarTop = 2.2f;
				constexpr float cBenchOuter = cAisleHalfWidth + cBenchDepth;

				TriangleList triangles;

				// Floor
				add_box(triangles, Vec3(-cHalfLength, -0.1f, -cBenchOuter - cWallThickness), Vec3(cHalfLength, 0, cBenchOuter + cWallThickness));

				for (float sign : { -1.0f, 1.0f })
				{
					// Benches consisting of multiple segments
					for (float x = cBenchStart; x < cHalfLength - 1.0e-3f; x += cBenchSegmentLength)
					{
						Vec3 p1(x, 0, sign * cAisleHalfWidth), p2(x + cBenchSegmentLength, cBenchHeight, sign * cBenchOuter);
						add_box(triangles, Vec3::sMin(p1, p2), Vec3::sMax(p1, p2));
					}

					// Side walls
					Vec3 w1(-cHalfLength, 0, sign * cBenchOuter), w2(cHalfLength, cWallHeight, sign * (cBenchOuter + cWallThickness));
					add_box(triangles, Vec3::sMin(w1, w2), Vec3::sMax(w1, w2));

					// Pillars on top of the side walls
					for (float x = -cHalfLength; x < cHalfLength + 1.0e-3f; x += cBenchSegmentLength)
					{
						Vec3 center(Clamp(x, -cHalfLength + cPillarHalfSize, cHalfLength - cPillarHalfSize), 0, sign * (cBenchOuter + 0.5f * cWallThickness));
						add_box(triangles, center + Vec3(-cPillarHalfSize, cWallHeight, -cPillarHalfSize), center + Vec3(cPillarHalfSize, cPillarTop, cPillarHalfSize));
					}
				}

				// Front wall
				add_box(triangles, Vec3(-cHalfLength - cWallThickness, 0, -cBenchOuter - cWallThickness), Vec3(-cHalfLength, cWallHeight, cBenchOuter + cWallThickness));

				// Console against the front wall
				add_box(triangles, Vec3(-cHalfLength, 0, -0.6f), Vec3(-cHalfLength + 0.4f, 1.0f, 0.6f));

				// Pole next to the console that goes up to the roof
				add_box(triangles, Vec3(-cHalfLength + 0.5f, 0, -0.6f), Vec3(-cHalfLength + 0.58f, cPillarTop, -0.52f));

				// Stool in front of the console
				add_box(triangles, Vec3(-2.35f, 0, 0.1f), Vec3(-2.05f, 0.6f, 0.4f));

				StaticCompoundShapeSettings compound;
				create_bodies(compound, triangles, 0.5f);
			}
		}
	}
#ifdef JPH_OBJECT_STREAM
	else
	{
		// Load scene
		Ref<PhysicsScene> scene;
		AssetStream stream(ToLower(String(sSceneName)) + ".bof", std::ios::in | std::ios::binary);
		if (!ObjectStreamIn::sReadObject(stream.Get(), scene))
			FatalError("Failed to load scene");
		scene->FixInvalidScales();
		for (BodyCreationSettings &settings : scene->GetBodies())
		{
			settings.mObjectLayer = Layers::NON_MOVING;
			settings.mFriction = 0.5f;
		}
		scene->CreateBodies(mPhysicsSystem);
	}
#endif // JPH_OBJECT_STREAM
}

void CharacterBaseTest::ProcessInput(const ProcessInputParams &inParams)
{
	// Determine controller input
	mControlInput = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(EKey::Left))	mControlInput.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(EKey::Right))	mControlInput.SetZ(1);
	if (inParams.mKeyboard->IsKeyPressed(EKey::Up))		mControlInput.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(EKey::Down))	mControlInput.SetX(-1);
	if (mControlInput != Vec3::sZero())
		mControlInput = mControlInput.Normalized();

	// Rotate controls to align with the camera
	Vec3 cam_fwd = inParams.mCameraState.mForward;
	cam_fwd.SetY(0.0f);
	cam_fwd = cam_fwd.NormalizedOr(Vec3::sAxisX());
	Quat rotation = Quat::sFromTo(Vec3::sAxisX(), cam_fwd);
	mControlInput = rotation * mControlInput;

	// Check actions
	mJump = inParams.mKeyboard->IsKeyPressedAndTriggered(EKey::RControl, mWasJump);
	mSwitchStance = inParams.mKeyboard->IsKeyPressedAndTriggered(EKey::RShift, mWasSwitchStance);
}

void CharacterBaseTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update scene time
	mTime += inParams.mDeltaTime;

	// Update camera pivot
	mCameraPivot = GetCharacterPosition();

	// Animate bodies
	if (!mRotatingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mRotatingBody, cRotatingPosition, Quat::sRotation(Vec3::sAxisY(), JPH_PI * Sin(mTime)), inParams.mDeltaTime);
	if (!mRotatingWallBody.IsInvalid())
		mBodyInterface->MoveKinematic(mRotatingWallBody, cRotatingWallPosition, Quat::sRotation(Vec3::sAxisY(), JPH_PI * Sin(mTime)), inParams.mDeltaTime);
	if (!mRotatingAndTranslatingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mRotatingAndTranslatingBody, cRotatingAndTranslatingPosition + 5.0f * Vec3(Sin(JPH_PI * mTime), 0, Cos(JPH_PI * mTime)), Quat::sRotation(Vec3::sAxisY(), JPH_PI * Sin(mTime)), inParams.mDeltaTime);
	if (!mHorizontallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mHorizontallyMovingBody, cHorizontallyMovingPosition + Vec3(3.0f * Sin(mTime), 0, 0), cHorizontallyMovingOrientation, inParams.mDeltaTime);
	if (!mSmoothVerticallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mSmoothVerticallyMovingBody, cSmoothVerticallyMovingPosition + Vec3(0, 1.75f * Sin(mTime), 0), cSmoothVerticallyMovingOrientation, inParams.mDeltaTime);
	if (!mReversingVerticallyMovingBody.IsInvalid())
	{
		RVec3 pos = mBodyInterface->GetPosition(mReversingVerticallyMovingBody);
		if (pos.GetY() < cReversingVerticallyMovingPosition.GetY())
			mReversingVerticallyMovingVelocity = 1.0f;
		else if (pos.GetY() > cReversingVerticallyMovingPosition.GetY() + 5.0f)
			mReversingVerticallyMovingVelocity = -1.0f;
		mBodyInterface->MoveKinematic(mReversingVerticallyMovingBody, pos + Vec3(0, mReversingVerticallyMovingVelocity * 3.0f * inParams.mDeltaTime, 0), cReversingVerticallyMovingOrientation, inParams.mDeltaTime);
	}
	for (const MovingCorner &corner : mMovingCorners)
		mBodyInterface->MoveKinematic(corner.mBodyID, corner.mPosition + 2.0f * Vec3(Sin(0.25f * JPH_PI * mTime), 1.0f - Cos(0.1f * JPH_PI * mTime), Cos(0.25f * JPH_PI * mTime)), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI * Sin(mTime)) * cCornersOrientation, inParams.mDeltaTime);

	// Animate character
	if (mAnimatedCharacter != nullptr)
		mAnimatedCharacter->SetLinearVelocity(Sin(mTime) * cCharacterVelocity);

	// Animate character virtual
	for (CharacterVirtual *character : { mAnimatedCharacterVirtual, mAnimatedCharacterVirtualWithInnerBody })
		if (character != nullptr)
		{
			// Draw the character
			DrawPaddedCharacter(character->GetShape(), character->GetCharacterPadding(), character->GetCenterOfMassTransform());

			// Update velocity and apply gravity
			Vec3 velocity;
			if (character->GetGroundState() == CharacterVirtual::EGroundState::OnGround)
				velocity = Vec3::sZero();
			else
				velocity = character->GetLinearVelocity() * mAnimatedCharacter->GetUp() + mPhysicsSystem->GetGravity() * inParams.mDeltaTime;
			velocity += Sin(mTime) * cCharacterVelocity;
			character->SetLinearVelocity(velocity);

			// Move character
			CharacterVirtual::ExtendedUpdateSettings update_settings;
			character->ExtendedUpdate(inParams.mDeltaTime,
				mPhysicsSystem->GetGravity(),
				update_settings,
				mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
				mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING),
				{ },
				{ },
				*mTempAllocator);
		}

	// Reset ramp blocks
	mRampBlocksTimeLeft -= inParams.mDeltaTime;
	if (mRampBlocksTimeLeft < 0.0f)
	{
		for (size_t i = 0; i < mRampBlocks.size(); ++i)
		{
			mBodyInterface->SetPositionAndRotation(mRampBlocks[i], cRampBlocksStart + float(i) * cRampBlocksDelta, cRampOrientation, EActivation::Activate);
			mBodyInterface->SetLinearAndAngularVelocity(mRampBlocks[i], Vec3::sZero(), Vec3::sZero());
		}
		mRampBlocksTimeLeft = cRampBlocksTime;
	}

	// Call handle input after new velocities have been set to avoid frame delay
	HandleInput(mControlInput, mJump, mSwitchStance, inParams.mDeltaTime);
}

void CharacterBaseTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});

	inUI->CreateTextButton(inSubMenu, "Character Movement Settings", [this, inUI]() {
		UIElement *movement_settings = inUI->CreateMenu();

		inUI->CreateCheckBox(movement_settings, "Control Movement During Jump", sControlMovementDuringJump, [](UICheckBox::EState inState) { sControlMovementDuringJump = inState == UICheckBox::STATE_CHECKED; });
		inUI->CreateSlider(movement_settings, "Character Speed", sCharacterSpeed, 0.1f, 10.0f, 0.1f, [](float inValue) { sCharacterSpeed = inValue; });
		inUI->CreateSlider(movement_settings, "Character Jump Speed", sJumpSpeed, 0.1f, 10.0f, 0.1f, [](float inValue) { sJumpSpeed = inValue; });
		AddCharacterMovementSettings(inUI, movement_settings);
		inUI->ShowMenu(movement_settings);
	});

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [this, inUI]() {
		UIElement *configuration_settings = inUI->CreateMenu();

		inUI->CreateComboBox(configuration_settings, "Shape Type", { "Capsule", "Cylinder", "Box", "Compound" }, (int)sShapeType, [](int inItem) { sShapeType = (EType)inItem; });
		AddConfigurationSettings(inUI, configuration_settings);
		inUI->CreateTextButton(configuration_settings, "Accept Changes", [this]() { RestartTest(); });
		inUI->ShowMenu(configuration_settings);
	});
}

void CharacterBaseTest::GetInitialCamera(CameraState& ioState) const
{
	// This will become the local space offset, look down the x axis and slightly down
	ioState.mPos = RVec3::sZero();
	ioState.mForward = Vec3(10.0f, -2.0f, 0).Normalized();
}

RMat44 CharacterBaseTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera
	Vec3 fwd = Vec3(Cos(inCameraPitch) * Cos(inCameraHeading), Sin(inCameraPitch), Cos(inCameraPitch) * Sin(inCameraHeading));
	return RMat44::sTranslation(mCameraPivot + Vec3(0, cCharacterHeightStanding + cCharacterRadiusStanding, 0) - 5.0f * fwd);
}

void CharacterBaseTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mRampBlocksTimeLeft);
	inStream.Write(mReversingVerticallyMovingVelocity);

	if (mAnimatedCharacterVirtual != nullptr)
		mAnimatedCharacterVirtual->SaveState(inStream);

	if (mAnimatedCharacterVirtualWithInnerBody != nullptr)
		mAnimatedCharacterVirtualWithInnerBody->SaveState(inStream);
}

void CharacterBaseTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mRampBlocksTimeLeft);
	inStream.Read(mReversingVerticallyMovingVelocity);

	if (mAnimatedCharacterVirtual != nullptr)
		mAnimatedCharacterVirtual->RestoreState(inStream);

	if (mAnimatedCharacterVirtualWithInnerBody != nullptr)
		mAnimatedCharacterVirtualWithInnerBody->RestoreState(inStream);
}

void CharacterBaseTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mControlInput);
	inStream.Write(mJump);
	inStream.Write(mSwitchStance);
}

void CharacterBaseTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mControlInput);
	inStream.Read(mJump);
	inStream.Read(mSwitchStance);
}

void CharacterBaseTest::DrawCharacterState(const CharacterBase *inCharacter, RMat44Arg inCharacterTransform, Vec3Arg inCharacterVelocity)
{
	// Draw current location
	// Drawing prior to update since the physics system state is also that prior to the simulation step (so that all detected collisions etc. make sense)
	mDebugRenderer->DrawCoordinateSystem(inCharacterTransform, 0.1f);

	// Draw the state of the ground contact
	CharacterBase::EGroundState ground_state = inCharacter->GetGroundState();
	if (ground_state != CharacterBase::EGroundState::InAir)
	{
		RVec3 ground_position = inCharacter->GetGroundPosition();
		Vec3 ground_normal = inCharacter->GetGroundNormal();
		Vec3 ground_velocity = inCharacter->GetGroundVelocity();

		// Draw ground position
		mDebugRenderer->DrawMarker(ground_position, Color::sRed, 0.1f);
		mDebugRenderer->DrawArrow(ground_position, ground_position + 2.0f * ground_normal, Color::sGreen, 0.1f);

		// Draw ground velocity
		if (!ground_velocity.IsNearZero())
			mDebugRenderer->DrawArrow(ground_position, ground_position + ground_velocity, Color::sBlue, 0.1f);
	}

	// Draw provided character velocity
	if (!inCharacterVelocity.IsNearZero())
		mDebugRenderer->DrawArrow(inCharacterTransform.GetTranslation(), inCharacterTransform.GetTranslation() + inCharacterVelocity, Color::sYellow, 0.1f);

	// Draw text info
	const PhysicsMaterial *ground_material = inCharacter->GetGroundMaterial();
	Vec3 horizontal_velocity = inCharacterVelocity;
	horizontal_velocity.SetY(0);
	Color color = horizontal_velocity.Length() > 1.01f * sCharacterSpeed? Color::sRed : Color::sWhite;
	mDebugRenderer->DrawText3D(inCharacterTransform.GetTranslation(), StringFormat("State: %s\nMat: %s\nHorizontal Vel: %.1f m/s\nVertical Vel: %.1f m/s", CharacterBase::sToString(ground_state), ground_material->GetDebugName(), (double)horizontal_velocity.Length(), (double)inCharacterVelocity.GetY()), color, 0.25f);
}

void CharacterBaseTest::DrawPaddedCharacter(const Shape *inShape, float inPadding, RMat44Arg inCenterOfMass)
{
	if (inShape->GetSubType() == EShapeSubType::Capsule)
	{
		const CapsuleShape *capsule = static_cast<const CapsuleShape *>(inShape);
		mDebugRenderer->DrawCapsule(inCenterOfMass, capsule->GetHalfHeightOfCylinder(), capsule->GetRadius() + inPadding, Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}
	else if (inShape->GetSubType() == EShapeSubType::Cylinder)
	{
		// Not correct as the edges should be rounded
		const CylinderShape *cylinder = static_cast<const CylinderShape *>(inShape);
		mDebugRenderer->DrawCylinder(inCenterOfMass, cylinder->GetHalfHeight() + inPadding, cylinder->GetRadius() + inPadding, Color::sGrey, DebugRenderer::ECastShadow::Off, DebugRenderer::EDrawMode::Wireframe);
	}
	else if (inShape->GetSubType() == EShapeSubType::Box)
	{
		// Not correct as the edges should be rounded
		const BoxShape *box = static_cast<const BoxShape *>(inShape);
		AABox bounds = box->GetLocalBounds();
		bounds.ExpandBy(Vec3::sReplicate(inPadding));
		mDebugRenderer->DrawWireBox(inCenterOfMass, bounds, Color::sGrey);
	}
	else if (inShape->GetSubType() == EShapeSubType::RotatedTranslated)
	{
		const RotatedTranslatedShape *rt = static_cast<const RotatedTranslatedShape *>(inShape);
		DrawPaddedCharacter(rt->GetInnerShape(), inPadding, inCenterOfMass);
	}
	else if (inShape->GetType() == EShapeType::Compound)
	{
		const CompoundShape *compound = static_cast<const CompoundShape *>(inShape);
		for (const CompoundShape::SubShape &sub_shape : compound->GetSubShapes())
			DrawPaddedCharacter(sub_shape.mShape, inPadding, inCenterOfMass * sub_shape.GetLocalTransformNoScale(Vec3::sOne()));
	}
}

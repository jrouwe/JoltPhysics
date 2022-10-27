// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Character/CharacterBaseTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(CharacterBaseTest) 
{ 
	JPH_ADD_BASE_CLASS(CharacterBaseTest, Test) 
}

const char *CharacterBaseTest::sScenes[] =
{
	"PerlinMesh",
	"PerlinHeightField",
	"ObstacleCourse",
	"Terrain1",
	"Terrain2",
};

const char *CharacterBaseTest::sSceneName = "ObstacleCourse";

// Scene constants
static const Vec3 cRotatingPosition(-5, 0.15f, 15);
static const Quat cRotatingOrientation = Quat::sIdentity();
static const Vec3 cVerticallyMovingPosition(0, 2.0f, 15);
static const Quat cVerticallyMovingOrientation = Quat::sIdentity();
static const Vec3 cHorizontallyMovingPosition(5, 1, 15);
static const Quat cHorizontallyMovingOrientation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
static const Vec3 cRampPosition(15, 2.2f, 15);
static const Quat cRampOrientation = Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI);
static const Vec3 cRampBlocksStart = cRampPosition + Vec3(-3.0f, 3.0f, 1.5f);
static const Vec3 cRampBlocksDelta = Vec3(2.0f, 0, 0);
static const float cRampBlocksTime = 5.0f;
static const Vec3 cSmallBumpsPosition = Vec3(-5.0f, 0, 2.5f);
static const float cSmallBumpHeight = 0.05f;
static const float cSmallBumpWidth = 0.01f;
static const float cSmallBumpDelta = 0.5f;
static const Vec3 cLargeBumpsPosition = Vec3(-10.0f, 0, 2.5f);
static const float cLargeBumpHeight = 0.3f;
static const float cLargeBumpWidth = 0.1f;
static const float cLargeBumpDelta = 2.0f;
static const Vec3 cStairsPosition = Vec3(-15.0f, 0, 2.5f);
static const float cStairsStepHeight = 0.3f;
static const Vec3 cMeshStairsPosition = Vec3(-20.0f, 0, 2.5f);
static const Vec3 cNoStairsPosition = Vec3(-15.0f, 0, 10.0f);
static const float cNoStairsStepHeight = 0.3f;
static const float cNoStairsStepDelta = 0.05f;
static const Vec3 cMeshNoStairsPosition = Vec3(-20.0f, 0, 10.0f);
static const Vec3 cMeshWallPosition = Vec3(-25.0f, 0, -27.0f);
static const float cMeshWallHeight = 3.0f;
static const float cMeshWallWidth = 2.0f;
static const float cMeshWallStepStart = 0.5f;
static const float cMeshWallStepEnd = 4.0f;
static const int cMeshWallSegments = 25;

void CharacterBaseTest::Initialize()
{
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
	else if (strcmp(sSceneName, "ObstacleCourse") == 0)
	{
		// Default terrain
		CreateFloor(350.0f);

		{
			// Create ramps with different inclinations
			Ref<Shape> ramp = RotatedTranslatedShapeSettings(Vec3(0, 0, -2.5f), Quat::sIdentity(), new BoxShape(Vec3(1.0f, 0.05f, 2.5f))).Create().Get();
			for (int angle = 0; angle < 18; ++angle)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(ramp, Vec3(-15.0f + angle * 2.0f, 0, -10.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(10.0f * angle)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Create wall consisting of vertical pillars
			// Note: Convex radius 0 because otherwise it will be a bumpy wall
			Ref<Shape> wall = new BoxShape(Vec3(0.1f, 2.5f, 0.1f), 0.0f); 
			for (int z = 0; z < 30; ++z)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(wall, Vec3(0.0f, 2.5f, 2.0f + 0.2f * z), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		{
			// Kinematic blocks to test interacting with moving objects
			Ref<Shape> kinematic = new BoxShape(Vec3(1, 0.15f, 3.0f));
			mRotatingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cRotatingPosition, cRotatingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mVerticallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cVerticallyMovingPosition, cVerticallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
			mHorizontallyMovingBody = mBodyInterface->CreateAndAddBody(BodyCreationSettings(kinematic, cHorizontallyMovingPosition, cHorizontallyMovingOrientation, EMotionType::Kinematic, Layers::MOVING), EActivation::Activate);
		}

		{
			// A rolling sphere towards the player
			BodyCreationSettings bcs(new SphereShape(0.2f), Vec3(0.0f, 0.2f, -1.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
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
				BodyCreationSettings bcs(block, Vec3(5.0f, 0.5f + float(y), 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 10.0f;
				mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
			}
		}

		{
			// Dynamic block on a static step (to test pushing block on stairs)
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 0.15f, 0.5f)), Vec3(10.0f, 0.15f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3::sReplicate(0.5f)), Vec3(10.0f, 0.8f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
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
				BodyCreationSettings bcs(new SphereShape(r), Vec3(15.0f, h, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				h += r;
				bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				bcs.mMassPropertiesOverride.mMass = 10.0f;
				mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
			}
		}

		{
			// A seesaw to test character gravity
			BodyID b1 = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, 0.2f, 0.05f)), Vec3(20.0f, 0.2f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3(1.0f, 0.05f, 5.0f)), Vec3(20.0f, 0.45f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			BodyID b2 = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);

			// Connect the parts with a hinge
			HingeConstraintSettings hinge;
			hinge.mPoint1 = hinge.mPoint2 = Vec3(20.0f, 0.4f, 0.0f);
			hinge.mHingeAxis1 = hinge.mHingeAxis2 = Vec3::sAxisX();
			mPhysicsSystem->AddConstraint(mBodyInterface->CreateConstraint(&hinge, b1, b2));
		}

		{
			// A board above the character to crouch and jump up against
			float h = 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching + 0.1f;
			for (int x = 0; x < 2; ++x)
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, h, 0.05f)), Vec3(25.0f, h, x == 0? -0.95f : 0.95f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
			BodyCreationSettings bcs(new BoxShape(Vec3(1.0f, 0.05f, 1.0f)), Vec3(25.0f, 2.0f * h + 0.05f, 0.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
			bcs.mMassPropertiesOverride.mMass = 10.0f;
			mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
		}

		{
			// A floating static block
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.5f)), Vec3(30.0f, 1.5f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
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
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(5.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 3; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 2.0f / 3.0f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(7.5f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}
		for (int i = 0; i < 4; ++i)
		{
			Quat rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(funnel, Vec3(10.0f, 0.1f, 5.0f) + rotation * Vec3(0.2f, 0, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), -DegreesToRadians(40.0f)), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		}

		// Create small bumps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cSmallBumpHeight, 0.5f * cSmallBumpWidth), 0.0f), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 10; ++i)
			{
				step.mPosition = cSmallBumpsPosition + Vec3(0, 0.5f * cSmallBumpHeight, cSmallBumpDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create large bumps
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cLargeBumpHeight, 0.5f * cLargeBumpWidth)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
			for (int i = 0; i < 5; ++i)
			{
				step.mPosition = cLargeBumpsPosition + Vec3(0, 0.5f * cLargeBumpHeight, cLargeBumpDelta * i);
				mBodyInterface->CreateAndAddBody(step, EActivation::DontActivate);
			}
		}

		// Create stairs
		{
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cStairsStepHeight, 0.5f * cStairsStepHeight)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
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

				p1 = p2;
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
			BodyCreationSettings step(new BoxShape(Vec3(2.0f, 0.5f * cNoStairsStepHeight, 0.5f * cNoStairsStepHeight)), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
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
				p1 = p2;
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
	}
	else
	{
		// Load scene
		Ref<PhysicsScene> scene;
		if (!ObjectStreamIn::sReadObject((String("Assets/") + sSceneName + ".bof").c_str(), scene))
			FatalError("Failed to load scene");
		scene->FixInvalidScales();
		for (BodyCreationSettings &settings : scene->GetBodies())
		{
			settings.mObjectLayer = Layers::NON_MOVING;
			settings.mFriction = 0.5f;
		}
		scene->CreateBodies(mPhysicsSystem);
	}

	// Create capsule shapes for all stances
	switch (sShapeType)
	{
	case EType::Capsule:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightStanding, cCharacterRadiusStanding)).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CapsuleShape(0.5f * cCharacterHeightCrouching, cCharacterRadiusCrouching)).Create().Get();
		break;

	case EType::Cylinder:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new CylinderShape(0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding)).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new CylinderShape(0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching)).Create().Get();
		break;

	case EType::Box:
		mStandingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusStanding, 0.5f * cCharacterHeightStanding + cCharacterRadiusStanding, cCharacterRadiusStanding))).Create().Get();
		mCrouchingShape = RotatedTranslatedShapeSettings(Vec3(0, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, 0), Quat::sIdentity(), new BoxShape(Vec3(cCharacterRadiusCrouching, 0.5f * cCharacterHeightCrouching + cCharacterRadiusCrouching, cCharacterRadiusCrouching))).Create().Get();
		break;
	}
}

void CharacterBaseTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update scene time
	mTime += inParams.mDeltaTime;

	// Determine controller input
	Vec3 control_input = Vec3::sZero();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))		control_input.SetZ(-1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))	control_input.SetZ(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))		control_input.SetX(1);
	if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		control_input.SetX(-1);
	if (control_input != Vec3::sZero())
		control_input = control_input.Normalized();

	// Rotate controls to align with the camera
	Vec3 cam_fwd = inParams.mCameraState.mForward;
	cam_fwd.SetY(0.0f);
	cam_fwd = cam_fwd.NormalizedOr(Vec3::sAxisX());
	Quat rotation = Quat::sFromTo(Vec3::sAxisX(), cam_fwd);
	control_input = rotation * control_input;

	// Check actions
	bool jump = false;
	bool switch_stance = false;
	for (int key = inParams.mKeyboard->GetFirstKey(); key != 0; key = inParams.mKeyboard->GetNextKey())
	{
		if (key == DIK_RSHIFT)
			switch_stance = true;
		else if (key == DIK_RCONTROL)
			jump = true;
	}

	HandleInput(control_input, jump, switch_stance, inParams.mDeltaTime);

	// Animate bodies
	if (!mRotatingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mRotatingBody, cRotatingPosition, Quat::sRotation(Vec3::sAxisY(), JPH_PI * Sin(mTime)), inParams.mDeltaTime);
	if (!mHorizontallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mHorizontallyMovingBody, cHorizontallyMovingPosition + Vec3(3.0f * Sin(mTime), 0, 0), cHorizontallyMovingOrientation, inParams.mDeltaTime);
	if (!mVerticallyMovingBody.IsInvalid())
		mBodyInterface->MoveKinematic(mVerticallyMovingBody, cVerticallyMovingPosition + Vec3(0, 1.75f * Sin(mTime), 0), cVerticallyMovingOrientation, inParams.mDeltaTime);

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
}

void CharacterBaseTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() { 
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [=]() {
		UIElement *configuration_settings = inUI->CreateMenu();

		inUI->CreateComboBox(configuration_settings, "Shape Type", { "Capsule", "Cylinder", "Box" }, (int)sShapeType, [](int inItem) { sShapeType = (EType)inItem; });
		AddConfigurationSettings(inUI, configuration_settings);
		inUI->CreateTextButton(configuration_settings, "Accept Changes", [=]() { RestartTest(); });
		inUI->ShowMenu(configuration_settings);
	});
}

void CharacterBaseTest::GetInitialCamera(CameraState& ioState) const
{
	// This will become the local space offset, look down the x axis and slightly down
	ioState.mPos = Vec3::sZero();
	ioState.mForward = Vec3(10.0f, -2.0f, 0).Normalized();
}

Mat44 CharacterBaseTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	// Pivot is center of character + distance behind based on the heading and pitch of the camera
	Vec3 fwd = Vec3(Cos(inCameraPitch) * Cos(inCameraHeading), Sin(inCameraPitch), Cos(inCameraPitch) * Sin(inCameraHeading));
	return Mat44::sTranslation(GetCharacterPosition() + Vec3(0, cCharacterHeightStanding + cCharacterRadiusStanding, 0) - 5.0f * fwd);
}

void CharacterBaseTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mRampBlocksTimeLeft);
}

void CharacterBaseTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mRampBlocksTimeLeft);
}

void CharacterBaseTest::DrawCharacterState(const CharacterBase *inCharacter, Mat44Arg inCharacterTransform, Vec3Arg inCharacterVelocity)
{
	// Draw current location
	// Drawing prior to update since the physics system state is also that prior to the simulation step (so that all detected collisions etc. make sense)
	mDebugRenderer->DrawCoordinateSystem(inCharacterTransform, 0.1f);

	// Determine color
	CharacterBase::EGroundState ground_state = inCharacter->GetGroundState();
	Color color;
	switch (ground_state)
	{
	case CharacterBase::EGroundState::OnGround:
		color = Color::sGreen;
		break;
	case CharacterBase::EGroundState::OnSteepGround:
		color = Color::sYellow;
		break;
	case CharacterBase::EGroundState::NotSupported:
		color = Color::sOrange;
		break;
	case CharacterBase::EGroundState::InAir:
	default:
		color = Color::sRed;
		break;
	}

	// Draw the state of the ground contact
	if (ground_state != CharacterBase::EGroundState::InAir)
	{
		Vec3 ground_position = inCharacter->GetGroundPosition();
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
	mDebugRenderer->DrawText3D(inCharacterTransform.GetTranslation(), StringFormat("Mat: %s\nHorizontal Vel: %.1f m/s\nVertical Vel: %.1f m/s", ground_material->GetDebugName(), (double)horizontal_velocity.Length(), (double)inCharacterVelocity.GetY()), color, 0.25f);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/HighSpeedTest.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Application/DebugUI.h>
#include <Utils/Log.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HighSpeedTest)
{
	JPH_ADD_BASE_CLASS(HighSpeedTest, Test)
}

const char *HighSpeedTest::sScenes[] =
{
	"Simple",
	"Convex Hull On Large Triangles",
	"Convex Hull On Terrain1",
};

int HighSpeedTest::sSelectedScene = 0;

void HighSpeedTest::CreateDominoBlocks(RVec3Arg inOffset, int inNumWalls, float inDensity, float inRadius)
{
	BodyCreationSettings box_settings;
	Ref<BoxShape> box_shape = new BoxShape(Vec3(0.9f, 1.0f, 0.1f));
	box_shape->SetDensity(inDensity); // Make box more heavy so the bouncing ball keeps a higher velocity
	box_settings.SetShape(box_shape);
	box_settings.mObjectLayer = Layers::MOVING;

	// U shaped set of thin boxes
	for (int i = 0; i < inNumWalls; ++i)
	{
		box_settings.mPosition = inOffset + Vec3(2.0f * i, 1, -1.1f - inRadius);
		mBodyInterface->CreateAndAddBody(box_settings, EActivation::DontActivate);

		box_settings.mPosition = inOffset + Vec3(2.0f * i, 1, +1.1f + inRadius);
		mBodyInterface->CreateAndAddBody(box_settings, EActivation::DontActivate);
	}

	box_settings.mPosition = inOffset + Vec3(-1.1f - inRadius, 1, 0);
	box_settings.mRotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI);
	mBodyInterface->CreateAndAddBody(box_settings, EActivation::DontActivate);
}

void HighSpeedTest::CreateDynamicObject(RVec3Arg inPosition, Vec3Arg inVelocity, Shape *inShape, EMotionQuality inMotionQuality)
{
	BodyCreationSettings creation_settings;
	creation_settings.SetShape(inShape);
	creation_settings.mFriction = 0.0f;
	creation_settings.mRestitution = 1.0f;
	creation_settings.mLinearDamping = 0.0f;
	creation_settings.mAngularDamping = 0.0f;
	creation_settings.mMotionQuality = inMotionQuality;
	creation_settings.mObjectLayer = Layers::MOVING;
	creation_settings.mPosition = inPosition;

	Body &body = *mBodyInterface->CreateBody(creation_settings);
	body.SetLinearVelocity(inVelocity);
	mBodyInterface->AddBody(body.GetID(), inVelocity.IsNearZero()? EActivation::DontActivate : EActivation::Activate);
}

void HighSpeedTest::CreateSimpleScene()
{
	// Floor
	CreateFloor();

	const float radius = 0.1f;
	const int num_walls = 5;
	const float density = 2000.0f;
	const float speed = 240.0f;

	RVec3 offset(0, 0, -30);

	{
		// U shaped set of thin walls
		TriangleList triangles;
		for (int i = 0; i < num_walls; ++i)
		{
			triangles.push_back(Triangle(Float3(2.0f*i-1,0,-1-radius), Float3(2.0f*i+1,0,-1-radius), Float3(2.0f*i,2,-1-radius)));
			triangles.push_back(Triangle(Float3(2.0f*i-1,0,1+radius), Float3(2.0f*i,2,1+radius), Float3(2.0f*i+1,0,1+radius)));
		}
		triangles.push_back(Triangle(Float3(-1-radius,0,-1), Float3(-1-radius,2,0), Float3(-1-radius,0,1)));
		Body &walls = *mBodyInterface->CreateBody(BodyCreationSettings(new MeshShapeSettings(triangles), offset, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		walls.SetRestitution(1.0f);
		walls.SetFriction(0.0f);
		mBodyInterface->AddBody(walls.GetID(), EActivation::DontActivate);

		// Fast moving sphere against mesh
		CreateDynamicObject(offset + Vec3(2.0f * num_walls - 1, 1, 0), Vec3(-speed, 0, -speed), new SphereShape(radius));
	}

	offset += Vec3(0, 0, 5);

	{
		// Create wall of domino blocks
		CreateDominoBlocks(offset, num_walls, density, radius);

		// Fast moving sphere against domino blocks
		CreateDynamicObject(offset + Vec3(2.0f * num_walls - 1, 1, 0), Vec3(-speed, 0, -speed), new SphereShape(radius));
	}

	offset += Vec3(0, 0, 5);

	{
		// Create wall of domino blocks
		CreateDominoBlocks(offset, num_walls, density, radius);

		// Fast moving scaled box against domino blocks
		CreateDynamicObject(offset + Vec3(2.0f * num_walls - 1, 1, 0), Vec3(-speed, 0, -speed), new ScaledShape(new BoxShape(Vec3::sReplicate(0.5f * radius), 0.01f), Vec3::sReplicate(2.0f)));
	}

	offset += Vec3(0, 0, 5);

	{
		// Fast moving box stuck in ground moving, one moving up, one moving down
		CreateDynamicObject(offset + Vec3(-1, 0, 0), Vec3(0, speed, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(1, 0, 0), Vec3(0, -speed, 0), new BoxShape(Vec3::sReplicate(radius)));
	}

	offset += Vec3(0, 0, 5);

	{
		// Single shape that has 4 walls to surround fast moving sphere
		BodyCreationSettings enclosing_settings;
		Ref<BoxShapeSettings> box_shape = new BoxShapeSettings(Vec3(1.0f, 1.0f, 0.1f));
		Ref<StaticCompoundShapeSettings> enclosing_shape = new StaticCompoundShapeSettings();
		enclosing_shape->AddShape(Vec3(0, 0, 1), Quat::sIdentity(), box_shape);
		enclosing_shape->AddShape(Vec3(0, 0, -1), Quat::sIdentity(), box_shape);
		enclosing_shape->AddShape(Vec3(1, 0, 0), Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI), box_shape);
		enclosing_shape->AddShape(Vec3(-1, 0, 0), Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI), box_shape);
		enclosing_settings.SetShapeSettings(enclosing_shape);
		enclosing_settings.mMotionType = EMotionType::Kinematic;
		enclosing_settings.mObjectLayer = Layers::MOVING;
		enclosing_settings.mPosition = offset + Vec3(0, 1, 0);
		Body &enclosing = *mBodyInterface->CreateBody(enclosing_settings);
		mBodyInterface->AddBody(enclosing.GetID(), EActivation::Activate);

		// Fast moving sphere in box
		CreateDynamicObject(offset + Vec3(0, 0.5f, 0), Vec3(-speed, 0, -0.5f * speed), new SphereShape(radius));
	}

	offset += Vec3(0, 0, 5);

	{
		// Two boxes on a collision course
		CreateDynamicObject(offset + Vec3(1, 0.5f, 0), Vec3(-speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(-1, 0.5f, 0), Vec3(speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
	}

	offset += Vec3(0, 0, 5);

	{
		// Two boxes on a collision course, off center
		CreateDynamicObject(offset + Vec3(1, 0.5f, 0), Vec3(-speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(-1, 0.5f, radius), Vec3(speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
	}

	offset += Vec3(0, 0, 5);

	{
		// Two boxes on a collision course, one discrete
		CreateDynamicObject(offset + Vec3(1, 0.5f, 0), Vec3(-speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(-1, 0.5f, 0), Vec3(60.0f, 0, 0), new BoxShape(Vec3::sReplicate(radius)), EMotionQuality::Discrete);
	}

	offset += Vec3(0, 0, 5);

	{
		// Two boxes on a collision course, one inactive
		CreateDynamicObject(offset + Vec3(1, 0.5f, 0), Vec3(-speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(0, 0.5f, 0), Vec3::sZero(), new BoxShape(Vec3::sReplicate(radius)));
	}

	offset += Vec3(0, 0, 5);

	{
		// Two boxes on a collision course, one inactive and discrete
		CreateDynamicObject(offset + Vec3(1, 0.5f, 0), Vec3(-speed, 0, 0), new BoxShape(Vec3::sReplicate(radius)));
		CreateDynamicObject(offset + Vec3(0, 0.5f, 0), Vec3::sZero(), new BoxShape(Vec3::sReplicate(radius)), EMotionQuality::Discrete);
	}

	offset += Vec3(0, 0, 5);

	{
		// Create long thin shape
		BoxShapeSettings box_settings(Vec3(0.05f, 0.8f, 0.03f), 0.015f);
		box_settings.SetEmbedded();
		BodyCreationSettings body_settings(&box_settings, offset + Vec3(0, 1, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
		body_settings.mMotionQuality = EMotionQuality::LinearCast;
		body_settings.mRestitution = 0.0f;
		body_settings.mFriction = 1.0f;

		Body &body = *mPhysicsSystem->GetBodyInterface().CreateBody(body_settings);
		body.SetLinearVelocity(Vec3(0, -100.0f, 0));
		mPhysicsSystem->GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
	}

	offset += Vec3(0, 0, 5);

	{
		// Create long thin shape under 45 degrees
		BoxShapeSettings box_settings(Vec3(0.05f, 0.8f, 0.03f), 0.015f);
		box_settings.SetEmbedded();
		BodyCreationSettings body_settings(&box_settings, offset + Vec3(0, 1, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
		body_settings.mMotionQuality = EMotionQuality::LinearCast;
		body_settings.mRestitution = 0.0f;
		body_settings.mFriction = 1.0f;

		Body &body = *mPhysicsSystem->GetBodyInterface().CreateBody(body_settings);
		body.SetLinearVelocity(Vec3(0, -100.0f, 0));
		mPhysicsSystem->GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
	}

	offset += Vec3(0, 0, 5);

	{
		// Create long thin shape with restitution
		BoxShapeSettings box_settings(Vec3(0.05f, 0.8f, 0.03f), 0.015f);
		box_settings.SetEmbedded();
		BodyCreationSettings body_settings(&box_settings, offset + Vec3(0, 1, 0), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
		body_settings.mMotionQuality = EMotionQuality::LinearCast;
		body_settings.mRestitution = 1.0f;
		body_settings.mFriction = 1.0f;

		Body &body = *mPhysicsSystem->GetBodyInterface().CreateBody(body_settings);
		body.SetLinearVelocity(Vec3(0, -100.0f, 0));
		mPhysicsSystem->GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
	}

	offset += Vec3(0, 0, 5);

	{
		// Create long thin shape under 45 degrees with restitution
		BoxShapeSettings box_settings(Vec3(0.05f, 0.8f, 0.03f), 0.015f);
		box_settings.SetEmbedded();
		BodyCreationSettings body_settings(&box_settings, offset + Vec3(0, 1, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
		body_settings.mMotionQuality = EMotionQuality::LinearCast;
		body_settings.mRestitution = 1.0f;
		body_settings.mFriction = 1.0f;

		Body &body = *mPhysicsSystem->GetBodyInterface().CreateBody(body_settings);
		body.SetLinearVelocity(Vec3(0, -100.0f, 0));
		mPhysicsSystem->GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
	}
}

void HighSpeedTest::CreateFastSmallConvexObjects()
{
	// Create small convex hull
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
	ConvexHullShapeSettings convex_settings(vertices);
	convex_settings.SetEmbedded();
	BodyCreationSettings body_settings(&convex_settings, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	body_settings.mMotionQuality = EMotionQuality::LinearCast;

	// Create many instances with high velocity
	default_random_engine rnd;
	uniform_real_distribution<float> restitution_distrib(0.0f, 0.1f);
	uniform_real_distribution<float> velocity_distrib(-10.0f, 10.0f);
	for (int x = -25; x < 25; ++x)
		for (int y = -25 ; y < 25; ++y)
		{
			// Cast a ray to find the terrain
			RVec3 origin(Real(x), 100.0_r, Real(y));
			Vec3 direction(0, -100.0f, 0);
			RRayCast ray { origin, direction };
			RayCastResult hit;
			if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING), SpecifiedObjectLayerFilter(Layers::NON_MOVING)))
			{
				// Place 10m above terrain
				body_settings.mPosition = ray.GetPointOnRay(hit.mFraction) + RVec3(0, 10, 0);
				body_settings.mRotation = Quat::sRandom(rnd);
				body_settings.mRestitution = restitution_distrib(rnd);

				Body &body = *mPhysicsSystem->GetBodyInterface().CreateBody(body_settings);
				body.SetLinearVelocity(Vec3(velocity_distrib(rnd), -100.0f, velocity_distrib(rnd)));
				mPhysicsSystem->GetBodyInterface().AddBody(body.GetID(), EActivation::Activate);
			}
		}
}

void HighSpeedTest::CreateConvexOnLargeTriangles()
{
	// Create floor
	CreateLargeTriangleFloor();

	CreateFastSmallConvexObjects();
}

void HighSpeedTest::CreateConvexOnTerrain1()
{
	// Load scene
	Ref<PhysicsScene> scene;
	if (!ObjectStreamIn::sReadObject("Assets/terrain1.bof", scene))
		FatalError("Failed to load scene");
	for (BodyCreationSettings &body : scene->GetBodies())
		body.mObjectLayer = Layers::NON_MOVING;
	scene->FixInvalidScales();
	scene->CreateBodies(mPhysicsSystem);

	CreateFastSmallConvexObjects();
}

void HighSpeedTest::Initialize()
{
	switch (sSelectedScene)
	{
	case 0:
		CreateSimpleScene();
		break;

	case 1:
		CreateConvexOnLargeTriangles();
		break;

	case 2:
		CreateConvexOnTerrain1();
		break;

	default:
		JPH_ASSERT(false);
		break;
	}
}

void HighSpeedTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSelectedScene = i; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

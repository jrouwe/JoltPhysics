// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/EnhancedInternalEdgeRemovalTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(EnhancedInternalEdgeRemovalTest)
{
	JPH_ADD_BASE_CLASS(EnhancedInternalEdgeRemovalTest, Test)
}

void EnhancedInternalEdgeRemovalTest::CreateSlidingObjects(RVec3Arg inStart)
{
	// Slide the shapes over the grid of boxes
	RVec3 pos = inStart - RVec3(0, 0, 12.0_r);
	static const char *labels[] = { "Normal", "Enhanced edge removal" };
	for (int enhanced_removal = 0; enhanced_removal < 2; ++enhanced_removal)
	{
		// A box
		BodyCreationSettings box_bcs(new BoxShape(Vec3::sReplicate(2.0f)), pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_bcs.mLinearVelocity = Vec3(20, 0, 0);
		box_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
		BodyID id = mBodyInterface->CreateAndAddBody(box_bcs, EActivation::Activate);
		SetBodyLabel(id, labels[enhanced_removal]);
		pos += RVec3(0, 0, 5.0_r);

		// A sphere
		BodyCreationSettings sphere_bcs(new SphereShape(2.0f), pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		sphere_bcs.mLinearVelocity = Vec3(20, 0, 0);
		sphere_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
		id = mBodyInterface->CreateAndAddBody(sphere_bcs, EActivation::Activate);
		SetBodyLabel(id, labels[enhanced_removal]);
		pos += RVec3(0, 0, 5.0_r);

		// Compound
		RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.1f));
		StaticCompoundShapeSettings compound;
		compound.SetEmbedded();
		for (int x = 0; x < 2; ++x)
			for (int y = 0; y < 2; ++y)
				for (int z = 0; z < 2; ++z)
					compound.AddShape(Vec3(x == 0? -1.9f : 1.9f, y == 0? -1.9f : 1.9f, z == 0? -1.9f : 1.9f), Quat::sIdentity(), box);
		BodyCreationSettings compound_bcs(&compound, pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		compound_bcs.mLinearVelocity = Vec3(20, 0, 0);
		compound_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
		id = mBodyInterface->CreateAndAddBody(compound_bcs, EActivation::Activate);
		SetBodyLabel(id, labels[enhanced_removal]);
		pos += RVec3(0, 0, 7.0_r);
	}
}

void EnhancedInternalEdgeRemovalTest::Initialize()
{
	// This test creates a grid of connected boxes and tests that objects don't hit the internal edges
	{
		StaticCompoundShapeSettings compound_settings;
		compound_settings.SetEmbedded();
		constexpr float size = 2.0f;
		RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(0.5f * size));
		for (int x = -10; x < 10; ++x)
			for (int z = -10; z < 10; ++z)
				compound_settings.AddShape(Vec3(size * x, 0, size * z), Quat::sIdentity(), box_shape);
		BodyID id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(&compound_settings, RVec3(0, -1, -40), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		SetBodyLabel(id, "Dense grid of boxes");

		CreateSlidingObjects(RVec3(-18, 1.9_r, -40.0_r));
	}

	// This tests if objects do not collide with internal edges
	{
		// Create a dense grid of triangles so that we have a large chance of hitting an internal edge
		constexpr float size = 2.0f;
		TriangleList triangles;
		for (int x = -10; x < 10; ++x)
			for (int z = -10; z < 10; ++z)
			{
				float x1 = size * x;
				float z1 = size * z;
				float x2 = x1 + size;
				float z2 = z1 + size;

				Float3 v1 = Float3(x1, 0, z1);
				Float3 v2 = Float3(x2, 0, z1);
				Float3 v3 = Float3(x1, 0, z2);
				Float3 v4 = Float3(x2, 0, z2);

				triangles.push_back(Triangle(v1, v3, v4));
				triangles.push_back(Triangle(v1, v4, v2));
			}

		MeshShapeSettings mesh_settings(triangles);
		mesh_settings.mActiveEdgeCosThresholdAngle = -1.0f; // Turn off regular active edge determination so that we only rely on the mEnhancedInternalEdgeRemoval flag
		mesh_settings.SetEmbedded();
		BodyID id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(&mesh_settings, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		SetBodyLabel(id, "Dense triangle mesh");

		CreateSlidingObjects(RVec3(-18, 1.9_r, 0));
	}

	// This test tests that we only ignore edges that are shared with voided triangles
	{
		// Create an L shape mesh lying on its back
		TriangleList triangles;
		constexpr float height = 0.5f;
		constexpr float half_width = 5.0f;
		constexpr float half_length = 2.0f;
		triangles.push_back(Triangle(Float3(-half_length, 0, half_width), Float3(half_length, 0, -half_width), Float3(-half_length, 0, -half_width)));
		triangles.push_back(Triangle(Float3(-half_length, 0, half_width), Float3(half_length, 0, half_width), Float3(half_length, 0, -half_width)));
		triangles.push_back(Triangle(Float3(half_length, height, half_width), Float3(half_length, height, -half_width), Float3(half_length, 0, half_width)));
		triangles.push_back(Triangle(Float3(half_length, 0, half_width), Float3(half_length, height, -half_width), Float3(half_length, 0, -half_width)));
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new MeshShapeSettings(triangles), RVec3(0, 0, 30), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

		// Roll a sphere towards the edge pointing upwards
		float z = 28.0f;
		for (int enhanced_removal = 0; enhanced_removal < 2; ++enhanced_removal)
		{
			// A sphere
			BodyCreationSettings sphere_bcs(new SphereShape(1.0f), RVec3(0, 1, z), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			sphere_bcs.mLinearVelocity = Vec3(20, 0, 0);
			sphere_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
			mBodyInterface->CreateAndAddBody(sphere_bcs, EActivation::Activate);
			z += 4.0f;
		}
	}

	// This tests that fast moving spheres rolling over a triangle will not be affected by internal edges
	{
		// Create a flat plane
		MeshShapeSettings plane_mesh({
			{
				Float3(-10, 0, -10),
				Float3(-10, 0, 10),
				Float3(10, 0, 10)
			},
			{
				Float3(-10, 0, -10),
				Float3(10, 0, 10),
				Float3(10, 0, -10)
			},
		});
		plane_mesh.SetEmbedded();
		BodyCreationSettings level_plane(&plane_mesh, RVec3(-10, 0, 50), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		level_plane.mFriction = 1;
		BodyID id = mBodyInterface->CreateAndAddBody(level_plane, EActivation::DontActivate);
		SetBodyLabel(id, "Dense triangle mesh");

		// Roll a ball over it
		BodyCreationSettings level_ball(new SphereShape(0.5f), RVec3(-10, 1, 41), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		level_ball.mEnhancedInternalEdgeRemoval = true;
		level_ball.mFriction = 1;
		level_ball.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		level_ball.mMassPropertiesOverride.mMass = 1;
		mLevelBall = mBodyInterface->CreateAndAddBody(level_ball, EActivation::Activate);

		// Create a sloped plane
		BodyCreationSettings slope_plane(&plane_mesh, RVec3(10, 0, 50), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(45)), EMotionType::Static, Layers::NON_MOVING);
		slope_plane.mFriction = 1;
		id = mBodyInterface->CreateAndAddBody(slope_plane, EActivation::DontActivate);
		SetBodyLabel(id, "Dense triangle mesh");

		// Roll a ball over it
		BodyCreationSettings slope_ball(new SphereShape(0.5f), RVec3(10, 8, 44), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		slope_ball.mEnhancedInternalEdgeRemoval = true;
		slope_ball.mFriction = 1;
		slope_ball.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		slope_ball.mMassPropertiesOverride.mMass = 1;
		mBodyInterface->CreateAndAddBody(slope_ball, EActivation::Activate);
	}

	// This tests a previous bug where a compound shape will fall through a box because features are voided by accident.
	// This is because both boxes of the compound shape collide with the top face of the static box. The big box will have a normal
	// that is aligned with the face so will be processed immediately. This will void the top face of the static box. The small box,
	// which collides with an edge of the top face will not be processed. This will cause the small box to penetrate the face.
	{
		// A box
		BodyCreationSettings box_bcs(new BoxShape(Vec3::sReplicate(2.5f)), RVec3(0, 0, 70), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		mBodyInterface->CreateAndAddBody(box_bcs, EActivation::DontActivate);

		// Compound
		StaticCompoundShapeSettings compound;
		compound.SetEmbedded();
		compound.AddShape(Vec3(-2.5f, 0, 0), Quat::sIdentity(), new BoxShape(Vec3(2.5f, 0.1f, 0.1f)));
		compound.AddShape(Vec3(0.1f, 0, 0), Quat::sIdentity(), new BoxShape(Vec3(0.1f, 1, 1)));
		BodyCreationSettings compound_bcs(&compound, RVec3(2, 5, 70), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		compound_bcs.mEnhancedInternalEdgeRemoval = true;
		mBodyInterface->CreateAndAddBody(compound_bcs, EActivation::Activate);
	}

	// Create a super dense grid of triangles
	{
		constexpr float size = 0.25f;
		TriangleList triangles;
		for (int x = -100; x < 100; ++x)
			for (int z = -5; z < 5; ++z)
			{
				float x1 = size * x;
				float z1 = size * z;
				float x2 = x1 + size;
				float z2 = z1 + size;

				Float3 v1 = Float3(x1, 0, z1);
				Float3 v2 = Float3(x2, 0, z1);
				Float3 v3 = Float3(x1, 0, z2);
				Float3 v4 = Float3(x2, 0, z2);

				triangles.push_back(Triangle(v1, v3, v4));
				triangles.push_back(Triangle(v1, v4, v2));
			}

		MeshShapeSettings mesh_settings(triangles);
		mesh_settings.mActiveEdgeCosThresholdAngle = -1.0f; // Turn off regular active edge determination so that we only rely on the mEnhancedInternalEdgeRemoval flag
		mesh_settings.SetEmbedded();
		BodyID id = mBodyInterface->CreateAndAddBody(BodyCreationSettings(&mesh_settings, RVec3(0, 0, 80), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
		SetBodyLabel(id, "Dense triangle mesh");

		BodyCreationSettings box_bcs(new BoxShape(Vec3::sReplicate(1.0f)), RVec3(-24, 0.9_r, 80), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_bcs.mLinearVelocity = Vec3(20, 0, 0);
		box_bcs.mEnhancedInternalEdgeRemoval = true;
		mBodyInterface->CreateAndAddBody(box_bcs, EActivation::Activate);
	}
}

void EnhancedInternalEdgeRemovalTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Increase rotation speed of the ball on the flat plane
	mBodyInterface->AddTorque(mLevelBall, Vec3(JPH_PI * 4, 0, 0));
}

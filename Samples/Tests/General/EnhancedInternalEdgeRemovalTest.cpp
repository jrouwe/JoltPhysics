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
	RVec3 pos = inStart - RVec3(0, 0, 8.5_r);
	for (int enhanced_removal = 0; enhanced_removal < 2; ++enhanced_removal)
	{
		// A box
		BodyCreationSettings box_bcs(new BoxShape(Vec3::sReplicate(2.0f)), pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box_bcs.mLinearVelocity = Vec3(20, 0, 0);
		box_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
		mBodyInterface->CreateAndAddBody(box_bcs, EActivation::Activate);
		pos += RVec3(0, 0, 5.0_r);

		// A sphere
		BodyCreationSettings sphere_bcs(new SphereShape(2.0f), pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		sphere_bcs.mLinearVelocity = Vec3(20, 0, 0);
		sphere_bcs.mEnhancedInternalEdgeRemoval = enhanced_removal == 1;
		mBodyInterface->CreateAndAddBody(sphere_bcs, EActivation::Activate);
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
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(&compound_settings, RVec3(0, -1, -40), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

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
		mesh_settings.mActiveEdgeCosThresholdAngle = FLT_MAX; // Turn off regular active edge determination so that we only rely on the mEnhancedInternalEdgeRemoval flag
		mesh_settings.SetEmbedded();
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(&mesh_settings, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

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
}

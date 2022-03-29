// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ActiveEdgesTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Geometry/Triangle.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ActiveEdgesTest) 
{ 
	JPH_ADD_BASE_CLASS(ActiveEdgesTest, Test) 
}

void ActiveEdgesTest::Initialize()
{
	const float cWidth = 5.0f;
	const float cLength = 10.0f;

	// Setings for a frictionless box
	Ref<BoxShape> box_shape = new BoxShape(Vec3(1.0f, 1.0f, 1.0f), cDefaultConvexRadius);
	BodyCreationSettings box_settings(box_shape, Vec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	box_settings.mFriction = 0.0f;
	box_settings.mLinearDamping = 0.0f;
	box_settings.mAllowSleeping = false;

	// Create various triangle strips
	TriangleList triangles;
	for (int angle = -90; angle <= 90; angle++)
	{
		// Under which normal we want to place the block
		Vec3 desired_normal = angle < 0? Vec3(0, 1, -1).Normalized() : Vec3(0, 1, 0);
		float best_dot = -FLT_MAX;

		// Place segments
		float x = cWidth * angle;
		Vec3 v1(x, 0.0f, -0.5f * cLength);
		Vec3 v2(x + cWidth, 0.0f, -0.5f * cLength);
		for (int total_angle = 0, cur_segment = 0; abs(total_angle) <= 90 && cur_segment < 90; total_angle += angle, ++cur_segment)
		{
			// Determine positions of end of this segment
			float total_angle_rad = DegreesToRadians(float(total_angle));
			Quat rotation = Quat::sRotation(Vec3::sAxisX(), total_angle_rad);
			Vec3 delta = cLength * rotation.RotateAxisZ();
			Vec3 v3 = v1 + delta;
			Vec3 v4 = v2 + delta;

			// Check if this segment is the best segment to place the dynamic block on
			Vec3 normal = (v3 - v1).Cross(v2 - v1).Normalized();
			float dot = normal.Dot(desired_normal);
			if (dot > best_dot)
			{
				best_dot = dot;
				box_settings.mPosition = (v1 + v2 + v3 + v4) / 4 + normal;
				box_settings.mRotation = rotation;
			}
				
			// Add segment
			triangles.push_back(Triangle(v1, v3, v4));
			triangles.push_back(Triangle(v1, v4, v2));

			// Add segment mirrored in Z axis
			if (cur_segment != 0)
			{
				Vec3 flip(1, 1, -1);
				triangles.push_back(Triangle(flip * v1, flip * v4, flip * v3));
				triangles.push_back(Triangle(flip * v1, flip * v2, flip * v4));
			}

			// The end of the segment will be the start for the next iteration
			v1 = v3;
			v2 = v4;
		}

		// Place box on best segment
		Body &body = *mBodyInterface->CreateBody(box_settings);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);

		// For convex segments give the block a push
		if (angle >= 0)
			body.SetLinearVelocity(Vec3(0, 0, 2.0f));
	}

	// Mesh
	BodyCreationSettings mesh_settings(new MeshShapeSettings(triangles), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	mesh_settings.mFriction = 0.0f;
	Body &mesh = *mBodyInterface->CreateBody(mesh_settings);
	mBodyInterface->AddBody(mesh.GetID(), EActivation::DontActivate);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/MeshShapeUserDataTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(MeshShapeUserDataTest)
{
	JPH_ADD_BASE_CLASS(MeshShapeUserDataTest, Test)
}

void MeshShapeUserDataTest::Initialize()
{
	// Create regular grid of triangles
	uint32 user_data = 0;
	TriangleList triangles;
	for (int x = -10; x < 10; ++x)
		for (int z = -10; z < 10; ++z)
		{
			float x1 = 10.0f * x;
			float z1 = 10.0f * z;
			float x2 = x1 + 10.0f;
			float z2 = z1 + 10.0f;

			Float3 v1 = Float3(x1, 0, z1);
			Float3 v2 = Float3(x2, 0, z1);
			Float3 v3 = Float3(x1, 0, z2);
			Float3 v4 = Float3(x2, 0, z2);

			triangles.push_back(Triangle(v1, v3, v4, 0, user_data++));
			triangles.push_back(Triangle(v1, v4, v2, 0, user_data++));
		}

	// Shuffle the triangles
	std::default_random_engine random;
	std::shuffle(triangles.begin(), triangles.end(), random);

	MeshShapeSettings mesh_settings(triangles);
	mesh_settings.mPerTriangleUserData = true;
	mesh_settings.SetEmbedded();

	// Floor
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(&mesh_settings, RVec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// 1 body with zero friction
	BodyCreationSettings bcs(new BoxShape(Vec3::sReplicate(2.0f)), RVec3(0, 55.0f, -50.0f), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
	bcs.mFriction = 0.0f;
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
}

void MeshShapeUserDataTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Cast a ray
	RayCastResult hit;
	RRayCast ray(inParams.mCameraState.mPos, inParams.mCameraState.mForward * 100.0f);
	mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit);

	// Get body (if there was a hit)
	BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), hit.mBodyID);
	if (lock.SucceededAndIsInBroadPhase())
	{
		// Get the user data
		uint32 user_data = static_cast<const MeshShape *>(lock.GetBody().GetShape())->GetTriangleUserData(hit.mSubShapeID2);
		RVec3 hit_pos = ray.GetPointOnRay(hit.mFraction);
		mDebugRenderer->DrawText3D(hit_pos, StringFormat("UserData: %d", user_data).c_str());
	}
}

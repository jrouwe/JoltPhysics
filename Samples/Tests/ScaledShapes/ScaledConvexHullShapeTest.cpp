// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledConvexHullShapeTest.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledConvexHullShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledConvexHullShapeTest, Test)
}

void ScaledConvexHullShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create tetrahedron
	Array<Vec3> tetrahedron;
	tetrahedron.push_back(Vec3::sZero());
	tetrahedron.push_back(Vec3(10, 0, 12.5f));
	tetrahedron.push_back(Vec3(15, 0, 2.5f));
	tetrahedron.push_back(Vec3(10, -5, 5));

	// Create vertices for box
	Array<Vec3> box;
	box.push_back(Vec3(1, 2, 3));
	box.push_back(Vec3(-1, 2, 3));
	box.push_back(Vec3(1, -2, 3));
	box.push_back(Vec3(-1, -2, 3));
	box.push_back(Vec3(1, 2, -3));
	box.push_back(Vec3(-1, 2, -3));
	box.push_back(Vec3(1, -2, -3));
	box.push_back(Vec3(-1, -2, -3));

	// Rotate and translate vertices
	Mat44 m = Mat44::sTranslation(Vec3(3.0f, -2.0f, 1.0f)) * Mat44::sRotationY(0.2f * JPH_PI) * Mat44::sRotationZ(0.1f * JPH_PI);
	for (Vec3 &v : box)
		v = m * v;

	// Create convex hulls
	RefConst<ShapeSettings> hull_shape[2] = { new ConvexHullShapeSettings(tetrahedron), new ConvexHullShapeSettings(box) };

	for (int i = 0; i < 2; ++i)
	{
		// Original shape
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(hull_shape[i], RVec3(-40, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

		// Uniformly scaled shape
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3::sReplicate(0.25f)), RVec3(-20, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

		// Non-uniform scaled shape
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(0.25f, 0.5f, 1.5f)), RVec3(0, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

		// Flipped in 2 axis
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(-0.25f, 0.5f, -1.5f)), RVec3(20, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

		// Inside out
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(-0.25f, 0.5f, 1.5f)), RVec3(40, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
	}
}

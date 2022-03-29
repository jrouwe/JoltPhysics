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
	vector<Vec3> tetrahedron;
	tetrahedron.push_back(Vec3::sZero());
	tetrahedron.push_back(Vec3(10, 0, 12.5f));
	tetrahedron.push_back(Vec3(15, 0, 2.5f));
	tetrahedron.push_back(Vec3(10, -5, 5));

	// Create vertices for box
	vector<Vec3> box;
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
		Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(hull_shape[i], Vec3(-40, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

		// Uniformly scaled shape
		Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3::sReplicate(0.25f)), Vec3(-20, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

		// Non-uniform scaled shape
		Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(0.25f, 0.5f, 1.5f)), Vec3(0, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

		// Flipped in 2 axis
		Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(-0.25f, 0.5f, -1.5f)), Vec3(20, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

		// Inside out
		Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(hull_shape[i], Vec3(-0.25f, 0.5f, 1.5f)), Vec3(40, 10, i * 20.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);
	}
}

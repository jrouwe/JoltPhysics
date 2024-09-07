// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/RotatedTranslatedShapeTest.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(RotatedTranslatedShapeTest)
{
	JPH_ADD_BASE_CLASS(RotatedTranslatedShapeTest, Test)
}

void RotatedTranslatedShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create a cone centered on the origin with the point pointing upwards
	Array<Vec3> points;
	points.push_back(Vec3(0, 2.5f, 0));
	for (float a = 0; a < DegreesToRadians(360); a += DegreesToRadians(36))
		points.push_back(Vec3(Sin(a), -2.5f, Cos(a)));
	Ref<ConvexHullShapeSettings> convex_hull = new ConvexHullShapeSettings(points);

	// Offset and rotate so that the cone is upside down on its point
	Ref<RotatedTranslatedShapeSettings> rot_trans = new RotatedTranslatedShapeSettings(Vec3(0, 2.5f, 0), Quat::sRotation(Vec3::sAxisX(), JPH_PI), convex_hull);

	// Place at 0 so that the point touches the floor
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(rot_trans, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}

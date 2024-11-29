// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/SimShapeFilterTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SimShapeFilterTest)
{
	JPH_ADD_BASE_CLASS(SimShapeFilterTest, Test)
}

SimShapeFilterTest::~SimShapeFilterTest()
{
	// Unregister shape filter
	mPhysicsSystem->SetSimShapeFilter(nullptr);
}

void SimShapeFilterTest::Initialize()
{
	// Register shape filter
	mPhysicsSystem->SetSimShapeFilter(&mShapeFilter);

	// Floor
	CreateFloor();

	// Platform
	mShapeFilter.mPlatformID = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(5.0f, 0.5f, 5.0f)), RVec3(0, 7.5f, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Compound shape
	Ref<Shape> capsule = new CapsuleShape(2, 0.1f);
	capsule->SetUserData(1); // Don't want the capsule to collide with the platform
	Ref<Shape> sphere = new SphereShape(0.5f);
	sphere->SetUserData(1); // Don't want the sphere to collide with the platform
	Ref<Shape> box = new BoxShape(Vec3::sReplicate(0.5f));
	Ref<StaticCompoundShapeSettings> compound = new StaticCompoundShapeSettings;
	compound->AddShape(Vec3::sZero(), Quat::sIdentity(), capsule);
	compound->AddShape(Vec3(0, -2, 0), Quat::sIdentity(), sphere);
	compound->AddShape(Vec3(0, 2, 0), Quat::sIdentity(), box);

	// Create compound above the platform
	BodyCreationSettings compound_body(compound, RVec3(0, 15, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	mShapeFilter.mCompoundID[0] = mBodyInterface->CreateAndAddBody(compound_body, EActivation::Activate);

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateClothWithFixatedCorners(20, 20, 0.2f), RVec3(10, 10.0f, 0), Quat::sIdentity(), Layers::MOVING);
	mShapeFilter.mClothID = mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Create compound above the cloth
	compound_body.mPosition = RVec3(10, 15, 0);
	mShapeFilter.mCompoundID[1] = mBodyInterface->CreateAndAddBody(compound_body, EActivation::Activate);
}

bool SimShapeFilterTest::Filter::ShouldCollide(const Body &inBody1, const Shape *inShape1, const SubShapeID &inSubShapeIDOfShape1, const Body &inBody2, const Shape *inShape2, const SubShapeID &inSubShapeIDOfShape2) const
{
	// If the platform/cloth is colliding with the compound, filter out collisions where the shape has user data 1
	if (inBody1.GetID() == mPlatformID || inBody1.GetID() == mClothID)
	{
		for (int i = 0; i < 2; ++i)
			if (inBody2.GetID() == mCompoundID[i])
				return inShape2->GetUserData() != 1;
	}
	else if (inBody2.GetID() == mPlatformID || inBody2.GetID() == mClothID)
	{
		for (int i = 0; i < 2; ++i)
			if (inBody1.GetID() == mCompoundID[i])
				return inShape1->GetUserData() != 1;
	}
	return true;
}

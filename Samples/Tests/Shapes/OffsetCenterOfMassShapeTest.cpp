// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/OffsetCenterOfMassShapeTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(OffsetCenterOfMassShapeTest)
{
	JPH_ADD_BASE_CLASS(OffsetCenterOfMassShapeTest, Test)
}

void OffsetCenterOfMassShapeTest::Initialize()
{
	// Floor
	Body &floor = CreateFloor();
	floor.SetFriction(1.0f);

	Ref<ShapeSettings> sphere = new SphereShapeSettings(1.0f);
	Ref<OffsetCenterOfMassShapeSettings> left = new OffsetCenterOfMassShapeSettings(Vec3(-1, 0, 0), sphere);
	Ref<OffsetCenterOfMassShapeSettings> right = new OffsetCenterOfMassShapeSettings(Vec3(1, 0, 0), sphere);

	// Sphere with center of mass moved to the left side
	Body &body_left = *mBodyInterface->CreateBody(BodyCreationSettings(left, RVec3(-5, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_left.SetFriction(1.0f);
	mBodyInterface->AddBody(body_left.GetID(), EActivation::Activate);

	// Sphere with center of mass centered
	Body &body_center = *mBodyInterface->CreateBody(BodyCreationSettings(sphere, RVec3(0, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_center.SetFriction(1.0f);
	mBodyInterface->AddBody(body_center.GetID(), EActivation::Activate);

	// Sphere with center of mass moved to the right side
	Body &body_right = *mBodyInterface->CreateBody(BodyCreationSettings(right, RVec3(5, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body_right.SetFriction(1.0f);
	mBodyInterface->AddBody(body_right.GetID(), EActivation::Activate);

	// Create body and apply a large angular impulse so see that it spins around the COM
	BodyCreationSettings bcs(new OffsetCenterOfMassShapeSettings(Vec3(-3, 0, 0), new SphereShapeSettings(1.0f)), RVec3(-5, 5, 10), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mGravityFactor = 0.0f;
	bcs.mLinearDamping = 0.0f;
	bcs.mAngularDamping = 0.0f;
	Body *body_rotating1 = mBodyInterface->CreateBody(bcs);
    mBodyInterface->AddBody(body_rotating1->GetID(), EActivation::Activate);
	body_rotating1->AddAngularImpulse(Vec3(0, 1.0e6f, 0));

	// Create the same body but this time apply a torque
	bcs.mPosition = RVec3(5, 5, 10);
	Body *body_rotating2 = mBodyInterface->CreateBody(bcs);
    mBodyInterface->AddBody(body_rotating2->GetID(), EActivation::Activate);
	body_rotating2->AddTorque(Vec3(0, 1.0e6f * 60.0f, 0)); // Assuming physics sim is at 60Hz here, otherwise the bodies won't rotate with the same speed
}

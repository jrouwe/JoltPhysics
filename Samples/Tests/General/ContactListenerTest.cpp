// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ContactListenerTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ContactListenerTest) 
{ 
	JPH_ADD_BASE_CLASS(ContactListenerTest, Test) 
}

void ContactListenerTest::Initialize()
{
	// Floor
	CreateFloor();

	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 1.0f, 2.0f));

	// Dynamic body 1
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body1.SetAllowSleeping(false);
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Dynamic body 2
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, Vec3(5, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Dynamic body 3
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(2.0f), Vec3(10, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body3.SetAllowSleeping(false);
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Dynamic body 4
	Ref<StaticCompoundShapeSettings> compound_shape = new StaticCompoundShapeSettings;
	compound_shape->AddShape(Vec3::sZero(), Quat::sIdentity(), new CapsuleShape(5, 1));
	compound_shape->AddShape(Vec3(0, -5, 0), Quat::sIdentity(), new SphereShape(2));
	compound_shape->AddShape(Vec3(0, 5, 0), Quat::sIdentity(), new SphereShape(2));
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape, Vec3(15, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body4.SetAllowSleeping(false);
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);
	
	// Store bodies for later use
	mBody[0] = &body1;
	mBody[1] = &body2;
	mBody[2] = &body3;
	mBody[3] = &body4;
}

ValidateResult ContactListenerTest::OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult)
{
	// Body 1 and 2 should never collide
	return ((&inBody1 == mBody[0] && &inBody2 == mBody[1]) || (&inBody1 == mBody[1] && &inBody2 == mBody[0]))? ValidateResult::RejectAllContactsForThisBodyPair : ValidateResult::AcceptAllContactsForThisBodyPair;
}

void ContactListenerTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Make body 1 bounce only when a new contact point is added but not when it is persisted (its restitution is normally 0)
	if (&inBody1 == mBody[0] || &inBody2 == mBody[0])
	{
		JPH_ASSERT(ioSettings.mCombinedRestitution == 0.0f);
		ioSettings.mCombinedRestitution = 1.0f;
	}
}

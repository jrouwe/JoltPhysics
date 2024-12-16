// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ContactListenerTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
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

	// Dynamic body 1, this body will have restitution 1 for new contacts and restitution 0 for persisting contacts
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body1.SetAllowSleeping(false);
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Dynamic body 2
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, RVec3(5, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Dynamic body 3
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(2.0f), RVec3(10, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body3.SetAllowSleeping(false);
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Dynamic body 4
	Ref<StaticCompoundShapeSettings> compound_shape = new StaticCompoundShapeSettings;
	compound_shape->AddShape(Vec3::sZero(), Quat::sIdentity(), new CapsuleShape(5, 1));
	compound_shape->AddShape(Vec3(0, -5, 0), Quat::sIdentity(), new SphereShape(2));
	compound_shape->AddShape(Vec3(0, 5, 0), Quat::sIdentity(), new SphereShape(2));
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape, RVec3(15, 10, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
	body4.SetAllowSleeping(false);
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

	// Dynamic body 5, a cube with a bigger cube surrounding it that acts as a sensor
	Ref<StaticCompoundShapeSettings> compound_shape2 = new StaticCompoundShapeSettings;
	compound_shape2->AddShape(Vec3::sZero(), Quat::sIdentity(), new BoxShape(Vec3::sReplicate(1)));
	compound_shape2->AddShape(Vec3::sZero(), Quat::sIdentity(), new BoxShape(Vec3::sReplicate(2))); // This will become a sensor in the contact callback
	BodyCreationSettings bcs5(compound_shape2, RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs5.mUseManifoldReduction = false; // Needed in order to prevent the physics system from combining contacts between sensor and non-sensor sub shapes
	Body &body5 = *mBodyInterface->CreateBody(bcs5);
	body5.SetAllowSleeping(false);
	mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);

	// Store bodies for later use
	mBody[0] = &body1;
	mBody[1] = &body2;
	mBody[2] = &body3;
	mBody[3] = &body4;
	mBody[4] = &body5;
}

void ContactListenerTest::PostPhysicsUpdate(float inDeltaTime)
{
	// Check that predicted velocities match actual velocities
	lock_guard lock(mPredictedVelocitiesMutex);
	for (const PredictedVelocity &v : mPredictedVelocities)
	{
		BodyLockRead body_lock(mPhysicsSystem->GetBodyLockInterface(), v.mBodyID);
		if (body_lock.Succeeded())
		{
			const Body &body = body_lock.GetBody();
			Vec3 linear_velocity = body.GetLinearVelocity();
			Vec3 angular_velocity = body.GetAngularVelocity();
			float diff_v = (v.mLinearVelocity - linear_velocity).Length();
			float diff_w = (v.mAngularVelocity - angular_velocity).Length();
			if (diff_v > 1.0e-3f || diff_w > 1.0e-3f)
				Trace("Mispredicted collision for body: %08x, v=%s, w=%s, predicted_v=%s, predicted_w=%s, diff_v=%f, diff_w=%f",
					body.GetID().GetIndex(),
					ConvertToString(linear_velocity).c_str(), ConvertToString(angular_velocity).c_str(),
					ConvertToString(v.mLinearVelocity).c_str(), ConvertToString(v.mAngularVelocity).c_str(),
					(double)diff_v,
					(double)diff_w);
		}
	}
	mPredictedVelocities.clear();
}

ValidateResult ContactListenerTest::OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult)
{
	// Body 1 and 2 should never collide
	return ((&inBody1 == mBody[0] && &inBody2 == mBody[1]) || (&inBody1 == mBody[1] && &inBody2 == mBody[0]))? ValidateResult::RejectAllContactsForThisBodyPair : ValidateResult::AcceptAllContactsForThisBodyPair;
}

void ContactListenerTest::MakeBody5PartialSensor(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Make the 2nd shape of body 5 a sensor
	SubShapeID body5_subshape_1 = StaticCast<CompoundShape>(mBody[4]->GetShape())->GetSubShapeIDFromIndex(1, SubShapeIDCreator()).GetID();
	if ((&inBody1 == mBody[4] && inManifold.mSubShapeID1 == body5_subshape_1)
		|| (&inBody2 == mBody[4] && inManifold.mSubShapeID2 == body5_subshape_1))
	{
		Trace("Sensor contact detected between body %08x and body %08x", inBody1.GetID().GetIndexAndSequenceNumber(), inBody2.GetID().GetIndexAndSequenceNumber());
		ioSettings.mIsSensor = true;
	}
}

void ContactListenerTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Make body 1 bounce only when a new contact point is added but not when it is persisted (its restitution is normally 0)
	if (&inBody1 == mBody[0] || &inBody2 == mBody[0])
	{
		JPH_ASSERT(ioSettings.mCombinedRestitution == 0.0f);
		ioSettings.mCombinedRestitution = 1.0f;
	}

	MakeBody5PartialSensor(inBody1, inBody2, inManifold, ioSettings);

	// Estimate the contact impulses.
	CollisionEstimationResult result;
	EstimateCollisionResponse(inBody1, inBody2, inManifold, result, ioSettings.mCombinedFriction, ioSettings.mCombinedRestitution);

	// Trace the result
	String impulses_str;
	for (const CollisionEstimationResult::Impulse &impulse : result.mImpulses)
		impulses_str += StringFormat("(%f, %f, %f) ", (double)impulse.mContactImpulse, (double)impulse.mFrictionImpulse1, (double)impulse.mFrictionImpulse2);

	Trace("Estimated velocity after collision, body1: %08x, v=%s, w=%s, body2: %08x, v=%s, w=%s, impulses: %s",
		inBody1.GetID().GetIndex(), ConvertToString(result.mLinearVelocity1).c_str(), ConvertToString(result.mAngularVelocity1).c_str(),
		inBody2.GetID().GetIndex(), ConvertToString(result.mLinearVelocity2).c_str(), ConvertToString(result.mAngularVelocity2).c_str(),
		impulses_str.c_str());

	// Log predicted velocities
	{
		lock_guard lock(mPredictedVelocitiesMutex);
		mPredictedVelocities.push_back({ inBody1.GetID(), result.mLinearVelocity1, result.mAngularVelocity1 });
		mPredictedVelocities.push_back({ inBody2.GetID(), result.mLinearVelocity2, result.mAngularVelocity2 });
	}
}

void ContactListenerTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	MakeBody5PartialSensor(inBody1, inBody2, inManifold, ioSettings);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SpringTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SpringTest)
{
	JPH_ADD_BASE_CLASS(SpringTest, Test)
}

void SpringTest::Initialize()
{
	// Floor
	CreateFloor();

	// Top fixed body
	RVec3 position(0, 75, 0);
	Body &top = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(100.0f, 1.0f, 1.0f)), position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(top.GetID(), EActivation::DontActivate);

	// Bodies attached with spring with different string lengths, same frequency and no damping
	for (int i = 0; i < 10; ++i)
	{
		// Create body
		RVec3 attachment_point = position + Vec3(-100.0f + i * 5.0f, 0, 0);
		RVec3 body_position = attachment_point - Vec3(0, 10.0f + i * 2.5f, 0);
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.75f)), body_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body.GetMotionProperties()->SetAngularDamping(0.0f);
		body.GetMotionProperties()->SetLinearDamping(0.0f);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);

		// Attach spring
		DistanceConstraintSettings settings;
		settings.mPoint1 = attachment_point;
		settings.mPoint2 = body_position;
		settings.mLimitsSpringSettings.mFrequency = 0.33f;
		mPhysicsSystem->AddConstraint(settings.Create(top, body));

		// Move the body up so that it can start oscillating
		mBodyInterface->SetPositionAndRotation(body.GetID(), attachment_point - Vec3(0, 5, 0), Quat::sIdentity(), EActivation::DontActivate);
	}

	// Bodies attached with spring with different frequency and no damping
	for (int i = 0; i < 10; ++i)
	{
		// Create body
		RVec3 attachment_point = position + Vec3(-25.0f + i * 5.0f, 0, 0);
		RVec3 body_position = attachment_point - Vec3(0, 25.0f, 0);
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.75f)), body_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body.GetMotionProperties()->SetAngularDamping(0.0f);
		body.GetMotionProperties()->SetLinearDamping(0.0f);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);

		// Attach spring
		DistanceConstraintSettings settings;
		settings.mPoint1 = attachment_point;
		settings.mPoint2 = body_position;
		settings.mLimitsSpringSettings.mFrequency = 0.1f + 0.1f * i;
		mPhysicsSystem->AddConstraint(settings.Create(top, body));

		// Move the body up so that it can start oscillating
		mBodyInterface->SetPositionAndRotation(body.GetID(), attachment_point - Vec3(0, 5, 0), Quat::sIdentity(), EActivation::DontActivate);
	}

	// Bodies attached with spring with same spring length, same frequency and different damping
	for (int i = 0; i < 10; ++i)
	{
		// Create body
		RVec3 attachment_point = position + Vec3(50.0f + i * 5.0f, 0, 0);
		RVec3 body_position = attachment_point - Vec3(0, 25.0f, 0);
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.75f)), body_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body.GetMotionProperties()->SetAngularDamping(0.0f);
		body.GetMotionProperties()->SetLinearDamping(0.0f);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);

		// Attach spring
		DistanceConstraintSettings settings;
		settings.mPoint1 = attachment_point;
		settings.mPoint2 = body_position;
		settings.mLimitsSpringSettings.mFrequency = 0.33f;
		settings.mLimitsSpringSettings.mDamping = (1.0f / 9.0f) * i;
		mPhysicsSystem->AddConstraint(settings.Create(top, body));

		// Move the body up so that it can start oscillating
		mBodyInterface->SetPositionAndRotation(body.GetID(), attachment_point - Vec3(0, 5, 0), Quat::sIdentity(), EActivation::DontActivate);
	}
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SwingTwistConstraintFrictionTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SwingTwistConstraintFrictionTest)
{
	JPH_ADD_BASE_CLASS(SwingTwistConstraintFrictionTest, Test)
}

void SwingTwistConstraintFrictionTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	float half_cylinder_height = 1.5f;
	RefConst<Shape> capsule = new CapsuleShape(half_cylinder_height, 0.5f);

	RVec3 body1_position(0, 10, 0);
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(capsule, body1_position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	RVec3 body2_position = body1_position + Vec3(0, -2.0f * half_cylinder_height, 0);
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(capsule, body2_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body2.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	body2.GetMotionProperties()->SetLinearDamping(0.0f);
	body2.GetMotionProperties()->SetAngularDamping(0.0f);
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	SwingTwistConstraintSettings settings;
	settings.mPosition1 = settings.mPosition2 = body1_position + Vec3(0, -half_cylinder_height, 0);
	settings.mTwistAxis1 = settings.mTwistAxis2 = Vec3(0, -1, 0);
	settings.mPlaneAxis1 = settings.mPlaneAxis2 = Vec3::sAxisX();
	settings.mNormalHalfConeAngle = DegreesToRadians(90);
	settings.mPlaneHalfConeAngle = DegreesToRadians(90);
	settings.mTwistMinAngle = -JPH_PI;
	settings.mTwistMaxAngle = JPH_PI;

	float body2_inertia = (body2.GetMotionProperties()->GetLocalSpaceInverseInertia().Inversed3x3() * Vec3::sAxisY()).Length();
	constexpr float max_angular_acceleration = DegreesToRadians(90.0f); // rad/s^2
	settings.mMaxFrictionTorque = body2_inertia * max_angular_acceleration;

	settings.mTwistMotorSettings = settings.mSwingMotorSettings = MotorSettings(10.0f, 2.0f);

	mConstraint = static_cast<SwingTwistConstraint *>(settings.Create(body1, body2));
	mPhysicsSystem->AddConstraint(mConstraint);
}

void SwingTwistConstraintFrictionTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mTime += inParams.mDeltaTime;

	bool pause = fmod(mTime, 5.0f) > 2.5f;

	if (pause)
	{
		mConstraint->SetSwingMotorState(EMotorState::Off);
		mConstraint->SetTwistMotorState(EMotorState::Off);
	}
	else
	{
		mConstraint->SetSwingMotorState(EMotorState::Velocity);
		mConstraint->SetTwistMotorState(EMotorState::Velocity);
		mConstraint->SetTargetAngularVelocityCS(Vec3(DegreesToRadians(180.0f), 0, 0));
	}
}

void SwingTwistConstraintFrictionTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void SwingTwistConstraintFrictionTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}

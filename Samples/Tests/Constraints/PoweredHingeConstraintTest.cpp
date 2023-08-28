// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PoweredHingeConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PoweredHingeConstraintTest)
{
	JPH_ADD_BASE_CLASS(PoweredHingeConstraintTest, Test)
}

void PoweredHingeConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	// Create box
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	RVec3 body1_position(0, 10, 0);
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, body1_position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	RVec3 body2_position = body1_position + Vec3(box_size, 0, 0);
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box, body2_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body2.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	body2.GetMotionProperties()->SetLinearDamping(0.0f);
	body2.GetMotionProperties()->SetAngularDamping(0.0f);
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	RVec3 constraint_position = body1_position + Vec3(0.5f * box_size, 0, 0.5f * box_size);

	HingeConstraintSettings settings;
	settings.mPoint1 = settings.mPoint2 = constraint_position;
	settings.mHingeAxis1 = settings.mHingeAxis2 = Vec3::sAxisY();
	settings.mNormalAxis1 = settings.mNormalAxis2 = Vec3::sAxisX();
	mConstraint = static_cast<HingeConstraint *>(settings.Create(body1, body2));
	mConstraint->SetMotorState(EMotorState::Velocity);
	mConstraint->SetTargetAngularVelocity(DegreesToRadians(25));
	mPhysicsSystem->AddConstraint(mConstraint);

	// Calculate inertia of body 2 as seen from the constraint
	MassProperties body2_inertia_from_constraint;
	body2_inertia_from_constraint.mMass = 1.0f / body2.GetMotionProperties()->GetInverseMass();
	body2_inertia_from_constraint.mInertia = body2.GetMotionProperties()->GetLocalSpaceInverseInertia().Inversed3x3();
	body2_inertia_from_constraint.Translate(Vec3(body2_position - constraint_position));
	mInertiaBody2AsSeenFromConstraint = (body2_inertia_from_constraint.mInertia * Vec3::sAxisY()).Length();
}

void PoweredHingeConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Torque = Inertia * Angular Acceleration (alpha)
	MotorSettings &motor_settings = mConstraint->GetMotorSettings();
	motor_settings.SetTorqueLimit(mInertiaBody2AsSeenFromConstraint * sMaxAngularAcceleration);
	motor_settings.mSpringSettings.mFrequency = sFrequency;
	motor_settings.mSpringSettings.mDamping = sDamping;
	mConstraint->SetMaxFrictionTorque(mInertiaBody2AsSeenFromConstraint * sMaxFrictionAngularAcceleration);
}

void PoweredHingeConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "Motor", { "Off", "Velocity", "Position" }, (int)mConstraint->GetMotorState(), [this](int inItem) { mConstraint->SetMotorState((EMotorState)inItem); });
	inUI->CreateSlider(inSubMenu, "Target Angular Velocity (deg/s)", RadiansToDegrees(mConstraint->GetTargetAngularVelocity()), -90.0f, 90.0f, 1.0f, [this](float inValue) { mConstraint->SetTargetAngularVelocity(DegreesToRadians(inValue)); });
	inUI->CreateSlider(inSubMenu, "Target Angle (deg)", RadiansToDegrees(mConstraint->GetTargetAngle()), -180.0f, 180.0f, 1.0f, [this](float inValue) { mConstraint->SetTargetAngle(DegreesToRadians(inValue)); });
	inUI->CreateSlider(inSubMenu, "Max Angular Acceleration (deg/s^2)", RadiansToDegrees(sMaxAngularAcceleration), 0.0f, 3600.0f, 10.0f, [](float inValue) { sMaxAngularAcceleration = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Frequency (Hz)", sFrequency, 0.0f, 20.0f, 0.1f, [](float inValue) { sFrequency = inValue; });
	inUI->CreateSlider(inSubMenu, "Damping", sDamping, 0.0f, 2.0f, 0.01f, [](float inValue) { sDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Friction Angular Acceleration (deg/s^2)", RadiansToDegrees(sMaxFrictionAngularAcceleration), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxFrictionAngularAcceleration = DegreesToRadians(inValue); });
}

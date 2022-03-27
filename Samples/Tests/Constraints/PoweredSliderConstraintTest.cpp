// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PoweredSliderConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PoweredSliderConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(PoweredSliderConstraintTest, Test) 
}

void PoweredSliderConstraintTest::Initialize()
{
	// Floor
	CreateFloor();
		
	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	// Create box
	float box_size = 4.0f;
	RefConst<Shape> box = new BoxShape(Vec3::sReplicate(0.5f * box_size));

	Vec3 position(0, 10, 0);
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	position += Vec3(box_size + 10.0f, 0, 0);

	mBody2 = mBodyInterface->CreateBody(BodyCreationSettings(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING)));
	mBody2->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBody2->GetMotionProperties()->SetLinearDamping(0.0f);
	mBody2->SetAllowSleeping(false);
	mBodyInterface->AddBody(mBody2->GetID(), EActivation::Activate);

	SliderConstraintSettings settings;
	settings.SetPoint(body1, *mBody2);
	settings.SetSliderAxis(Vec3::sAxisX());
	settings.mLimitsMin = -5.0f;
	settings.mLimitsMax = 100.0f;
	mConstraint = static_cast<SliderConstraint *>(settings.Create(body1, *mBody2));
	mConstraint->SetMotorState(EMotorState::Velocity);
	mConstraint->SetTargetVelocity(1);
	mPhysicsSystem->AddConstraint(mConstraint);
}

void PoweredSliderConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{ 
	MotorSettings &motor_settings = mConstraint->GetMotorSettings();
	motor_settings.SetForceLimit(sMaxMotorAcceleration / mBody2->GetMotionProperties()->GetInverseMass()); // F = m * a
	motor_settings.mFrequency = sFrequency;
	motor_settings.mDamping = sDamping;
	mConstraint->SetMaxFrictionForce(sMaxFrictionAcceleration / mBody2->GetMotionProperties()->GetInverseMass());
}

void PoweredSliderConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "Motor", { "Off", "Velocity", "Position" }, (int)mConstraint->GetMotorState(), [this](int inItem) { mConstraint->SetMotorState((EMotorState)inItem); });
	inUI->CreateSlider(inSubMenu, "Target Velocity (m/s)", mConstraint->GetTargetVelocity(), -10.0f, 10.0f, 0.1f, [this](float inValue) { mConstraint->SetTargetVelocity(inValue); });
	inUI->CreateSlider(inSubMenu, "Target Position (m)", mConstraint->GetTargetPosition(), -5.0f, 20.0f, 0.1f, [this](float inValue) { mConstraint->SetTargetPosition(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Acceleration (m/s^2)", sMaxMotorAcceleration, 0.0f, 250.0f, 1.0f, [](float inValue) { sMaxMotorAcceleration = inValue; });
	inUI->CreateSlider(inSubMenu, "Frequency (Hz)", sFrequency, 0.0f, 20.0f, 0.1f, [](float inValue) { sFrequency = inValue; });
	inUI->CreateSlider(inSubMenu, "Damping", sDamping, 0.0f, 2.0f, 0.01f, [](float inValue) { sDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Friction Acceleration (m/s^2)", sMaxFrictionAcceleration, 0.0f, 10.0f, 0.1f, [](float inValue) { sMaxFrictionAcceleration = inValue; });
}

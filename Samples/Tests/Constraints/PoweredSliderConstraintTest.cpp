// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Constraints/PoweredSliderConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

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

	RVec3 position(0, 10, 0);
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	position += Vec3(box_size + 10.0f, 0, 0);

	mBody2 = mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBody2->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBody2->GetMotionProperties()->SetLinearDamping(0.0f);
	mBody2->SetAllowSleeping(false);
	mBodyInterface->AddBody(mBody2->GetID(), EActivation::Activate);

	SliderConstraintSettings settings;
	settings.mAutoDetectPoint = true;
	settings.SetSliderAxis(Vec3::sAxisX());
	settings.mLimitsMin = -5.0f;
	settings.mLimitsMax = 100.0f;
	mConstraint = static_cast<SliderConstraint *>(settings.Create(body1, *mBody2));
	mPhysicsSystem->AddConstraint(mConstraint);
}

void PoweredSliderConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	MotorSettings &motor_settings = mConstraint->GetMotorSettings();
	motor_settings.SetForceLimit(sMaxMotorAcceleration / mBody2->GetMotionProperties()->GetInverseMass()); // F = m * a
	motor_settings.mSpringSettings.mFrequency = sFrequency;
	motor_settings.mSpringSettings.mDamping = sDamping;
	mConstraint->SetMotorState(sMotorState);
	mConstraint->SetTargetVelocity(sTargetVelocity);
	mConstraint->SetTargetPosition(sTargetPosition);
	mConstraint->SetMaxFrictionForce(sMaxFrictionAcceleration / mBody2->GetMotionProperties()->GetInverseMass());
	switch (sSpringMode)
	{
	case ESpringMode::FrequencyAndDamping:
		motor_settings.mSpringSettings.mMode = ESpringMode::FrequencyAndDamping;
		motor_settings.mSpringSettings.mFrequency = sFrequency;
		motor_settings.mSpringSettings.mDamping = sDampingRatio;
		break;

	case ESpringMode::StiffnessAndDamping:
		motor_settings.mSpringSettings.mMode = ESpringMode::StiffnessAndDamping;
		motor_settings.mSpringSettings.mFrequency = sStiffness / mBody2->GetMotionProperties()->GetInverseMass(); // Make sStiffness mass independent to avoid having to work with really large numbers
		motor_settings.mSpringSettings.mDamping = sDamping / mBody2->GetMotionProperties()->GetInverseMass();
		break;

	case ESpringMode::MassNormalizedStiffnessAndDamping:
		motor_settings.mSpringSettings.mMode = ESpringMode::MassNormalizedStiffnessAndDamping;
		motor_settings.mSpringSettings.mFrequency = sStiffness;
		motor_settings.mSpringSettings.mDamping = sDamping;
		break;
	}

	// Draw state
	float position = mConstraint->GetCurrentPosition();
	float velocity = mConstraint->GetBody2()->GetLinearVelocity().GetX();
	mDebugRenderer->DrawText3D(mConstraint->GetBody2()->GetPosition(), StringFormat("Position: %.1f, Velocity: %.1f m/s", (double)position, (double)velocity), Color::sWhite, 1.0f);
}

void PoweredSliderConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateComboBox(inSubMenu, "Motor", { "Off", "Velocity", "Position", "PositionAndVelocity" }, (int)sMotorState, [](int inItem) { sMotorState = (EMotorState)inItem; });
	inUI->CreateSlider(inSubMenu, "Target Position (m)", sTargetPosition, -5.0f, 20.0f, 0.1f, [](float inValue) { sTargetPosition = inValue; });
	inUI->CreateSlider(inSubMenu, "Target Velocity (m/s)", sTargetVelocity, -10.0f, 10.0f, 0.1f, [](float inValue) { sTargetVelocity = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Acceleration (m/s^2)", sMaxMotorAcceleration, 0.0f, 250.0f, 1.0f, [](float inValue) { sMaxMotorAcceleration = inValue; });
	inUI->CreateComboBox(inSubMenu, "SpringMode", { "FrequencyAndDamping", "StiffnessAndDamping", "NormStiffAndDamp" }, (int)sSpringMode, [](int inItem) { sSpringMode = (ESpringMode)inItem; });
	inUI->CreateSlider(inSubMenu, "Frequency (Hz)", sFrequency, 0.0f, 20.0f, 0.1f, [](float inValue) { sFrequency = inValue; });
	inUI->CreateSlider(inSubMenu, "Damping Ratio", sDampingRatio, 0.0f, 2.0f, 0.01f, [](float inValue) { sDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Stiffness", sStiffness, 0.0f, 200.0f, 0.1f, [](float inValue) { sStiffness = inValue; });
	inUI->CreateSlider(inSubMenu, "Damping", sDamping, 0.0f, 100.0f, 0.1f, [](float inValue) { sDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Friction Acceleration (m/s^2)", sMaxFrictionAcceleration, 0.0f, 10.0f, 0.1f, [](float inValue) { sMaxFrictionAcceleration = inValue; });
}

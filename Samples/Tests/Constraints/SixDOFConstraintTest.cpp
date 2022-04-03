// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/SixDOFConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SixDOFConstraintTest) 
{ 
	JPH_ADD_BASE_CLASS(SixDOFConstraintTest, Test) 
}

float SixDOFConstraintTest::sLimitMin[EAxis::Num] = { 0, 0, 0, 0, 0, 0 };
float SixDOFConstraintTest::sLimitMax[EAxis::Num] = { 0, 0, 0, 0, 0, 0 };
bool SixDOFConstraintTest::sEnableLimits[EAxis::Num] = { true, true, true, true, true, true };

SixDOFConstraintTest::SettingsRef SixDOFConstraintTest::sSettings = []() {
		SixDOFConstraintSettings *settings = new SixDOFConstraintSettings();
		settings->mAxisX1 = settings->mAxisX2 = -Vec3::sAxisY();
		settings->mAxisY1 = settings->mAxisY2 = Vec3::sAxisZ();
		for (int i = 0; i < 6; ++i)
			settings->mMotorSettings[i] = MotorSettings(10.0f, 2.0f);
		return settings;
	}();

void SixDOFConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	// Convert limits to settings class
	for (int i = 0; i < EAxis::Num; ++i)
		if (sEnableLimits[i])
		{
			if (sLimitMax[i] - sLimitMin[i] < 1.0e-3f)
				sSettings->MakeFixedAxis((EAxis)i);
			else
				sSettings->SetLimitedAxis((EAxis)i, sLimitMin[i], sLimitMax[i]);
		}
		else
		{
			sSettings->MakeFreeAxis((EAxis)i);
		}

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	// Create box
	float half_box_height = 1.5f;
	Vec3 position(0, 25, 0);
	RefConst<BoxShape> box = new BoxShape(Vec3(0.5f, half_box_height, 0.25f));

	// Create static body
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, position, Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	// Create dynamic body
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box, position - Vec3(0, 2.0f * half_box_height, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body2.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Set constraint position
	sSettings->mPosition1 = sSettings->mPosition2 = position - Vec3(0, half_box_height, 0);

	// Set force limits
	const float max_acceleration = 1.0f;
	for (int i = 0; i < 3; ++i)
		sSettings->mMotorSettings[i].SetForceLimit(max_acceleration / body2.GetMotionProperties()->GetInverseMass());

	// Create constraint
	mConstraint = static_cast<SixDOFConstraint *>(sSettings->Create(body1, body2));
	mPhysicsSystem->AddConstraint(mConstraint);
}

void SixDOFConstraintTest::GetInitialCamera(CameraState &ioState) const 
{
	ioState.mPos = Vec3(4, 30, 4);
	ioState.mForward = Vec3(-1, -1, -1).Normalized();
}

void SixDOFConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	vector<string> labels = { "Translation X", "Translation Y", "Translation Z", "Rotation X", "Rotation Y", "Rotation Z" };
	vector<string> motor_states = { "Off", "Velocity", "Position" };

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [=]() {
		UIElement *configuration_settings = inUI->CreateMenu();
		
		for (int i = 0; i < 3; ++i)
		{
			inUI->CreateCheckBox(configuration_settings, "Enable Limits " + labels[i], sEnableLimits[i], [=](UICheckBox::EState inState) { sEnableLimits[i] = inState == UICheckBox::STATE_CHECKED; });
			inUI->CreateSlider(configuration_settings, "Limit Min", sLimitMin[i], -10.0f, 0.0f, 0.1f, [=](float inValue) { sLimitMin[i] = inValue; });
			inUI->CreateSlider(configuration_settings, "Limit Max", sLimitMax[i], 0.0f, 10.0f, 0.1f, [=](float inValue) { sLimitMax[i] = inValue; });
		}

		for (int i = 3; i < 6; ++i)
		{
			inUI->CreateCheckBox(configuration_settings, "Enable Limits " + labels[i], sEnableLimits[i], [=](UICheckBox::EState inState) { sEnableLimits[i] = inState == UICheckBox::STATE_CHECKED; });
			if (i == 3)
			{
				inUI->CreateSlider(configuration_settings, "Limit Min", RadiansToDegrees(sLimitMin[i]), -180.0f, 0.0f, 1.0f, [=](float inValue) { sLimitMin[i] = DegreesToRadians(inValue); });
				inUI->CreateSlider(configuration_settings, "Limit Max", RadiansToDegrees(sLimitMax[i]), 0.0f, 180.0f, 1.0f, [=](float inValue) { sLimitMax[i] = DegreesToRadians(inValue); });
			}
			else
			{
				inUI->CreateSlider(configuration_settings, "Limit Max", RadiansToDegrees(sLimitMax[i]), 0.0f, 180.0f, 1.0f, [=](float inValue) { sLimitMin[i] = -DegreesToRadians(inValue); sLimitMax[i] = DegreesToRadians(inValue); });
			}
		}

		for (int i = 0; i < 6; ++i)
			inUI->CreateSlider(configuration_settings, "Max Friction " + labels[i], sSettings->mMaxFriction[i], 0.0f, 500.0f, 1.0f, [=](float inValue) { sSettings->mMaxFriction[i] = inValue; });

		inUI->CreateTextButton(configuration_settings, "Accept Changes", [=]() { RestartTest(); });

		inUI->ShowMenu(configuration_settings);
	});

	inUI->CreateTextButton(inSubMenu, "Runtime Settings", [=]() {
		UIElement *runtime_settings = inUI->CreateMenu();

		for (int i = 0; i < 3; ++i)
		{
			EAxis axis = EAxis(EAxis::TranslationX + i);

			UIComboBox *combo = inUI->CreateComboBox(runtime_settings, "Motor " + labels[i], motor_states, (int)mConstraint->GetMotorState(axis), [=](int inItem) { mConstraint->SetMotorState(axis, (EMotorState)inItem); });
			combo->SetDisabled(sSettings->IsFixedAxis(axis));
	
			UISlider *velocity = inUI->CreateSlider(runtime_settings, "Target Velocity", mConstraint->GetTargetVelocityCS()[i], -10.0f, 10.0f, 0.1f, [=](float inValue) { 
				Vec3 v = mConstraint->GetTargetVelocityCS();
				v.SetComponent(i, inValue);
				mConstraint->SetTargetVelocityCS(v); });
			velocity->SetDisabled(sSettings->IsFixedAxis(axis));

			UISlider *position = inUI->CreateSlider(runtime_settings, "Target Position", mConstraint->GetTargetPositionCS()[i], -10.0f, 10.0f, 0.1f, [=](float inValue) {
				Vec3 v = mConstraint->GetTargetPositionCS();
				v.SetComponent(i, inValue);
				mConstraint->SetTargetPositionCS(v); });
			position->SetDisabled(sSettings->IsFixedAxis(axis));
		}

		for (int i = 0; i < 3; ++i)
		{
			EAxis axis = EAxis(EAxis::RotationX + i);

			inUI->CreateComboBox(runtime_settings, "Motor " + labels[axis], motor_states, (int)mConstraint->GetMotorState(axis), [=](int inItem) { mConstraint->SetMotorState(axis, (EMotorState)inItem); });
	
			inUI->CreateSlider(runtime_settings, "Target Velocity", RadiansToDegrees(mConstraint->GetTargetAngularVelocityCS()[i]), -90.0f, 90.0f, 1.0f, [this, i](float inValue) { 
				Vec3 v = mConstraint->GetTargetAngularVelocityCS();
				v.SetComponent(i, DegreesToRadians(inValue)); 
				mConstraint->SetTargetAngularVelocityCS(v); });

			inUI->CreateSlider(runtime_settings, "Target Position", RadiansToDegrees(mTargetOrientationCS[i]), -180.0f, 180.0f, 1.0f, [this, i](float inValue) { 
				mTargetOrientationCS.SetComponent(i, DegreesToRadians(Clamp(inValue, -179.99f, 179.99f))); // +/- 180 degrees is ambiguous, so add a little bit of a margin
				mConstraint->SetTargetOrientationCS(Quat::sEulerAngles(mTargetOrientationCS)); });
		}

		inUI->ShowMenu(runtime_settings);
	});
}

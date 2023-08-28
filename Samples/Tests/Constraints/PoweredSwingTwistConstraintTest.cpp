// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PoweredSwingTwistConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PoweredSwingTwistConstraintTest)
{
	JPH_ADD_BASE_CLASS(PoweredSwingTwistConstraintTest, Test)
}

Vec3 PoweredSwingTwistConstraintTest::sBodyRotation[] = { Vec3::sZero(), Vec3::sZero() };

void PoweredSwingTwistConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	float half_box_height = 1.5f;
	RefConst<Shape> box = new BoxShape(Vec3(0.25f, half_box_height, 0.5f));
	Quat body1_rotation = Quat::sEulerAngles(sBodyRotation[0]);
	Quat body2_rotation = Quat::sEulerAngles(sBodyRotation[1]) * body1_rotation;

	RVec3 body1_position(0, 20, 0);
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(box, body1_position, body1_rotation, EMotionType::Static, Layers::NON_MOVING));
	body1.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(body1.GetID(), EActivation::DontActivate);

	RVec3 constraint_position = body1_position + body1_rotation * Vec3(0, -half_box_height, 0);

	RVec3 body2_position = constraint_position + body2_rotation * Vec3(0, -half_box_height, 0);
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(box, body2_position, body2_rotation, EMotionType::Dynamic, Layers::MOVING));
	body2.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	body2.GetMotionProperties()->SetLinearDamping(0.0f);
	body2.GetMotionProperties()->SetAngularDamping(0.0f);
	body2.SetAllowSleeping(false);
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	Ref<SwingTwistConstraintSettings> settings = new SwingTwistConstraintSettings;
	settings->mNormalHalfConeAngle = sNormalHalfConeAngle;
	settings->mPlaneHalfConeAngle = sPlaneHalfConeAngle;
	settings->mTwistMinAngle = sTwistMinAngle;
	settings->mTwistMaxAngle = sTwistMaxAngle;

	settings->mPosition1 = settings->mPosition2 = constraint_position;
	settings->mTwistAxis1 = settings->mTwistAxis2 = -body1_rotation.RotateAxisY();
	settings->mPlaneAxis1 = settings->mPlaneAxis2 = body1_rotation.RotateAxisX();

	mConstraint = static_cast<SwingTwistConstraint *>(settings->Create(body1, body2));
	mPhysicsSystem->AddConstraint(mConstraint);

	// Calculate inertia along the axis of the box, so that the acceleration of the motor / friction are correct for twist
	Mat44 body2_inertia = body2.GetMotionProperties()->GetLocalSpaceInverseInertia().Inversed3x3();
	mInertiaBody2AsSeenFromConstraint = (body2_inertia * Vec3::sAxisY()).Length();
}

void PoweredSwingTwistConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Torque = Inertia * Angular Acceleration (alpha)
	mConstraint->SetMaxFrictionTorque(mInertiaBody2AsSeenFromConstraint * sMaxFrictionAngularAcceleration);

	mConstraint->SetNormalHalfConeAngle(sNormalHalfConeAngle);
	mConstraint->SetPlaneHalfConeAngle(sPlaneHalfConeAngle);
	mConstraint->SetTwistMinAngle(sTwistMinAngle);
	mConstraint->SetTwistMaxAngle(sTwistMaxAngle);

	mConstraint->SetSwingMotorState(sSwingMotorState);
	mConstraint->SetTwistMotorState(sTwistMotorState);
	mConstraint->SetTargetAngularVelocityCS(sTargetVelocityCS);
	mConstraint->SetTargetOrientationCS(Quat::sEulerAngles(sTargetOrientationCS));

	MotorSettings &swing = mConstraint->GetSwingMotorSettings();
	swing.SetTorqueLimit(mInertiaBody2AsSeenFromConstraint * sMaxAngularAcceleration);
	swing.mSpringSettings.mFrequency = sFrequency;
	swing.mSpringSettings.mDamping = sDamping;

	MotorSettings &twist = mConstraint->GetTwistMotorSettings();
	twist.SetTorqueLimit(mInertiaBody2AsSeenFromConstraint * sMaxAngularAcceleration);
	twist.mSpringSettings.mFrequency = sFrequency;
	twist.mSpringSettings.mDamping = sDamping;
}

void PoweredSwingTwistConstraintTest::GetInitialCamera(CameraState &ioState) const
{
	ioState.mPos = RVec3(4, 25, 4);
	ioState.mForward = Vec3(-1, -1, -1).Normalized();
}

void PoweredSwingTwistConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	Array<String> axis_label = { "X", "Y", "Z" };
	Array<String> constraint_label = { "Twist", "Plane", "Normal" };

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [=]() {
		UIElement *configuration_settings = inUI->CreateMenu();

		for (int body = 0; body < 2; ++body)
			for (int axis = 0; axis < 3; ++axis)
				inUI->CreateSlider(configuration_settings, "Body " + ConvertToString(body + 1) + " Rotation " + axis_label[axis] + " (deg)", RadiansToDegrees(sBodyRotation[body][axis]), -180.0f, 180.0f, 1.0f, [=](float inValue) { sBodyRotation[body].SetComponent(axis, DegreesToRadians(inValue)); });

		inUI->CreateTextButton(configuration_settings, "Accept Changes", [=]() { RestartTest(); });

		inUI->ShowMenu(configuration_settings);
	});

	inUI->CreateTextButton(inSubMenu, "Runtime Settings", [=]() {
		UIElement *runtime_settings = inUI->CreateMenu();

		inUI->CreateSlider(runtime_settings, "Min Twist Angle (deg)", RadiansToDegrees(sTwistMinAngle), -180.0f, 0.0f, 1.0f, [=](float inValue) { sTwistMinAngle = DegreesToRadians(inValue); });
		inUI->CreateSlider(runtime_settings, "Max Twist Angle (deg)", RadiansToDegrees(sTwistMaxAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sTwistMaxAngle = DegreesToRadians(inValue); });
		inUI->CreateSlider(runtime_settings, "Normal Half Cone Angle (deg)", RadiansToDegrees(sNormalHalfConeAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sNormalHalfConeAngle = DegreesToRadians(inValue); });
		inUI->CreateSlider(runtime_settings, "Plane Half Cone Angle (deg)", RadiansToDegrees(sPlaneHalfConeAngle), 0.0f, 180.0f, 1.0f, [=](float inValue) { sPlaneHalfConeAngle = DegreesToRadians(inValue); });

		inUI->CreateComboBox(runtime_settings, "Swing Motor", { "Off", "Velocity", "Position" }, (int)sSwingMotorState, [=](int inItem) { sSwingMotorState = (EMotorState)inItem; });
		inUI->CreateComboBox(runtime_settings, "Twist Motor", { "Off", "Velocity", "Position" }, (int)sTwistMotorState, [=](int inItem) { sTwistMotorState = (EMotorState)inItem; });

		for (int i = 0; i < 3; ++i)
			inUI->CreateSlider(runtime_settings, "Target Angular Velocity " + constraint_label[i] + " (deg/s)", RadiansToDegrees(sTargetVelocityCS[i]), -90.0f, 90.0f, 1.0f, [i](float inValue) { sTargetVelocityCS.SetComponent(i, DegreesToRadians(inValue)); });

		for (int i = 0; i < 3; ++i)
			inUI->CreateSlider(runtime_settings, "Target Angle " + constraint_label[i] + " (deg)", RadiansToDegrees(sTargetOrientationCS[i]), -180, 180.0f, 1.0f, [i](float inValue) {
				sTargetOrientationCS.SetComponent(i, DegreesToRadians(Clamp(inValue, -179.99f, 179.99f))); // +/- 180 degrees is ambiguous, so add a little bit of a margin
			});

		inUI->CreateSlider(runtime_settings, "Max Angular Acceleration (deg/s^2)", RadiansToDegrees(sMaxAngularAcceleration), 0.0f, 36000.0f, 100.0f, [=](float inValue) { sMaxAngularAcceleration = DegreesToRadians(inValue); });
		inUI->CreateSlider(runtime_settings, "Frequency (Hz)", sFrequency, 0.0f, 20.0f, 0.1f, [=](float inValue) { sFrequency = inValue; });
		inUI->CreateSlider(runtime_settings, "Damping", sDamping, 0.0f, 2.0f, 0.01f, [=](float inValue) { sDamping = inValue; });
		inUI->CreateSlider(runtime_settings, "Max Friction Angular Acceleration (deg/s^2)", RadiansToDegrees(sMaxFrictionAngularAcceleration), 0.0f, 900.0f, 1.0f, [=](float inValue) { sMaxFrictionAngularAcceleration = DegreesToRadians(inValue); });

		inUI->ShowMenu(runtime_settings);
	});
}

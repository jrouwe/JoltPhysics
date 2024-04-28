// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/PathConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/PathConstraintPathHermite.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PathConstraintTest)
{
	JPH_ADD_BASE_CLASS(PathConstraintTest, Test)
}

EPathRotationConstraintType PathConstraintTest::sOrientationType = EPathRotationConstraintType::Free;

void PathConstraintTest::Initialize()
{
	// Floor
	CreateFloor();

	{
		// Create spiral path
		Ref<PathConstraintPathHermite> path = new PathConstraintPathHermite;
		Vec3 normal(0, 1, 0);
		Array<Vec3> positions;
		for (float a = -0.1f * JPH_PI; a < 4.0f * JPH_PI; a += 0.1f * JPH_PI)
			positions.push_back(Vec3(5.0f * Cos(a), -a, 5.0f * Sin(a)));
		for (int i = 1; i < int(positions.size() - 1); ++i)
		{
			Vec3 tangent = 0.5f * (positions[i + 1] - positions[i - 1]);
			path->AddPoint(positions[i], tangent, normal);
		}
		mPaths[0] = path;

		// Dynamic base plate to which the path attaches
		Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(5, 0.5f, 5)), RVec3(-10, 1, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

		// Dynamic body attached to the path
		Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 1.0f, 2.0f)), RVec3(-5, 15, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body2.SetAllowSleeping(false);
		mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

		// Create path constraint
		PathConstraintSettings settings;
		settings.mPath = path;
		settings.mPathPosition = Vec3(0, 15, 0);
		settings.mRotationConstraintType = sOrientationType;
		mConstraints[0] = static_cast<PathConstraint *>(settings.Create(body1, body2));
		mPhysicsSystem->AddConstraint(mConstraints[0]);
	}

	{
		// Create circular path
		Ref<PathConstraintPathHermite> path = new PathConstraintPathHermite;
		path->SetIsLooping(true);
		Vec3 normal(0, 1, 0);
		Array<Vec3> positions;
		for (int i = -1; i < 11; ++i)
		{
			float a = 2.0f * JPH_PI * i / 10.0f;
			positions.push_back(Vec3(5.0f * Cos(a), 0.0f, 5.0f * Sin(a)));
		}
		for (int i = 1; i < int(positions.size() - 1); ++i)
		{
			Vec3 tangent = 0.5f * (positions[i + 1] - positions[i - 1]);
			path->AddPoint(positions[i], tangent, normal);
		}
		mPaths[1] = path;

		// Dynamic base plate to which the path attaches
		Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(5, 0.5f, 5)), RVec3(10, 1, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

		// Dynamic body attached to the path
		Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.5f, 1.0f, 2.0f)), RVec3(15, 5, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		body2.SetAllowSleeping(false);
		mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

		// Create path constraint
		PathConstraintSettings settings;
		settings.mPath = path;
		settings.mPathPosition = Vec3(0, 5, 0);
		settings.mPathRotation = Quat::sRotation(Vec3::sAxisX(), -0.1f * JPH_PI);
		settings.mRotationConstraintType = sOrientationType;
		mConstraints[1] = static_cast<PathConstraint *>(settings.Create(body1, body2));
		mPhysicsSystem->AddConstraint(mConstraints[1]);
	}

}

void PathConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	for (PathConstraint *c : mConstraints)
	{
		MotorSettings &motor_settings = c->GetPositionMotorSettings();
		motor_settings.SetForceLimit(sMaxMotorAcceleration / c->GetBody2()->GetMotionProperties()->GetInverseMass()); // F = m * a
		motor_settings.mSpringSettings.mFrequency = sFrequency;
		motor_settings.mSpringSettings.mDamping = sDamping;
		c->SetMaxFrictionForce(sMaxFrictionAcceleration / c->GetBody2()->GetMotionProperties()->GetInverseMass());
	}
}

void PathConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	static Array<String> constraint_types = { "Free", "Tangent", "Normal", "Binormal", "Path", "Full" };

	inUI->CreateTextButton(inSubMenu, "Configuration Settings", [this, inUI]() {
		UIElement *configuration_settings = inUI->CreateMenu();
		inUI->CreateComboBox(configuration_settings, "Rotation Constraint", constraint_types, (int)sOrientationType, [=](int inItem) { sOrientationType = (EPathRotationConstraintType)inItem; });
		inUI->CreateTextButton(configuration_settings, "Accept Changes", [this]() { RestartTest(); });
		inUI->ShowMenu(configuration_settings);
	});

	inUI->CreateTextButton(inSubMenu, "Runtime Settings", [this, inUI]() {
		UIElement *runtime_settings = inUI->CreateMenu();
		inUI->CreateComboBox(runtime_settings, "Motor", { "Off", "Velocity", "Position" }, (int)mConstraints[0]->GetPositionMotorState(), [this](int inItem) { for (PathConstraint *c : mConstraints) c->SetPositionMotorState((EMotorState)inItem); });
		inUI->CreateSlider(runtime_settings, "Target Velocity (m/s)", mConstraints[0]->GetTargetVelocity(), -10.0f, 10.0f, 0.1f, [this](float inValue) { for (PathConstraint *c : mConstraints) c->SetTargetVelocity(inValue); });
		inUI->CreateSlider(runtime_settings, "Target Path Fraction", mConstraints[0]->GetTargetPathFraction(), 0, mPaths[0]->GetPathMaxFraction(), 0.1f, [this](float inValue) { for (PathConstraint *c : mConstraints) c->SetTargetPathFraction(inValue); });
		inUI->CreateSlider(runtime_settings, "Max Acceleration (m/s^2)", sMaxMotorAcceleration, 0.0f, 100.0f, 1.0f, [](float inValue) { sMaxMotorAcceleration = inValue; });
		inUI->CreateSlider(runtime_settings, "Frequency (Hz)", sFrequency, 0.0f, 20.0f, 0.1f, [](float inValue) { sFrequency = inValue; });
		inUI->CreateSlider(runtime_settings, "Damping", sDamping, 0.0f, 2.0f, 0.01f, [](float inValue) { sDamping = inValue; });
		inUI->CreateSlider(runtime_settings, "Max Friction Acceleration (m/s^2)", sMaxFrictionAcceleration, 0.0f, 10.0f, 0.1f, [](float inValue) { sMaxFrictionAcceleration = inValue; });
		inUI->ShowMenu(runtime_settings);
	});

	inUI->ShowMenu(inSubMenu);
}

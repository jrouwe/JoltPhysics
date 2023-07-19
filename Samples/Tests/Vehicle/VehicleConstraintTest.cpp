// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/VehicleConstraintTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleConstraintTest)
{
	JPH_ADD_BASE_CLASS(VehicleConstraintTest, VehicleTest)
}

VehicleConstraintTest::~VehicleConstraintTest()
{
	mPhysicsSystem->RemoveStepListener(mVehicleConstraint);
}

void VehicleConstraintTest::Initialize()
{
	VehicleTest::Initialize();

	const float wheel_radius = 0.3f;
	const float wheel_width = 0.1f;
	const float half_vehicle_length = 2.0f;
	const float half_vehicle_width = 0.9f;
	const float half_vehicle_height = 0.2f;

	// Create collision testers
	mTesters[0] = new VehicleCollisionTesterRay(Layers::MOVING);
	mTesters[1] = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * wheel_width);
	mTesters[2] = new VehicleCollisionTesterCastCylinder(Layers::MOVING);

	// Create vehicle body
	RVec3 position(0, 2, 0);
	RefConst<Shape> car_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
	BodyCreationSettings car_body_settings(car_shape, position, Quat::sRotation(Vec3::sAxisZ(), sInitialRollAngle), EMotionType::Dynamic, Layers::MOVING);
	car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;
	mCarBody = mBodyInterface->CreateBody(car_body_settings);
	mBodyInterface->AddBody(mCarBody->GetID(), EActivation::Activate);

	// Create vehicle constraint
	VehicleConstraintSettings vehicle;
	vehicle.mDrawConstraintSize = 0.1f;
	vehicle.mMaxPitchRollAngle = sMaxRollAngle;

	// Suspension direction
	Vec3 front_suspension_dir = Vec3(Tan(sFrontSuspensionSidewaysAngle), -1, Tan(sFrontSuspensionForwardAngle)).Normalized();
	Vec3 front_steering_axis = Vec3(-Tan(sFrontKingPinAngle), 1, -Tan(sFrontCasterAngle)).Normalized();
	Vec3 front_wheel_up = Vec3(Sin(sFrontCamber), Cos(sFrontCamber), 0);
	Vec3 front_wheel_forward = Vec3(-Sin(sFrontToe), 0, Cos(sFrontToe));
	Vec3 rear_suspension_dir = Vec3(Tan(sRearSuspensionSidewaysAngle), -1, Tan(sRearSuspensionForwardAngle)).Normalized();
	Vec3 rear_steering_axis = Vec3(-Tan(sRearKingPinAngle), 1, -Tan(sRearCasterAngle)).Normalized();
	Vec3 rear_wheel_up = Vec3(Sin(sRearCamber), Cos(sRearCamber), 0);
	Vec3 rear_wheel_forward = Vec3(-Sin(sRearToe), 0, Cos(sRearToe));
	Vec3 flip_x(-1, 1, 1);

	// Wheels, left front
	WheelSettingsWV *w1 = new WheelSettingsWV;
	w1->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w1->mSuspensionDirection = front_suspension_dir;
	w1->mSteeringAxis = front_steering_axis;
	w1->mWheelUp = front_wheel_up;
	w1->mWheelForward = front_wheel_forward;
	w1->mSuspensionMinLength = sFrontSuspensionMinLength;
	w1->mSuspensionMaxLength = sFrontSuspensionMaxLength;
	w1->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
	w1->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
	w1->mMaxSteerAngle = sMaxSteeringAngle;
	w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	// Right front
	WheelSettingsWV *w2 = new WheelSettingsWV;
	w2->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w2->mSuspensionDirection = flip_x * front_suspension_dir;
	w2->mSteeringAxis = flip_x * front_steering_axis;
	w2->mWheelUp = flip_x * front_wheel_up;
	w2->mWheelForward = flip_x * front_wheel_forward;
	w2->mSuspensionMinLength = sFrontSuspensionMinLength;
	w2->mSuspensionMaxLength = sFrontSuspensionMaxLength;
	w2->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
	w2->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
	w2->mMaxSteerAngle = sMaxSteeringAngle;
	w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	// Left rear
	WheelSettingsWV *w3 = new WheelSettingsWV;
	w3->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w3->mSuspensionDirection = rear_suspension_dir;
	w3->mSteeringAxis = rear_steering_axis;
	w3->mWheelUp = rear_wheel_up;
	w3->mWheelForward = rear_wheel_forward;
	w3->mSuspensionMinLength = sRearSuspensionMinLength;
	w3->mSuspensionMaxLength = sRearSuspensionMaxLength;
	w3->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
	w3->mSuspensionSpring.mDamping = sRearSuspensionDamping;
	w3->mMaxSteerAngle = 0.0f;

	// Right rear
	WheelSettingsWV *w4 = new WheelSettingsWV;
	w4->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w4->mSuspensionDirection = flip_x * rear_suspension_dir;
	w4->mSteeringAxis = flip_x * rear_steering_axis;
	w4->mWheelUp = flip_x * rear_wheel_up;
	w4->mWheelForward = flip_x * rear_wheel_forward;
	w4->mSuspensionMinLength = sRearSuspensionMinLength;
	w4->mSuspensionMaxLength = sRearSuspensionMaxLength;
	w4->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
	w4->mSuspensionSpring.mDamping = sRearSuspensionDamping;
	w4->mMaxSteerAngle = 0.0f;

	vehicle.mWheels = { w1, w2, w3, w4 };

	for (WheelSettings *w : vehicle.mWheels)
	{
		w->mRadius = wheel_radius;
		w->mWidth = wheel_width;
	}

	WheeledVehicleControllerSettings *controller = new WheeledVehicleControllerSettings;
	vehicle.mController = controller;

	// Differential
	controller->mDifferentials.resize(sFourWheelDrive? 2 : 1);
	controller->mDifferentials[0].mLeftWheel = 0;
	controller->mDifferentials[0].mRightWheel = 1;
	if (sFourWheelDrive)
	{
		controller->mDifferentials[1].mLeftWheel = 2;
		controller->mDifferentials[1].mRightWheel = 3;

		// Split engine torque
		controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
	}

	// Anti rollbars
	if (sAntiRollbar)
	{
		vehicle.mAntiRollBars.resize(2);
		vehicle.mAntiRollBars[0].mLeftWheel = 0;
		vehicle.mAntiRollBars[0].mRightWheel = 1;
		vehicle.mAntiRollBars[1].mLeftWheel = 2;
		vehicle.mAntiRollBars[1].mRightWheel = 3;
	}

	mVehicleConstraint = new VehicleConstraint(*mCarBody, vehicle);
	mPhysicsSystem->AddConstraint(mVehicleConstraint);
	mPhysicsSystem->AddStepListener(mVehicleConstraint);
}

void VehicleConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	VehicleTest::PrePhysicsUpdate(inParams);

	// Determine acceleration and brake
	float forward = 0.0f, right = 0.0f, brake = 0.0f, hand_brake = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))
		forward = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))
		forward = -1.0f;

	// Check if we're reversing direction
	if (mPreviousForward * forward < 0.0f)
	{
		// Get vehicle velocity in local space to the body of the vehicle
		float velocity = (mCarBody->GetRotation().Conjugated() * mCarBody->GetLinearVelocity()).GetZ();
		if ((forward > 0.0f && velocity < -0.1f) || (forward < 0.0f && velocity > 0.1f))
		{
			// Brake while we've not stopped yet
			forward = 0.0f;
			brake = 1.0f;
		}
		else
		{
			// When we've come to a stop, accept the new direction
			mPreviousForward = forward;
		}
	}

	// Hand brake will cancel gas pedal
	if (inParams.mKeyboard->IsKeyPressed(DIK_Z))
	{
		forward = 0.0f;
		hand_brake = 1.0f;
	}

	// Steering
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))
		right = -1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))
		right = 1.0f;

	// On user input, assure that the car is active
	if (right != 0.0f || forward != 0.0f || brake != 0.0f || hand_brake != 0.0f)
		mBodyInterface->ActivateBody(mCarBody->GetID());

	WheeledVehicleController *controller = static_cast<WheeledVehicleController *>(mVehicleConstraint->GetController());

	// Update vehicle statistics
	controller->GetEngine().mMaxTorque = sMaxEngineTorque;
	controller->GetTransmission().mClutchStrength = sClutchStrength;

	// Set slip ratios to the same for everything
	float limited_slip_ratio = sLimitedSlipDifferentials? 1.4f : FLT_MAX;
	controller->SetDifferentialLimitedSlipRatio(limited_slip_ratio);
	for (VehicleDifferentialSettings &d : controller->GetDifferentials())
		d.mLimitedSlipRatio = limited_slip_ratio;

	// Pass the input on to the constraint
	controller->SetDriverInput(forward, right, brake, hand_brake);

	// Set the collision tester
	mVehicleConstraint->SetVehicleCollisionTester(mTesters[sCollisionMode]);

	// Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
	for (uint w = 0; w < 4; ++w)
	{
		const WheelSettings *settings = mVehicleConstraint->GetWheels()[w]->GetSettings();
		RMat44 wheel_transform = mVehicleConstraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cyclinder we draw is aligned with Y so we specify that as rotational axis
		mDebugRenderer->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);
	}
}

void VehicleConstraintTest::SaveState(StateRecorder& inStream) const
{
	VehicleTest::SaveState(inStream);

	inStream.Write(mPreviousForward);
}

void VehicleConstraintTest::RestoreState(StateRecorder& inStream)
{
	VehicleTest::RestoreState(inStream);

	inStream.Read(mPreviousForward);
}

void VehicleConstraintTest::GetInitialCamera(CameraState &ioState) const
{
	// Position camera behind car
	RVec3 cam_tgt = RVec3(0, 0, 5);
	ioState.mPos = RVec3(0, 2.5f, -5);
	ioState.mForward = Vec3(cam_tgt - ioState.mPos).Normalized();
}

RMat44 VehicleConstraintTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	// Pivot is center of car and rotates with car around Y axis only
	Vec3 fwd = mCarBody->GetRotation().RotateAxisZ();
	fwd.SetY(0.0f);
	float len = fwd.Length();
	if (len != 0.0f)
		fwd /= len;
	else
		fwd = Vec3::sAxisZ();
	Vec3 up = Vec3::sAxisY();
	Vec3 right = up.Cross(fwd);
	return RMat44(Vec4(right, 0), Vec4(up, 0), Vec4(fwd, 0), mCarBody->GetPosition());
}

void VehicleConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	VehicleTest::CreateSettingsMenu(inUI, inSubMenu);

	inUI->CreateSlider(inSubMenu, "Initial Roll Angle", RadiansToDegrees(sInitialRollAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sInitialRollAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Roll Angle", RadiansToDegrees(sMaxRollAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxRollAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Max Steering Angle", RadiansToDegrees(sMaxSteeringAngle), 0.0f, 90.0f, 1.0f, [](float inValue) { sMaxSteeringAngle = DegreesToRadians(inValue); });
	inUI->CreateComboBox(inSubMenu, "Collision Mode", { "Ray", "Cast Sphere", "Cast Cylinder" }, sCollisionMode, [](int inItem) { sCollisionMode = inItem; });
	inUI->CreateCheckBox(inSubMenu, "4 Wheel Drive", sFourWheelDrive, [](UICheckBox::EState inState) { sFourWheelDrive = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Anti Rollbars", sAntiRollbar, [](UICheckBox::EState inState) { sAntiRollbar = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Limited Slip Differentials", sLimitedSlipDifferentials, [](UICheckBox::EState inState) { sLimitedSlipDifferentials = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateSlider(inSubMenu, "Max Engine Torque", float(sMaxEngineTorque), 100.0f, 2000.0f, 10.0f, [](float inValue) { sMaxEngineTorque = inValue; });
	inUI->CreateSlider(inSubMenu, "Clutch Strength", float(sClutchStrength), 1.0f, 40.0f, 1.0f, [](float inValue) { sClutchStrength = inValue; });
	inUI->CreateSlider(inSubMenu, "Front Caster Angle", RadiansToDegrees(sFrontCasterAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontCasterAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front King Pin Angle", RadiansToDegrees(sFrontKingPinAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontKingPinAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front Camber", RadiansToDegrees(sFrontCamber), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontCamber = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front Toe", RadiansToDegrees(sFrontToe), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontToe = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front Suspension Forward Angle", RadiansToDegrees(sFrontSuspensionForwardAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontSuspensionForwardAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front Suspension Sideways Angle", RadiansToDegrees(sFrontSuspensionSidewaysAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sFrontSuspensionSidewaysAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Front Suspension Min Length", sFrontSuspensionMinLength, 0.0f, 3.0f, 0.01f, [](float inValue) { sFrontSuspensionMinLength = inValue; });
	inUI->CreateSlider(inSubMenu, "Front Suspension Max Length", sFrontSuspensionMaxLength, 0.0f, 3.0f, 0.01f, [](float inValue) { sFrontSuspensionMaxLength = inValue; });
	inUI->CreateSlider(inSubMenu, "Front Suspension Frequency", sFrontSuspensionFrequency, 0.1f, 5.0f, 0.01f, [](float inValue) { sFrontSuspensionFrequency = inValue; });
	inUI->CreateSlider(inSubMenu, "Front Suspension Damping", sFrontSuspensionDamping, 0.0f, 2.0f, 0.01f, [](float inValue) { sFrontSuspensionDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Rear Caster Angle", RadiansToDegrees(sRearCasterAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearCasterAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear King Pin Angle", RadiansToDegrees(sRearKingPinAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearKingPinAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear Camber", RadiansToDegrees(sRearCamber), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearCamber = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear Toe", RadiansToDegrees(sRearToe), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearToe = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Forward Angle", RadiansToDegrees(sRearSuspensionForwardAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearSuspensionForwardAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Sideways Angle", RadiansToDegrees(sRearSuspensionSidewaysAngle), -89.0f, 89.0f, 1.0f, [](float inValue) { sRearSuspensionSidewaysAngle = DegreesToRadians(inValue); });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Min Length", sRearSuspensionMinLength, 0.0f, 3.0f, 0.01f, [](float inValue) { sRearSuspensionMinLength = inValue; });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Max Length", sRearSuspensionMaxLength, 0.0f, 3.0f, 0.01f, [](float inValue) { sRearSuspensionMaxLength = inValue; });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Frequency", sRearSuspensionFrequency, 0.1f, 5.0f, 0.01f, [](float inValue) { sRearSuspensionFrequency = inValue; });
	inUI->CreateSlider(inSubMenu, "Rear Suspension Damping", sRearSuspensionDamping, 0.0f, 2.0f, 0.01f, [](float inValue) { sRearSuspensionDamping = inValue; });
	inUI->CreateTextButton(inSubMenu, "Accept", [this]() { RestartTest(); });
}

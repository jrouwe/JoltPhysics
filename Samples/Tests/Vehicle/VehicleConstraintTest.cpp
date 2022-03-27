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

int VehicleConstraintTest::sCollisionMode = 1;

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
	const float suspension_min_length = 0.3f;
	const float suspension_max_length = 0.5f;
	const float max_steering_angle = DegreesToRadians(30);

	// Create collision testers
	mTesters[0] = new VehicleCollisionTesterRay(Layers::MOVING);
	mTesters[1] = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * wheel_width);

	// Create vehicle body
	Vec3 position(0, 2, 0);
	RefConst<Shape> car_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
	BodyCreationSettings car_body_settings(car_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;
	mCarBody = mBodyInterface->CreateBody(car_body_settings);
	mBodyInterface->AddBody(mCarBody->GetID(), EActivation::Activate);

	// Create vehicle constraint
	VehicleConstraintSettings vehicle;
	vehicle.mDrawConstraintSize = 0.1f;
	vehicle.mMaxPitchRollAngle = DegreesToRadians(60.0f);

	// Wheels
	WheelSettingsWV *w1 = new WheelSettingsWV;
	w1->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w1->mMaxSteerAngle = max_steering_angle;
	w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	WheelSettingsWV *w2 = new WheelSettingsWV;
	w2->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w2->mMaxSteerAngle = max_steering_angle;
	w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	WheelSettingsWV *w3 = new WheelSettingsWV;
	w3->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w3->mMaxSteerAngle = 0.0f;

	WheelSettingsWV *w4 = new WheelSettingsWV;
	w4->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w4->mMaxSteerAngle = 0.0f;

	vehicle.mWheels = { w1, w2, w3, w4 };
	
	for (WheelSettings *w : vehicle.mWheels)
	{
		w->mRadius = wheel_radius;
		w->mWidth = wheel_width;
		w->mSuspensionMinLength = suspension_min_length;
		w->mSuspensionMaxLength = suspension_max_length;
	}

	WheeledVehicleControllerSettings *controller = new WheeledVehicleControllerSettings;
	vehicle.mController = controller;

	// Differential
	controller->mDifferentials.resize(1);
	controller->mDifferentials[0].mLeftWheel = 0;
	controller->mDifferentials[0].mRightWheel = 1;

	// Anti rollbars
	vehicle.mAntiRollBars.resize(2);
	vehicle.mAntiRollBars[0].mLeftWheel = 0;
	vehicle.mAntiRollBars[0].mRightWheel = 1;
	vehicle.mAntiRollBars[1].mLeftWheel = 2;
	vehicle.mAntiRollBars[1].mRightWheel = 3;

	mVehicleConstraint = new VehicleConstraint(*mCarBody, vehicle);
	mPhysicsSystem->AddConstraint(mVehicleConstraint);
	mPhysicsSystem->AddStepListener(mVehicleConstraint);
}

void VehicleConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
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

	// Pass the input on to the constraint
	static_cast<WheeledVehicleController *>(mVehicleConstraint->GetController())->SetDriverInput(forward, right, brake, hand_brake);

	// Set the collision tester
	mVehicleConstraint->SetVehicleCollisionTester(mTesters[sCollisionMode]);

	// Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
	for (uint w = 0; w < 4; ++w)
	{
		const WheelSettings *settings = mVehicleConstraint->GetWheels()[w]->GetSettings();
		Mat44 wheel_transform = mVehicleConstraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cyclinder we draw is aligned with Y so we specify that as rotational axis
		mDebugRenderer->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);
	}
}

void VehicleConstraintTest::GetInitialCamera(CameraState &ioState) const 
{
	// Position camera behind car
	Vec3 cam_tgt = Vec3(0, 0, 5);
	ioState.mPos = Vec3(0, 2.5f, -5);
	ioState.mForward = (cam_tgt - ioState.mPos).Normalized();
}

Mat44 VehicleConstraintTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
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
	return Mat44(Vec4(right, 0), Vec4(up, 0), Vec4(fwd, 0), Vec4(mCarBody->GetPosition(), 1.0f));
}

void VehicleConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	VehicleTest::CreateSettingsMenu(inUI, inSubMenu);

	inUI->CreateComboBox(inSubMenu, "Collision Mode", { "Ray", "Cast Sphere" }, sCollisionMode, [](int inItem) { sCollisionMode = inItem; });
}

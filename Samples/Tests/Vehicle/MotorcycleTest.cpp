// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/MotorcycleTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(MotorcycleTest) 
{ 
	JPH_ADD_BASE_CLASS(MotorcycleTest, VehicleTest) 
}

MotorcycleTest::~MotorcycleTest()
{
	mPhysicsSystem->RemoveStepListener(mVehicleConstraint);
}

void MotorcycleTest::Initialize()
{
	VehicleTest::Initialize();

	// Loosly based on: https://www.whitedogbikes.com/whitedogblog/yamaha-xj-900-specs/
	const float back_wheel_radius = 0.31f;
	const float back_wheel_width = 0.05f;
	const float back_wheel_pos_z = -0.75f;
	const float back_suspension_min_length = 0.3f;
	const float back_suspension_max_length = 0.5f;
	const float back_suspension_freq = 2.0f;
	const float back_brake_torque = 250.0f;

	const float front_wheel_radius = 0.31f;
	const float front_wheel_width = 0.05f;
	const float front_wheel_pos_z = 0.75f;
	const float front_suspension_min_length = 0.3f;
	const float front_suspension_max_length = 0.5f;
	const float front_suspension_freq = 1.5f;
	const float front_brake_torque = 500.0f;

	const float half_vehicle_length = 0.4f;
	const float half_vehicle_width = 0.2f;
	const float half_vehicle_height = 0.3f;

	const float max_steering_angle = DegreesToRadians(30);

	// Angle of the front suspension
	const float caster_angle = DegreesToRadians(30);

	// Create vehicle body
	RVec3 position(0, 2, 0);
	RefConst<Shape> motorcycle_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
	BodyCreationSettings motorcycle_body_settings(motorcycle_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	motorcycle_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	motorcycle_body_settings.mMassPropertiesOverride.mMass = 240.0f;
	mMotorcycleBody = mBodyInterface->CreateBody(motorcycle_body_settings);
	mBodyInterface->AddBody(mMotorcycleBody->GetID(), EActivation::Activate);

	// Create vehicle constraint
	VehicleConstraintSettings vehicle;
	vehicle.mDrawConstraintSize = 0.1f;
	vehicle.mMaxPitchRollAngle = DegreesToRadians(60.0f);

	// Wheels
	WheelSettingsWV *front = new WheelSettingsWV;
	front->mPosition = Vec3(0.0f, -0.9f * half_vehicle_height, front_wheel_pos_z);
	front->mMaxSteerAngle = max_steering_angle;
	front->mSuspensionDirection = Vec3(0, -1, Tan(caster_angle)).Normalized();
	front->mSteeringAxis = -front->mSuspensionDirection;
	front->mRadius = front_wheel_radius;
	front->mWidth = front_wheel_width;
	front->mSuspensionMinLength = front_suspension_min_length;
	front->mSuspensionMaxLength = front_suspension_max_length;
	front->mSuspensionFrequency = front_suspension_freq;
	front->mMaxBrakeTorque = front_brake_torque;

	WheelSettingsWV *back = new WheelSettingsWV;
	back->mPosition = Vec3(0.0f, -0.9f * half_vehicle_height, back_wheel_pos_z);
	back->mMaxSteerAngle = 0.0f;
	back->mRadius = back_wheel_radius;
	back->mWidth = back_wheel_width;
	back->mSuspensionMinLength = back_suspension_min_length;
	back->mSuspensionMaxLength = back_suspension_max_length;
	back->mSuspensionFrequency = back_suspension_freq;
	back->mMaxBrakeTorque = back_brake_torque;

	vehicle.mWheels = { front, back };

	MotorcycleControllerSettings *controller = new MotorcycleControllerSettings;
	controller->mEngine.mMaxTorque = 80.0f;
	controller->mEngine.mMinRPM = 1000.0f;
	controller->mEngine.mMaxRPM = 10000.0f;
	controller->mTransmission.mShiftDownRPM = 2000.0f;
	controller->mTransmission.mShiftUpRPM = 9000.0f;
	controller->mTransmission.mGearRatios = { 2.27f, 1.63f, 1.3f, 1.09f, 0.96f, 0.88f }; // From: https://www.blocklayer.com/rpm-gear-bikes
	controller->mTransmission.mReverseGearRatios = { -4.0f };
	vehicle.mController = controller;

	// Differential (not really applicable to a motorcycle but we need one anyway to drive it)
	controller->mDifferentials.resize(1);
	controller->mDifferentials[0].mLeftWheel = -1;
	controller->mDifferentials[0].mRightWheel = 1;
	controller->mDifferentials[0].mDifferentialRatio = 1.93f * 40.0f / 16.0f; // Combining primary and final drive (back divided by front sprockets) from: https://www.blocklayer.com/rpm-gear-bikes

	mVehicleConstraint = new VehicleConstraint(*mMotorcycleBody, vehicle);
	mVehicleConstraint->SetVehicleCollisionTester(new VehicleCollisionTesterCastCylinder(Layers::MOVING, 1.0f)); // Use half wheel width as convex radius so we get a rounded cyclinder
	mPhysicsSystem->AddConstraint(mVehicleConstraint);
	mPhysicsSystem->AddStepListener(mVehicleConstraint);
}

void MotorcycleTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	VehicleTest::PrePhysicsUpdate(inParams);

	// Determine acceleration and brake
	float forward = 0.0f, right = 0.0f, brake = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(DIK_Z))
		brake = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_UP))
		forward = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))		
		forward = -1.0f;

	// Check if we're reversing direction
	if (mPreviousForward * forward < 0.0f)
	{
		// Get vehicle velocity in local space to the body of the vehicle
		float velocity = (mMotorcycleBody->GetRotation().Conjugated() * mMotorcycleBody->GetLinearVelocity()).GetZ();
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

	// Steering
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))
		right = -1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))
		right = 1.0f;
	const float steer_speed = 4.0f;
	if (right > mCurrentRight)
		mCurrentRight = min(mCurrentRight + steer_speed * inParams.mDeltaTime, right);
	else if (right < mCurrentRight)
		mCurrentRight = max(mCurrentRight - steer_speed * inParams.mDeltaTime, right);

	// When leaned, we don't want to use the brakes fully as we'll spin out
	if (brake > 0.0f)
	{
		Vec3 world_up = -mPhysicsSystem->GetGravity().Normalized();
		Vec3 up = mMotorcycleBody->GetRotation() * mVehicleConstraint->GetLocalUp();
		Vec3 fwd = mMotorcycleBody->GetRotation() * mVehicleConstraint->GetLocalForward();
		float sin_lean_angle = abs(world_up.Cross(up).Dot(fwd));
		float brake_multiplier = Square(1.0f - sin_lean_angle);
		brake *= brake_multiplier;
	}

	// On user input, assure that the motorcycle is active
	if (mCurrentRight != 0.0f || forward != 0.0f || brake != 0.0f)
		mBodyInterface->ActivateBody(mMotorcycleBody->GetID());

	// Pass the input on to the constraint
	WheeledVehicleController *controller = static_cast<WheeledVehicleController *>(mVehicleConstraint->GetController());
	controller->SetDriverInput(forward, mCurrentRight, brake, false);

	// Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
	for (uint w = 0; w < 2; ++w)
	{
		const WheelSettings *settings = mVehicleConstraint->GetWheels()[w]->GetSettings();
		RMat44 wheel_transform = mVehicleConstraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cyclinder we draw is aligned with Y so we specify that as rotational axis
		mDebugRenderer->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);
	}
}

void MotorcycleTest::SaveState(StateRecorder& inStream) const
{
	VehicleTest::SaveState(inStream);

	inStream.Write(mPreviousForward);
	inStream.Write(mCurrentRight);
}

void MotorcycleTest::RestoreState(StateRecorder& inStream)
{
	VehicleTest::RestoreState(inStream);

	inStream.Read(mPreviousForward);
	inStream.Read(mCurrentRight);
}

void MotorcycleTest::GetInitialCamera(CameraState &ioState) const 
{
	// Position camera behind motorcycle
	RVec3 cam_tgt = RVec3(0, 0, 5);
	ioState.mPos = RVec3(0, 2.5f, -5);
	ioState.mForward = Vec3(cam_tgt - ioState.mPos).Normalized();
}

RMat44 MotorcycleTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const 
{
	// Pivot is center of motorcycle and rotates with motorcycle around Y axis only
	Vec3 fwd = mMotorcycleBody->GetRotation().RotateAxisZ();
	fwd.SetY(0.0f);
	float len = fwd.Length();
	if (len != 0.0f)
		fwd /= len;
	else
		fwd = Vec3::sAxisZ();
	Vec3 up = Vec3::sAxisY();
	Vec3 right = up.Cross(fwd);
	return RMat44(Vec4(right, 0), Vec4(up, 0), Vec4(fwd, 0), mMotorcycleBody->GetPosition());
}

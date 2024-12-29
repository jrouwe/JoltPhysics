// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/VehicleSixDOFTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleSixDOFTest)
{
	JPH_ADD_BASE_CLASS(VehicleSixDOFTest, VehicleTest)
}

void VehicleSixDOFTest::Initialize()
{
	VehicleTest::Initialize();

	const float half_vehicle_length = 2.0f;
	const float half_vehicle_width = 0.9f;
	const float half_vehicle_height = 0.2f;

	const float half_wheel_height = 0.3f;
	const float half_wheel_width = 0.05f;
	const float half_wheel_travel = 0.5f;

	Vec3 wheel_position[] =
	{
		Vec3(-half_vehicle_width, -half_vehicle_height, half_vehicle_length - 2.0f * half_wheel_height),
		Vec3(half_vehicle_width, -half_vehicle_height, half_vehicle_length - 2.0f * half_wheel_height),
		Vec3(-half_vehicle_width, -half_vehicle_height, -half_vehicle_length + 2.0f * half_wheel_height),
		Vec3(half_vehicle_width, -half_vehicle_height, -half_vehicle_length + 2.0f * half_wheel_height),
	};

	RVec3 position(0, 2, 0);

	RefConst<Shape> body_shape = new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length));

	Ref<CylinderShape> wheel_shape = new CylinderShape(half_wheel_width, half_wheel_height);
	wheel_shape->SetDensity(1.0e4f);

	// Create group filter
	Ref<GroupFilterTable> group_filter = new GroupFilterTable;

	// Create vehicle body
	mCarBody = mBodyInterface->CreateBody(BodyCreationSettings(body_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mCarBody->SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
	mBodyInterface->AddBody(mCarBody->GetID(), EActivation::Activate);

	// Create wheels
	for (int i = 0; i < (int)EWheel::Num; ++i)
	{
		bool is_front = sIsFrontWheel((EWheel)i);
		bool is_left = sIsLeftWheel((EWheel)i);

		RVec3 wheel_pos1 = position + wheel_position[i];
		RVec3 wheel_pos2 = wheel_pos1 - Vec3(0, half_wheel_travel, 0);

		// Create body
		Body &wheel = *mBodyInterface->CreateBody(BodyCreationSettings(wheel_shape, wheel_pos2, Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING));
		wheel.SetFriction(1.0f);
		wheel.SetCollisionGroup(CollisionGroup(group_filter, 0, 0));
		mBodyInterface->AddBody(wheel.GetID(), EActivation::Activate);

		// Create constraint
		SixDOFConstraintSettings settings;
		settings.mPosition1 = wheel_pos1;
		settings.mPosition2 = wheel_pos2;
		settings.mAxisX1 = settings.mAxisX2 = is_left? -Vec3::sAxisX() : Vec3::sAxisX();
		settings.mAxisY1 = settings.mAxisY2 = Vec3::sAxisY();

		// The suspension works in the Y translation axis only
		settings.MakeFixedAxis(EAxis::TranslationX);
		settings.SetLimitedAxis(EAxis::TranslationY, -half_wheel_travel, half_wheel_travel);
		settings.MakeFixedAxis(EAxis::TranslationZ);
		settings.mMotorSettings[EAxis::TranslationY] = MotorSettings(2.0f, 1.0f, 1.0e5f, 0.0f);

		// Front wheel can rotate around the Y axis
		if (is_front)
			settings.SetLimitedAxis(EAxis::RotationY, -cMaxSteeringAngle, cMaxSteeringAngle);
		else
			settings.MakeFixedAxis(EAxis::RotationY);

		// The Z axis is static
		settings.MakeFixedAxis(EAxis::RotationZ);

		// The main engine drives the X axis
		settings.MakeFreeAxis(EAxis::RotationX);
		settings.mMotorSettings[EAxis::RotationX] = MotorSettings(2.0f, 1.0f, 0.0f, 0.5e4f);

		// The front wheel needs to be able to steer around the Y axis
		// However the motors work in the constraint space of the wheel, and since this rotates around the
		// X axis we need to drive both the Y and Z to steer
		if (is_front)
			settings.mMotorSettings[EAxis::RotationY] = settings.mMotorSettings[EAxis::RotationZ] = MotorSettings(10.0f, 1.0f, 0.0f, 1.0e6f);

		SixDOFConstraint *wheel_constraint = static_cast<SixDOFConstraint *>(settings.Create(*mCarBody, wheel));
		mPhysicsSystem->AddConstraint(wheel_constraint);
		mWheels[i] = wheel_constraint;

		// Drive the suspension
		wheel_constraint->SetTargetPositionCS(Vec3(0, -half_wheel_travel, 0));
		wheel_constraint->SetMotorState(EAxis::TranslationY, EMotorState::Position);

		// The front wheels steer around the Y axis, but in constraint space of the wheel this means we need to drive
		// both Y and Z (see comment above)
		if (is_front)
		{
			wheel_constraint->SetTargetOrientationCS(Quat::sIdentity());
			wheel_constraint->SetMotorState(EAxis::RotationY, EMotorState::Position);
			wheel_constraint->SetMotorState(EAxis::RotationZ, EMotorState::Position);
		}
	}

	UpdateCameraPivot();
}

void VehicleSixDOFTest::ProcessInput(const ProcessInputParams &inParams)
{
	const float max_rotation_speed = 10.0f * JPH_PI;

	// Determine steering and speed
	mSteeringAngle = 0.0f;
	mSpeed = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Left))	mSteeringAngle = cMaxSteeringAngle;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Right))	mSteeringAngle = -cMaxSteeringAngle;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Up))		mSpeed = max_rotation_speed;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Down))	mSpeed = -max_rotation_speed;
}

void VehicleSixDOFTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	VehicleTest::PrePhysicsUpdate(inParams);

	UpdateCameraPivot();

	// On user input, assure that the car is active
	if (mSteeringAngle != 0.0f || mSpeed != 0.0f)
		mBodyInterface->ActivateBody(mCarBody->GetID());

	// Brake if current velocity is in the opposite direction of the desired velocity
	float car_speed = mCarBody->GetLinearVelocity().Dot(mCarBody->GetRotation().RotateAxisZ());
	bool brake = mSpeed != 0.0f && car_speed != 0.0f && Sign(mSpeed) != Sign(car_speed);

	// Front wheels
	const EWheel front_wheels[] = { EWheel::LeftFront, EWheel::RightFront };
	for (EWheel w : front_wheels)
	{
		SixDOFConstraint *wheel_constraint = mWheels[(int)w];
		if (wheel_constraint == nullptr)
			continue;

		// Steer front wheels
		Quat steering_rotation = Quat::sRotation(Vec3::sAxisY(), mSteeringAngle);
		wheel_constraint->SetTargetOrientationCS(steering_rotation);

		if (brake)
		{
			// Brake on front wheels
			wheel_constraint->SetTargetAngularVelocityCS(Vec3::sZero());
			wheel_constraint->SetMotorState(EAxis::RotationX, EMotorState::Velocity);
		}
		else if (mSpeed != 0.0f)
		{
			// Front wheel drive, since the motors are applied in the constraint space of the wheel
			// it is always applied on the X axis
			wheel_constraint->SetTargetAngularVelocityCS(Vec3(sIsLeftWheel(w)? -mSpeed : mSpeed, 0, 0));
			wheel_constraint->SetMotorState(EAxis::RotationX, EMotorState::Velocity);
		}
		else
		{
			// Free spin
			wheel_constraint->SetMotorState(EAxis::RotationX, EMotorState::Off);
		}
	}

	// Rear wheels
	const EWheel rear_wheels[] = { EWheel::LeftRear, EWheel::RightRear };
	for (EWheel w : rear_wheels)
	{
		SixDOFConstraint *wheel_constraint = mWheels[(int)w];
		if (wheel_constraint == nullptr)
			continue;

		if (brake)
		{
			// Brake on rear wheels
			wheel_constraint->SetTargetAngularVelocityCS(Vec3::sZero());
			wheel_constraint->SetMotorState(EAxis::RotationX, EMotorState::Velocity);
		}
		else
		{
			// Free spin
			wheel_constraint->SetMotorState(EAxis::RotationX, EMotorState::Off);
		}
	}
}

void VehicleSixDOFTest::GetInitialCamera(CameraState &ioState) const
{
	// Position camera behind car
	RVec3 cam_tgt = RVec3(0, 0, 5);
	ioState.mPos = RVec3(0, 2.5_r, -5);
	ioState.mForward = Vec3(cam_tgt - ioState.mPos).Normalized();
}

void VehicleSixDOFTest::UpdateCameraPivot()
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
	mCameraPivot = RMat44(Vec4(right, 0), Vec4(up, 0), Vec4(fwd, 0), mCarBody->GetPosition());
}

void VehicleSixDOFTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mSteeringAngle);
	inStream.Write(mSpeed);
}

void VehicleSixDOFTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mSteeringAngle);
	inStream.Read(mSpeed);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/TankTest.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Vehicle/TrackedVehicleController.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(TankTest)
{
	JPH_ADD_BASE_CLASS(TankTest, VehicleTest)
}

TankTest::~TankTest()
{
	mPhysicsSystem->RemoveStepListener(mVehicleConstraint);
}

void TankTest::Initialize()
{
	VehicleTest::Initialize();

	const float wheel_radius = 0.3f;
	const float wheel_width = 0.1f;
	const float half_vehicle_length = 3.2f;
	const float half_vehicle_width = 1.7f;
	const float half_vehicle_height = 0.5f;
	const float suspension_min_length = 0.3f;
	const float suspension_max_length = 0.5f;
	const float suspension_frequency = 1.0f;

	const float half_turret_width = 1.4f;
	const float	half_turret_length = 2.0f;
	const float half_turret_height = 0.4f;

	const float half_barrel_length = 1.5f;
	const float barrel_radius = 0.1f;
	const float barrel_rotation_offset = 0.2f;

	static Vec3 wheel_pos[] = {
		Vec3(0.0f, -0.0f, 2.95f),
		Vec3(0.0f, -0.3f, 2.1f),
		Vec3(0.0f, -0.3f, 1.4f),
		Vec3(0.0f, -0.3f, 0.7f),
		Vec3(0.0f, -0.3f, 0.0f),
		Vec3(0.0f, -0.3f, -0.7f),
		Vec3(0.0f, -0.3f, -1.4f),
		Vec3(0.0f, -0.3f, -2.1f),
		Vec3(0.0f, -0.0f, -2.75f),
	};

	// Create filter to prevent body, turret and barrel from colliding
	GroupFilter *filter = new GroupFilterTable;

	// Create tank body
	RVec3 body_position(0, 2, 0);
	RefConst<Shape> tank_body_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
	BodyCreationSettings tank_body_settings(tank_body_shape, body_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	tank_body_settings.mCollisionGroup.SetGroupFilter(filter);
	tank_body_settings.mCollisionGroup.SetGroupID(0);
	tank_body_settings.mCollisionGroup.SetSubGroupID(0);
	tank_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	tank_body_settings.mMassPropertiesOverride.mMass = 4000.0f;
	mTankBody = mBodyInterface->CreateBody(tank_body_settings);
	mBodyInterface->AddBody(mTankBody->GetID(), EActivation::Activate);

	// Create vehicle constraint
	VehicleConstraintSettings vehicle;
	vehicle.mDrawConstraintSize = 0.1f;
	vehicle.mMaxPitchRollAngle = DegreesToRadians(60.0f);

	TrackedVehicleControllerSettings *controller = new TrackedVehicleControllerSettings;
	vehicle.mController = controller;

	for (int t = 0; t < 2; ++t)
	{
		VehicleTrackSettings &track = controller->mTracks[t];

		// Last wheel is driven wheel
		track.mDrivenWheel = (uint)(vehicle.mWheels.size() + size(wheel_pos) - 1);

		for (uint wheel = 0; wheel < size(wheel_pos); ++wheel)
		{
			WheelSettingsTV *w = new WheelSettingsTV;
			w->mPosition = wheel_pos[wheel];
			w->mPosition.SetX(t == 0? half_vehicle_width : -half_vehicle_width);
			w->mRadius = wheel_radius;
			w->mWidth = wheel_width;
			w->mSuspensionMinLength = suspension_min_length;
			w->mSuspensionMaxLength = wheel == 0 || wheel == size(wheel_pos) - 1? suspension_min_length : suspension_max_length;
			w->mSuspensionSpring.mFrequency = suspension_frequency;

			// Add the wheel to the vehicle
			track.mWheels.push_back((uint)vehicle.mWheels.size());
			vehicle.mWheels.push_back(w);
		}
	}

	mVehicleConstraint = new VehicleConstraint(*mTankBody, vehicle);
	mVehicleConstraint->SetVehicleCollisionTester(new VehicleCollisionTesterRay(Layers::MOVING));
#ifdef JPH_DEBUG_RENDERER
	static_cast<TrackedVehicleController *>(mVehicleConstraint->GetController())->SetRPMMeter(Vec3(0, 2, 0), 0.5f);
#endif // JPH_DEBUG_RENDERER
	mPhysicsSystem->AddConstraint(mVehicleConstraint);
	mPhysicsSystem->AddStepListener(mVehicleConstraint);

	// Create turret
	RVec3 turret_position = body_position + Vec3(0, half_vehicle_height + half_turret_height, 0);
	BodyCreationSettings turret_body_setings(new BoxShape(Vec3(half_turret_width, half_turret_height, half_turret_length)), turret_position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	turret_body_setings.mCollisionGroup.SetGroupFilter(filter);
	turret_body_setings.mCollisionGroup.SetGroupID(0);
	turret_body_setings.mCollisionGroup.SetSubGroupID(0);
	turret_body_setings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	turret_body_setings.mMassPropertiesOverride.mMass = 2000.0f;
	mTurretBody = mBodyInterface->CreateBody(turret_body_setings);
	mBodyInterface->AddBody(mTurretBody->GetID(), EActivation::Activate);

	// Attach turret to body
	HingeConstraintSettings turret_hinge;
	turret_hinge.mPoint1 = turret_hinge.mPoint2 = body_position + Vec3(0, half_vehicle_height, 0);
	turret_hinge.mHingeAxis1 = turret_hinge.mHingeAxis2 = Vec3::sAxisY();
	turret_hinge.mNormalAxis1 = turret_hinge.mNormalAxis2 = Vec3::sAxisZ();
	turret_hinge.mMotorSettings = MotorSettings(0.5f, 1.0f);
	mTurretHinge = static_cast<HingeConstraint *>(turret_hinge.Create(*mTankBody, *mTurretBody));
	mTurretHinge->SetMotorState(EMotorState::Position);
	mPhysicsSystem->AddConstraint(mTurretHinge);

	// Create barrel
	RVec3 barrel_position = turret_position + Vec3(0, 0, half_turret_length + half_barrel_length - barrel_rotation_offset);
	BodyCreationSettings barrel_body_setings(new CylinderShape(half_barrel_length, barrel_radius), barrel_position, Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), EMotionType::Dynamic, Layers::MOVING);
	barrel_body_setings.mCollisionGroup.SetGroupFilter(filter);
	barrel_body_setings.mCollisionGroup.SetGroupID(0);
	barrel_body_setings.mCollisionGroup.SetSubGroupID(0);
	barrel_body_setings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	barrel_body_setings.mMassPropertiesOverride.mMass = 200.0f;
	mBarrelBody = mBodyInterface->CreateBody(barrel_body_setings);
	mBodyInterface->AddBody(mBarrelBody->GetID(), EActivation::Activate);

	// Attach barrel to turret
	HingeConstraintSettings barrel_hinge;
	barrel_hinge.mPoint1 = barrel_hinge.mPoint2 = barrel_position - Vec3(0, 0, half_barrel_length);
	barrel_hinge.mHingeAxis1 = barrel_hinge.mHingeAxis2 = -Vec3::sAxisX();
	barrel_hinge.mNormalAxis1 = barrel_hinge.mNormalAxis2 = Vec3::sAxisZ();
	barrel_hinge.mLimitsMin = DegreesToRadians(-10.0f);
	barrel_hinge.mLimitsMax = DegreesToRadians(40.0f);
	barrel_hinge.mMotorSettings = MotorSettings(10.0f, 1.0f);
	mBarrelHinge = static_cast<HingeConstraint *>(barrel_hinge.Create(*mTurretBody, *mBarrelBody));
	mBarrelHinge->SetMotorState(EMotorState::Position);
	mPhysicsSystem->AddConstraint(mBarrelHinge);

	// Update camera pivot
	mCameraPivot = mTankBody->GetPosition();
}

void TankTest::ProcessInput(const ProcessInputParams &inParams)
{
	const float min_velocity_pivot_turn = 1.0f;

	// Determine acceleration and brake
	mForward = 0.0f;
	mBrake = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(DIK_RSHIFT))
		mBrake = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_UP))
		mForward = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))
		mForward = -1.0f;

	// Steering
	mLeftRatio = 1.0f;
	mRightRatio = 1.0f;
	float velocity = (mTankBody->GetRotation().Conjugated() * mTankBody->GetLinearVelocity()).GetZ();
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))
	{
		if (mBrake == 0.0f && mForward == 0.0f && abs(velocity) < min_velocity_pivot_turn)
		{
			// Pivot turn
			mLeftRatio = -1.0f;
			mForward = 1.0f;
		}
		else
			mLeftRatio = 0.6f;
	}
	else if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))
	{
		if (mBrake == 0.0f && mForward == 0.0f && abs(velocity) < min_velocity_pivot_turn)
		{
			// Pivot turn
			mRightRatio = -1.0f;
			mForward = 1.0f;
		}
		else
			mRightRatio = 0.6f;
	}

	// Check if we're reversing direction
	if (mPreviousForward * mForward < 0.0f)
	{
		// Get vehicle velocity in local space to the body of the vehicle
		if ((mForward > 0.0f && velocity < -0.1f) || (mForward < 0.0f && velocity > 0.1f))
		{
			// Brake while we've not stopped yet
			mForward = 0.0f;
			mBrake = 1.0f;
		}
		else
		{
			// When we've come to a stop, accept the new direction
			mPreviousForward = mForward;
		}
	}

	// Cast ray to find target
	RRayCast ray { inParams.mCameraState.mPos, 1000.0f * inParams.mCameraState.mForward };
	RayCastSettings ray_settings;
	ClosestHitCollisionCollector<CastRayCollector> collector;
	IgnoreMultipleBodiesFilter body_filter;
	body_filter.Reserve(3);
	body_filter.IgnoreBody(mTankBody->GetID());
	body_filter.IgnoreBody(mTurretBody->GetID());
	body_filter.IgnoreBody(mBarrelBody->GetID());
	mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, ray_settings, collector, {}, {}, body_filter);
	RVec3 hit_pos = collector.HadHit()? ray.GetPointOnRay(collector.mHit.mFraction) : ray.mOrigin + ray.mDirection;
	mDebugRenderer->DrawMarker(hit_pos, Color::sGreen, 1.0f);

	// Orient the turret towards the hit position
	RMat44 turret_to_world = mTankBody->GetCenterOfMassTransform() * mTurretHinge->GetConstraintToBody1Matrix();
	Vec3 hit_pos_in_turret = Vec3(turret_to_world.InversedRotationTranslation() * hit_pos);
	mTurretHeading = ATan2(hit_pos_in_turret.GetZ(), hit_pos_in_turret.GetY());

	// Orient barrel towards the hit position
	RMat44 barrel_to_world = mTurretBody->GetCenterOfMassTransform() * mBarrelHinge->GetConstraintToBody1Matrix();
	Vec3 hit_pos_in_barrel = Vec3(barrel_to_world.InversedRotationTranslation() * hit_pos);
	mBarrelPitch = ATan2(hit_pos_in_barrel.GetZ(), hit_pos_in_barrel.GetY());

	// If user wants to fire
	mFire = inParams.mKeyboard->IsKeyPressed(DIK_RETURN);
}

void TankTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	VehicleTest::PrePhysicsUpdate(inParams);

	const float bullet_radius = 0.061f; // 120 mm
	const Vec3 bullet_pos = Vec3(0, 1.6f, 0);
	const Vec3 bullet_velocity = Vec3(0, 400.0f, 0); // Normal exit velocities are around 1100-1700 m/s, use a lower variable as we have a limit to max velocity (See: https://tanks-encyclopedia.com/coldwar-usa-120mm-gun-tank-m1e1-abrams/)
	const float bullet_mass = 40.0f; // Normal projectile weight is around 7 kg, use an increased value so the momentum is more realistic (with the lower exit velocity)
	const float bullet_reload_time = 2.0f;

	// Update camera pivot
	mCameraPivot = mTankBody->GetPosition();

	// Assure the tank stays active as we're controlling the turret with the mouse
	mBodyInterface->ActivateBody(mTankBody->GetID());

	// Pass the input on to the constraint
	static_cast<TrackedVehicleController *>(mVehicleConstraint->GetController())->SetDriverInput(mForward, mLeftRatio, mRightRatio, mBrake);
	mTurretHinge->SetTargetAngle(mTurretHeading);
	mBarrelHinge->SetTargetAngle(mBarrelPitch);

	// Update reload time
	mReloadTime = max(0.0f, mReloadTime - inParams.mDeltaTime);

	// Shoot bullet
	if (mReloadTime == 0.0f && mFire)
	{
		// Create bullet
		BodyCreationSettings bullet_creation_settings(new SphereShape(bullet_radius), mBarrelBody->GetCenterOfMassTransform() * bullet_pos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		bullet_creation_settings.mMotionQuality = EMotionQuality::LinearCast;
		bullet_creation_settings.mFriction = 1.0f;
		bullet_creation_settings.mRestitution = 0.0f;
		bullet_creation_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		bullet_creation_settings.mMassPropertiesOverride.mMass = bullet_mass;
		Body *bullet = mBodyInterface->CreateBody(bullet_creation_settings);
		bullet->SetLinearVelocity(mBarrelBody->GetRotation() * bullet_velocity);
		mBodyInterface->AddBody(bullet->GetID(), EActivation::Activate);

		// Start reloading
		mReloadTime = bullet_reload_time;

		// Apply opposite impulse to turret body
		mBodyInterface->AddImpulse(mTurretBody->GetID(), -bullet->GetLinearVelocity() * bullet_mass);
	}

	// Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
	for (uint w = 0; w < mVehicleConstraint->GetWheels().size(); ++w)
	{
		const WheelSettings *settings = mVehicleConstraint->GetWheels()[w]->GetSettings();
		RMat44 wheel_transform = mVehicleConstraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cylinder we draw is aligned with Y so we specify that as rotational axis
		mDebugRenderer->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);
	}
}

void TankTest::SaveState(StateRecorder &inStream) const
{
	VehicleTest::SaveState(inStream);

	inStream.Write(mReloadTime);
}

void TankTest::RestoreState(StateRecorder &inStream)
{
	VehicleTest::RestoreState(inStream);

	inStream.Read(mReloadTime);
}

void TankTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mForward);
	inStream.Write(mPreviousForward);
	inStream.Write(mLeftRatio);
	inStream.Write(mRightRatio);
	inStream.Write(mBrake);
	inStream.Write(mTurretHeading);
	inStream.Write(mBarrelPitch);
	inStream.Write(mFire);
}

void TankTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mForward);
	inStream.Read(mPreviousForward);
	inStream.Read(mLeftRatio);
	inStream.Read(mRightRatio);
	inStream.Read(mBrake);
	inStream.Read(mTurretHeading);
	inStream.Read(mBarrelPitch);
	inStream.Read(mFire);
}

void TankTest::GetInitialCamera(CameraState &ioState) const
{
	// Position camera behind tank
	ioState.mPos = RVec3(0, 4.0f, 0);
	ioState.mForward = Vec3(0, -2.0f, 10.0f).Normalized();
}

RMat44 TankTest::GetCameraPivot(float inCameraHeading, float inCameraPitch) const
{
	// Pivot is center of tank + a distance away from the tank based on the heading and pitch of the camera
	Vec3 fwd = Vec3(Cos(inCameraPitch) * Cos(inCameraHeading), Sin(inCameraPitch), Cos(inCameraPitch) * Sin(inCameraHeading));
	return RMat44::sTranslation(mCameraPivot - 10.0f * fwd);
}

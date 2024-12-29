// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Water/BoatTest.h>
#include <Jolt/Core/QuickSort.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(BoatTest)
{
	JPH_ADD_BASE_CLASS(BoatTest, Test)
}

void BoatTest::Initialize()
{
	// Create boat
	ConvexHullShapeSettings boat_hull;
	boat_hull.mPoints = {
		Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
		Vec3(cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
		Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
		Vec3(cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
		Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
		Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
		Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
		Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
		Vec3(0, cHalfBoatHeight, cHalfBoatLength + cBoatBowLength)
	};
	boat_hull.SetEmbedded();
	OffsetCenterOfMassShapeSettings com_offset(Vec3(0, -cHalfBoatHeight, 0), &boat_hull);
	com_offset.SetEmbedded();
	RVec3 position(0, cMaxWaterHeight + 2, 0);
	BodyCreationSettings boat(&com_offset, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	boat.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	boat.mMassPropertiesOverride.mMass = cBoatMass;
	mBoatBody = mBodyInterface->CreateBody(boat);
	mBodyInterface->AddBody(mBoatBody->GetID(), EActivation::Activate);

	// Create water sensor. We use this to detect which bodies entered the water (in this sample we could have assumed everything is in the water)
	BodyCreationSettings water_sensor(new BoxShape(Vec3(cWaterWidth, cMaxWaterHeight, cWaterWidth)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
	water_sensor.mIsSensor = true;
	mWaterSensor = mBodyInterface->CreateAndAddBody(water_sensor, EActivation::Activate);

	// Create some barrels to float in the water
	default_random_engine random;
	BodyCreationSettings barrel(new CylinderShape(1.0f, 0.7f), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	barrel.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	barrel.mMassPropertiesOverride.mMass = cBarrelMass;
	for (int i = 0; i < 10; ++i)
	{
		barrel.mPosition = RVec3(-10.0f + i * 2.0f, cMaxWaterHeight + 2, 10);
		barrel.mRotation = Quat::sRandom(random);
		mBodyInterface->CreateAndAddBody(barrel, EActivation::Activate);
	}

	UpdateCameraPivot();
}

void BoatTest::ProcessInput(const ProcessInputParams &inParams)
{
	// Determine acceleration and brake
	mForward = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Up))
		mForward = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(EKey::Down))
		mForward = -1.0f;

	// Steering
	mRight = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(EKey::Left))
		mRight = -1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(EKey::Right))
		mRight = 1.0f;
}

RVec3 BoatTest::GetWaterSurfacePosition(RVec3Arg inXZPosition) const
{
	return RVec3(inXZPosition.GetX(), cMinWaterHeight + Sin(0.1f * float(inXZPosition.GetZ()) + mTime) * (cMaxWaterHeight - cMinWaterHeight), inXZPosition.GetZ());
}

void BoatTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	// Draw the water surface
	const float step = 1.0f;
	for (float z = -cWaterWidth; z < cWaterWidth; z += step)
	{
		RVec3 p1 = GetWaterSurfacePosition(RVec3(-cWaterWidth, 0, z));
		RVec3 p2 = GetWaterSurfacePosition(RVec3(-cWaterWidth, 0, z + step));
		RVec3 p3 = GetWaterSurfacePosition(RVec3(cWaterWidth, 0, z));
		RVec3 p4 = GetWaterSurfacePosition(RVec3(cWaterWidth, 0, z + step));
		mDebugRenderer->DrawTriangle(p1, p2, p3, Color::sBlue);
		mDebugRenderer->DrawTriangle(p2, p4, p3, Color::sBlue);
	}

	// Apply buoyancy to all bodies in the water
	{
		lock_guard<Mutex> lock(mBodiesInWaterMutex);
		for (const BodyID &id : mBodiesInWater)
		{
			BodyLockWrite body_lock(mPhysicsSystem->GetBodyLockInterface(), id);
			Body &body = body_lock.GetBody();
			if (body.IsActive())
			{
				// Use center of mass position to determine water surface position (you could test multiple points on the actual shape of the boat to get a more accurate result)
				RVec3 surface_position = GetWaterSurfacePosition(body.GetCenterOfMassPosition());

				// Crude way of approximating the surface normal
				RVec3 p2 = GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(0, 0, 1));
				RVec3 p3 = GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(1, 0, 0));
				Vec3 surface_normal = Vec3(p2 - surface_position).Cross(Vec3(p3 - surface_position)).Normalized();

				// Determine buoyancy and drag
				float buoyancy, linear_drag, angular_drag;
				if (id == mBoatBody->GetID())
				{
					buoyancy = cBoatBuoyancy;
					linear_drag = cBoatLinearDrag;
					angular_drag = cBoatAngularDrag;
				}
				else
				{
					buoyancy = cBarrelBuoyancy;
					linear_drag = cBarrelLinearDrag;
					angular_drag = cBarrelAngularDrag;
				}

				// Apply buoyancy to the body
				body.ApplyBuoyancyImpulse(surface_position, surface_normal, buoyancy, linear_drag, angular_drag, Vec3::sZero(), mPhysicsSystem->GetGravity(), inParams.mDeltaTime);
			}
		}
	}

	// On user input, assure that the boat is active
	if (mRight != 0.0f || mForward != 0.0f)
		mBodyInterface->ActivateBody(mBoatBody->GetID());

	// Apply forces to rear of boat where the propeller would be but only when the propeller is under water
	RVec3 propeller_position = mBoatBody->GetWorldTransform() * Vec3(0, -cHalfBoatHeight, -cHalfBoatLength);
	RVec3 propeller_surface_position = GetWaterSurfacePosition(propeller_position);
	if (propeller_surface_position.GetY() > propeller_position.GetY())
	{
		Vec3 forward = mBoatBody->GetRotation().RotateAxisZ();
		Vec3 right = mBoatBody->GetRotation().RotateAxisX();
		mBoatBody->AddImpulse((forward * mForward * cForwardAcceleration + right * Sign(mForward) * mRight * cSteerAcceleration) * cBoatMass * inParams.mDeltaTime, propeller_position);
	}

	UpdateCameraPivot();
}

void BoatTest::SaveInputState(StateRecorder &inStream) const
{
	inStream.Write(mForward);
	inStream.Write(mRight);
}

void BoatTest::RestoreInputState(StateRecorder &inStream)
{
	inStream.Read(mForward);
	inStream.Read(mRight);
}

void BoatTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mBodiesInWater);
}

void BoatTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mBodiesInWater);
}

void BoatTest::GetInitialCamera(CameraState &ioState) const
{
	// Position camera behind boat
	RVec3 cam_tgt = RVec3(0, 0, 5);
	ioState.mPos = RVec3(0, 5, -10);
	ioState.mForward = Vec3(cam_tgt - ioState.mPos).Normalized();
}

void BoatTest::UpdateCameraPivot()
{
	// Pivot is center of boat and rotates with boat around Y axis only
	Vec3 fwd = mBoatBody->GetRotation().RotateAxisZ();
	fwd.SetY(0.0f);
	float len = fwd.Length();
	if (len != 0.0f)
		fwd /= len;
	else
		fwd = Vec3::sAxisZ();
	Vec3 up = Vec3::sAxisY();
	Vec3 right = up.Cross(fwd);
	mCameraPivot = RMat44(Vec4(right, 0), Vec4(up, 0), Vec4(fwd, 0), mBoatBody->GetPosition());
}

void BoatTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// When a body enters the water add it to the list of bodies in the water
	lock_guard<Mutex> lock(mBodiesInWaterMutex);
	if (inBody1.GetID() == mWaterSensor)
		mBodiesInWater.push_back(inBody2.GetID());
	else if (inBody2.GetID() == mWaterSensor)
		mBodiesInWater.push_back(inBody1.GetID());
	QuickSort(mBodiesInWater.begin(), mBodiesInWater.end()); // Sort to make deterministic (OnContactAdded is called from multiple threads and the order is not guaranteed)
}

void BoatTest::OnContactRemoved(const SubShapeIDPair &inSubShapePair)
{
	// When a body leaves the water remove it from the list of bodies in the water
	lock_guard<Mutex> lock(mBodiesInWaterMutex);
	if (inSubShapePair.GetBody1ID() == mWaterSensor)
		mBodiesInWater.erase(std::find(mBodiesInWater.begin(), mBodiesInWater.end(), inSubShapePair.GetBody2ID()));
	else if (inSubShapePair.GetBody2ID() == mWaterSensor)
		mBodiesInWater.erase(std::find(mBodiesInWater.begin(), mBodiesInWater.end(), inSubShapePair.GetBody1ID()));
}

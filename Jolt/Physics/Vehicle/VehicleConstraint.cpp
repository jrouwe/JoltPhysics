// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/VehicleController.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(VehicleConstraintSettings)
{
	JPH_ADD_BASE_CLASS(VehicleConstraintSettings, ConstraintSettings)

	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mUp)
	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mForward)
	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mMaxPitchRollAngle)
	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mWheels)
	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mAntiRollBars)
	JPH_ADD_ATTRIBUTE(VehicleConstraintSettings, mController)
}

void VehicleConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	ConstraintSettings::SaveBinaryState(inStream);

	inStream.Write(mUp);
	inStream.Write(mForward);
	inStream.Write(mMaxPitchRollAngle);

	uint32 num_anti_rollbars = (uint32)mAntiRollBars.size();
	inStream.Write(num_anti_rollbars);
	for (const VehicleAntiRollBar &r : mAntiRollBars)
		r.SaveBinaryState(inStream);

	uint32 num_wheels = (uint32)mWheels.size();
	inStream.Write(num_wheels);
	for (const WheelSettings *w : mWheels)
		w->SaveBinaryState(inStream);

	inStream.Write(mController->GetRTTI()->GetHash());
	mController->SaveBinaryState(inStream);
}

void VehicleConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	ConstraintSettings::RestoreBinaryState(inStream);

	inStream.Read(mUp);
	inStream.Read(mForward);
	inStream.Read(mMaxPitchRollAngle);

	uint32 num_anti_rollbars = 0;
	inStream.Read(num_anti_rollbars);
	mAntiRollBars.resize(num_anti_rollbars);
	for (VehicleAntiRollBar &r : mAntiRollBars)
		r.RestoreBinaryState(inStream);

	uint32 num_wheels = 0;
	inStream.Read(num_wheels);
	mWheels.resize(num_wheels);
	for (WheelSettings *w : mWheels)
		w->RestoreBinaryState(inStream);

	uint32 hash = 0;
	inStream.Read(hash);
	const RTTI *rtti = Factory::sInstance->Find(hash);
	mController = reinterpret_cast<VehicleControllerSettings *>(rtti->CreateObject());
	mController->RestoreBinaryState(inStream);
}

VehicleConstraint::VehicleConstraint(Body &inVehicleBody, const VehicleConstraintSettings &inSettings) :
	Constraint(inSettings)
{
	// Check sanity of incoming settings
	JPH_ASSERT(inSettings.mForward.IsNormalized());
	JPH_ASSERT(inSettings.mUp.IsNormalized());
	JPH_ASSERT(!inSettings.mWheels.empty());

	// Store general properties
	mBody = &inVehicleBody;
	mUp = inSettings.mUp;
	mForward = inSettings.mForward;
	SetMaxPitchRollAngle(inSettings.mMaxPitchRollAngle);

	// Copy anti-rollbar settings
	mAntiRollBars.resize(inSettings.mAntiRollBars.size());
	for (uint i = 0; i < mAntiRollBars.size(); ++i)
	{
		const VehicleAntiRollBar &r = inSettings.mAntiRollBars[i];
		mAntiRollBars[i] = r;
		JPH_ASSERT(r.mStiffness >= 0.0f);
	}

	// Construct our controler class
	mController = inSettings.mController->ConstructController(*this);

	// Create wheels
	mWheels.resize(inSettings.mWheels.size());
	for (uint i = 0; i < mWheels.size(); ++i)
		mWheels[i] = mController->ConstructWheel(*inSettings.mWheels[i]);
}

VehicleConstraint::~VehicleConstraint()
{
	// Destroy controller
	delete mController;

	// Destroy our wheels
	for (Wheel *w : mWheels)
		delete w;
}

Mat44 VehicleConstraint::GetWheelLocalTransform(uint inWheelIndex, Vec3Arg inWheelRight, Vec3Arg inWheelUp) const
{
	JPH_ASSERT(inWheelIndex < mWheels.size());

	const Wheel *wheel = mWheels[inWheelIndex];
	const WheelSettings *settings = wheel->mSettings;

	// Use the two vectors provided to calculate a matrix that takes us from wheel model space to X = right, Y = up, Z = forward (the space where we will rotate the wheel)
	Mat44 wheel_to_rotational = Mat44(Vec4(inWheelRight, 0), Vec4(inWheelUp, 0), Vec4(inWheelUp.Cross(inWheelRight), 0), Vec4(0, 0, 0, 1)).Transposed();

	// Calculate the matrix that takes us from the rotational space to vehicle local space
	Vec3 local_forward = Quat::sRotation(mUp, wheel->mSteerAngle) * mForward;
	Vec3 local_right = local_forward.Cross(mUp);
	Vec3 local_wheel_pos = settings->mPosition + settings->mDirection * (wheel->mContactLength - settings->mRadius);
	Mat44 rotational_to_local(Vec4(local_right, 0), Vec4(mUp, 0), Vec4(local_forward, 0), Vec4(local_wheel_pos, 1));

	// Calculate transform of rotated wheel
	return rotational_to_local * Mat44::sRotationX(wheel->mAngle) * wheel_to_rotational;
}

Mat44 VehicleConstraint::GetWheelWorldTransform(uint inWheelIndex, Vec3Arg inWheelRight, Vec3Arg inWheelUp) const
{
	return mBody->GetWorldTransform() * GetWheelLocalTransform(inWheelIndex, inWheelRight, inWheelUp);
}

void VehicleConstraint::OnStep(float inDeltaTime, PhysicsSystem &inPhysicsSystem)
{
	JPH_PROFILE_FUNCTION();

	// Callback on our controller
	mController->PreCollide(inDeltaTime, inPhysicsSystem);

	// Calculate if this constraint is active by checking if our main vehicle body is active or any of the bodies we touch are active
	mIsActive = mBody->IsActive();

	// Test collision for wheels
	for (uint wheel_index = 0; wheel_index < mWheels.size(); ++wheel_index)
	{
		Wheel *w = mWheels[wheel_index];
		const WheelSettings *settings = w->mSettings;

		// Reset contact
		w->mContactBodyID = BodyID();
		w->mContactBody = nullptr;
		w->mContactSubShapeID = SubShapeID();
		float max_len = settings->mSuspensionMaxLength + settings->mRadius;
		w->mContactLength = max_len;

		// Test collision to find the floor
		Vec3 origin = mBody->GetCenterOfMassPosition() + mBody->GetRotation() * (settings->mPosition - mBody->GetShape()->GetCenterOfMass());
		w->mWSDirection = mBody->GetRotation() * settings->mDirection;
		if (mVehicleCollisionTester->Collide(inPhysicsSystem, wheel_index, origin, w->mWSDirection, max_len, mBody->GetID(), w->mContactBody, w->mContactSubShapeID, w->mContactPosition, w->mContactNormal, w->mContactLength))
		{
			// Store ID (pointer is not valid outside of the simulation step)
			w->mContactBodyID = w->mContactBody->GetID();

			// Store contact velocity, cache this as the contact body may be removed
			w->mContactPointVelocity = w->mContactBody->GetPointVelocity(w->mContactPosition);

			// Check if body is active, if so the entire vehicle should be active
			mIsActive |= w->mContactBody->IsActive();

			// Determine world space forward using steering angle and body rotation
			Vec3 forward = mBody->GetRotation() * Quat::sRotation(mUp, w->mSteerAngle) * mForward;

			// Calculate frame of reference for the contact
			w->mContactLateral = forward.Cross(w->mContactNormal).NormalizedOr(Vec3::sZero());
			w->mContactLongitudinal = w->mContactNormal.Cross(w->mContactLateral);
		}
	}

	// Calculate anti-rollbar impulses
	for (const VehicleAntiRollBar &r : mAntiRollBars)
	{
		Wheel *lw = mWheels[r.mLeftWheel];
		Wheel *rw = mWheels[r.mRightWheel];

		if (lw->mContactBody != nullptr && rw->mContactBody != nullptr)
		{
			// Calculate the impulse to apply based on the difference in suspension length
			float difference = rw->mContactLength - lw->mContactLength;
			float impulse = difference * r.mStiffness * inDeltaTime;
			lw->mAntiRollBarImpulse = -impulse;
			rw->mAntiRollBarImpulse = impulse;
		}
		else
		{
			// When one of the wheels is not on the ground we don't apply any impulses
			lw->mAntiRollBarImpulse = rw->mAntiRollBarImpulse = 0.0f;
		}
	}

	// Callback on our controller
	mController->PostCollide(inDeltaTime, inPhysicsSystem);

	// If the wheels are rotating, we don't want to go to sleep yet
	bool allow_sleep = true;
	for (const Wheel *w : mWheels)
		if (abs(w->mAngularVelocity) > DegreesToRadians(10.0f))
			allow_sleep = false;
	if (mBody->GetAllowSleeping() != allow_sleep)
		mBody->SetAllowSleeping(allow_sleep);
}

void VehicleConstraint::BuildIslands(uint32 inConstraintIndex, IslandBuilder &ioBuilder, BodyManager &inBodyManager) 
{
	// Find dynamic bodies that our wheels are touching
	BodyID *body_ids = (BodyID *)JPH_STACK_ALLOC((mWheels.size() + 1) * sizeof(BodyID));
	int num_bodies = 0;
	bool needs_to_activate = false;
	for (const Wheel *w : mWheels)
		if (w->mContactBody != nullptr)
		{
			// Avoid adding duplicates
			bool duplicate = false;
			BodyID id = w->mContactBody->GetID();
			for (int i = 0; i < num_bodies; ++i)
				if (body_ids[i] == id)
				{
					duplicate = true;
					break;
				}
			if (duplicate)
				continue;

			if (w->mContactBody->IsDynamic())
				body_ids[num_bodies++] = id;
			needs_to_activate |= !w->mContactBody->IsActive();
		}

	// Activate bodies
	if (needs_to_activate)
	{
		if (!mBody->IsActive())
		{
			// Our main body is not active, activate it too
			body_ids[num_bodies] = mBody->GetID();
			inBodyManager.ActivateBodies(body_ids, num_bodies + 1);
		}
		else
		{
			// Only activate bodies the wheels are touching
			inBodyManager.ActivateBodies(body_ids, num_bodies);
		}
	}

	// Link the bodies into the same island
	uint32 min_active_index = Body::cInactiveIndex;
	for (int i = 0; i < num_bodies; ++i)
	{
		const Body &body = inBodyManager.GetBody(body_ids[i]);
		min_active_index = min(min_active_index, body.GetIndexInActiveBodiesInternal());
		ioBuilder.LinkBodies(mBody->GetIndexInActiveBodiesInternal(), body.GetIndexInActiveBodiesInternal());
	}

	// Link the constraint in the island
	ioBuilder.LinkConstraint(inConstraintIndex, mBody->GetIndexInActiveBodiesInternal(), min_active_index); 
}

void VehicleConstraint::CalculateWheelContactPoint(Mat44Arg inBodyTransform, const Wheel &inWheel, Vec3 &outR1PlusU, Vec3 &outR2) const
{
	Vec3 contact_pos = inBodyTransform * (inWheel.mSettings->mPosition + inWheel.mSettings->mDirection * inWheel.mContactLength);
	outR1PlusU = contact_pos - mBody->GetCenterOfMassPosition();
	outR2 = contact_pos - mBody->GetCenterOfMassPosition();
}

void VehicleConstraint::CalculatePitchRollConstraintProperties(float inDeltaTime, Mat44Arg inBodyTransform)
{
	// Check if a limit was specified
	if (mCosMaxPitchRollAngle < JPH_PI)
	{
		// Calculate cos of angle between world up vector and vehicle up vector
		Vec3 vehicle_up = inBodyTransform.Multiply3x3(mUp);
		mCosPitchRollAngle = mUp.Dot(vehicle_up);
		if (mCosPitchRollAngle < mCosMaxPitchRollAngle)
		{
			// Calculate rotation axis to rotate vehicle towards up
			Vec3 rotation_axis = mUp.Cross(vehicle_up);
			float len = rotation_axis.Length();
			if (len > 0.0f)
				mPitchRollRotationAxis = rotation_axis / len;

			mPitchRollPart.CalculateConstraintProperties(inDeltaTime, *mBody, Body::sFixedToWorld, mPitchRollRotationAxis);
		}
		else
			mPitchRollPart.Deactivate();
	}
	else
		mPitchRollPart.Deactivate();
}

void VehicleConstraint::SetupVelocityConstraint(float inDeltaTime)
{
	Mat44 body_transform = mBody->GetWorldTransform();

	for (Wheel *w : mWheels)
		if (w->mContactBody != nullptr)
		{
			const WheelSettings *settings = w->mSettings;

			Vec3 r1_plus_u, r2;
			CalculateWheelContactPoint(body_transform, *w, r1_plus_u, r2);

			// Suspension spring
			if (settings->mSuspensionMaxLength > settings->mSuspensionMinLength)
				w->mSuspensionPart.CalculateConstraintProperties(inDeltaTime, *mBody, r1_plus_u, *w->mContactBody, r2, w->mWSDirection, w->mAntiRollBarImpulse, w->mContactLength - settings->mRadius - settings->mSuspensionMaxLength - settings->mSuspensionPreloadLength, settings->mSuspensionFrequency, settings->mSuspensionDamping);
			else
				w->mSuspensionPart.Deactivate();

			// Check if we reached the 'max up' position
			float max_up_error = w->mContactLength - settings->mRadius - settings->mSuspensionMinLength;
			if (max_up_error < 0.0f)
				w->mSuspensionMaxUpPart.CalculateConstraintProperties(inDeltaTime, *mBody, r1_plus_u, *w->mContactBody, r2, w->mWSDirection, 0.0f, max_up_error);
			else
				w->mSuspensionMaxUpPart.Deactivate();
			
			// Friction and propulsion
			w->mLongitudinalPart.CalculateConstraintProperties(inDeltaTime, *mBody, r1_plus_u, *w->mContactBody, r2, -w->mContactLongitudinal, 0.0f, 0.0f, 0.0f, 0.0f);
			w->mLateralPart.CalculateConstraintProperties(inDeltaTime, *mBody, r1_plus_u, *w->mContactBody, r2, -w->mContactLateral, 0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			// No contact -> disable everything
			w->mSuspensionPart.Deactivate();
			w->mSuspensionMaxUpPart.Deactivate();
			w->mLongitudinalPart.Deactivate();
			w->mLateralPart.Deactivate();
		}

	CalculatePitchRollConstraintProperties(inDeltaTime, body_transform);
}

void VehicleConstraint::WarmStartVelocityConstraint(float inWarmStartImpulseRatio) 
{
	for (Wheel *w : mWheels)
		if (w->mContactBody != nullptr)
		{
			w->mSuspensionPart.WarmStart(*mBody, *w->mContactBody, w->mWSDirection, inWarmStartImpulseRatio);
			w->mSuspensionMaxUpPart.WarmStart(*mBody, *w->mContactBody, w->mWSDirection, inWarmStartImpulseRatio);
			w->mLongitudinalPart.WarmStart(*mBody, *w->mContactBody, -w->mContactLongitudinal, 0.0f); // Don't warm start the longitudinal part (the engine/brake force, we don't want to preserve anything from the last frame)
			w->mLateralPart.WarmStart(*mBody, *w->mContactBody, -w->mContactLateral, inWarmStartImpulseRatio);	
		}

	mPitchRollPart.WarmStart(*mBody, Body::sFixedToWorld, inWarmStartImpulseRatio);
}

bool VehicleConstraint::SolveVelocityConstraint(float inDeltaTime) 
{
	bool impulse = false;

	// Solve suspension
	for (Wheel *w : mWheels)
		if (w->mContactBody != nullptr)
		{
			// Suspension spring, note that it can only push and not pull
			if (w->mSuspensionPart.IsActive())
				impulse |= w->mSuspensionPart.SolveVelocityConstraint(*mBody, *w->mContactBody, w->mWSDirection, 0.0f, FLT_MAX);

			// When reaching the minimal suspension length only allow forces pushing the bodies away
			if (w->mSuspensionMaxUpPart.IsActive())
				impulse |= w->mSuspensionMaxUpPart.SolveVelocityConstraint(*mBody, *w->mContactBody, w->mWSDirection, 0.0f, FLT_MAX);
		}

	// Solve the horizontal movement of the vehicle
	impulse |= mController->SolveLongitudinalAndLateralConstraints(inDeltaTime);

	// Apply the pitch / roll constraint to avoid the vehicle from toppling over
	impulse |= mPitchRollPart.SolveVelocityConstraint(*mBody, Body::sFixedToWorld, mPitchRollRotationAxis, 0, FLT_MAX);

	return impulse;
}

bool VehicleConstraint::SolvePositionConstraint(float inDeltaTime, float inBaumgarte) 
{
	bool impulse = false;

	Mat44 body_transform = mBody->GetWorldTransform();

	for (Wheel *w : mWheels)
		if (w->mContactBody != nullptr)
		{
			const WheelSettings *settings = w->mSettings;

			// Calculate new contact length as the body may have moved
			// TODO: This assumes that only the vehicle moved and not the ground (contact point/normal is stored in world space)
			Vec3 ws_direction = body_transform.Multiply3x3(settings->mDirection);
			Vec3 ws_position = body_transform * settings->mPosition;
			float contact_length = (w->mContactPosition - ws_position).Dot(ws_direction);

			// Check if we reached the 'max up' position
			float max_up_error = contact_length - settings->mRadius - settings->mSuspensionMinLength;
			if (max_up_error < 0.0f)
			{
				// Recalculate constraint properties since the body may have moved
				Vec3 r1_plus_u, r2;
				CalculateWheelContactPoint(body_transform, *w, r1_plus_u, r2);
				w->mSuspensionMaxUpPart.CalculateConstraintProperties(inDeltaTime, *mBody, r1_plus_u, *w->mContactBody, r2, ws_direction, 0.0f, max_up_error);

				impulse |= w->mSuspensionMaxUpPart.SolvePositionConstraint(*mBody, *w->mContactBody, ws_direction, max_up_error, inBaumgarte);
			}
		}

	// Apply the pitch / roll constraint to avoid the vehicle from toppling over
	CalculatePitchRollConstraintProperties(inDeltaTime, body_transform);
	if (mPitchRollPart.IsActive())
		impulse |= mPitchRollPart.SolvePositionConstraint(*mBody, Body::sFixedToWorld, mCosPitchRollAngle - mCosMaxPitchRollAngle, inBaumgarte);

	return impulse;
}

#ifdef JPH_DEBUG_RENDERER

void VehicleConstraint::DrawConstraint(DebugRenderer *inRenderer) const 
{
	mController->Draw(inRenderer);
}

void VehicleConstraint::DrawConstraintLimits(DebugRenderer *inRenderer) const 
{
}

#endif // JPH_DEBUG_RENDERER

void VehicleConstraint::SaveState(StateRecorder &inStream) const
{
	Constraint::SaveState(inStream);

	mController->SaveState(inStream);

	for (const Wheel *w : mWheels)
	{
		inStream.Write(w->mAngularVelocity);
		inStream.Write(w->mAngle);

		w->mSuspensionPart.SaveState(inStream);
		w->mSuspensionMaxUpPart.SaveState(inStream);
		w->mLongitudinalPart.SaveState(inStream);
		w->mLateralPart.SaveState(inStream);
	}

	inStream.Write(mPitchRollRotationAxis); // When rotation is too small we use last frame so we need to store it
	mPitchRollPart.SaveState(inStream);
}

void VehicleConstraint::RestoreState(StateRecorder &inStream)
{
	Constraint::RestoreState(inStream);

	mController->RestoreState(inStream);

	for (Wheel *w : mWheels)
	{
		inStream.Read(w->mAngularVelocity);
		inStream.Read(w->mAngle);

		w->mSuspensionPart.RestoreState(inStream);
		w->mSuspensionMaxUpPart.RestoreState(inStream);
		w->mLongitudinalPart.RestoreState(inStream);
		w->mLateralPart.RestoreState(inStream);
	}

	inStream.Read(mPitchRollRotationAxis);
	mPitchRollPart.RestoreState(inStream);
}

Ref<ConstraintSettings> VehicleConstraint::GetConstraintSettings() const
{
	JPH_ASSERT(false); // Not implemented yet
	return nullptr;
}

JPH_NAMESPACE_END

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(WheeledVehicleControllerSettings)
{
	JPH_ADD_BASE_CLASS(WheeledVehicleControllerSettings, VehicleControllerSettings)

	JPH_ADD_ATTRIBUTE(WheeledVehicleControllerSettings, mEngine)
	JPH_ADD_ATTRIBUTE(WheeledVehicleControllerSettings, mTransmission)
	JPH_ADD_ATTRIBUTE(WheeledVehicleControllerSettings, mDifferentials)
}

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(WheelSettingsWV)
{
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mInertia)
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mAngularDamping)	
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mMaxSteerAngle)
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mLongitudinalFriction)
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mLateralFriction)
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mMaxBrakeTorque)
	JPH_ADD_ATTRIBUTE(WheelSettingsWV, mMaxHandBrakeTorque)
}

WheelSettingsWV::WheelSettingsWV()
{
	mLongitudinalFriction.Reserve(3);
	mLongitudinalFriction.AddPoint(0.0f, 0.0f);
	mLongitudinalFriction.AddPoint(0.06f, 1.2f);
	mLongitudinalFriction.AddPoint(0.2f, 1.0f);

	mLateralFriction.Reserve(3);
	mLateralFriction.AddPoint(0.0f, 0.0f);
	mLateralFriction.AddPoint(3.0f, 1.2f);
	mLateralFriction.AddPoint(20.0f, 1.0f);
}

void WheelSettingsWV::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mInertia);
	inStream.Write(mAngularDamping);
	inStream.Write(mMaxSteerAngle);
	mLongitudinalFriction.SaveBinaryState(inStream);
	mLateralFriction.SaveBinaryState(inStream);
	inStream.Write(mMaxBrakeTorque);
	inStream.Write(mMaxHandBrakeTorque);
}

void WheelSettingsWV::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mInertia);
	inStream.Read(mAngularDamping);
	inStream.Read(mMaxSteerAngle);
	mLongitudinalFriction.RestoreBinaryState(inStream);
	mLateralFriction.RestoreBinaryState(inStream);
	inStream.Read(mMaxBrakeTorque);
	inStream.Read(mMaxHandBrakeTorque);
}

WheelWV::WheelWV(const WheelSettingsWV &inSettings) :
	Wheel(inSettings)
{
	JPH_ASSERT(inSettings.mInertia >= 0.0f);
	JPH_ASSERT(inSettings.mAngularDamping >= 0.0f);
	JPH_ASSERT(abs(inSettings.mMaxSteerAngle) <= 0.5f * JPH_PI);
	JPH_ASSERT(inSettings.mMaxBrakeTorque >= 0.0f);
	JPH_ASSERT(inSettings.mMaxHandBrakeTorque >= 0.0f);
}

void WheelWV::Update(float inDeltaTime, const VehicleConstraint &inConstraint)
{
	const WheelSettingsWV *settings = GetSettings();

	// Angular damping: dw/dt = -c * w
	// Solution: w(t) = w(0) * e^(-c * t) or w2 = w1 * e^(-c * dt)
	// Taylor expansion of e^(-c * dt) = 1 - c * dt + ...
	// Since dt is usually in the order of 1/60 and c is a low number too this approximation is good enough
	mAngularVelocity *= max(0.0f, 1.0f - settings->mAngularDamping * inDeltaTime);

	// Update rotation of wheel
	mAngle = fmod(mAngle + mAngularVelocity * inDeltaTime, 2.0f * JPH_PI);

	if (mContactBody != nullptr)
	{
		const Body *body = inConstraint.GetVehicleBody();

		// Calculate relative velocity between wheel contact point and floor
		Vec3 relative_velocity = body->GetPointVelocity(mContactPosition) - mContactPointVelocity;

		// Cancel relative velocity in the normal plane
		relative_velocity -= mContactNormal.Dot(relative_velocity) * mContactNormal;
		float relative_longitudinal_velocity = relative_velocity.Dot(mContactLongitudinal);

		// Calculate longitudinal friction based on difference between velocity of rolling wheel and drive surface
		float longitudinal_slip	= relative_longitudinal_velocity != 0.0f? abs((mAngularVelocity * settings->mRadius - relative_longitudinal_velocity) / relative_longitudinal_velocity) : 0.0f;
		float longitudinal_slip_friction = settings->mLongitudinalFriction.GetValue(longitudinal_slip);

		// Calculate lateral friction based on slip angle
		float relative_velocity_len = relative_velocity.Length();
		float lateral_slip_angle = relative_velocity_len < 1.0e-3f? 0.0f : RadiansToDegrees(ACos(abs(relative_longitudinal_velocity) / relative_velocity_len));
		float lateral_slip_friction = settings->mLateralFriction.GetValue(lateral_slip_angle);

		// Tire friction
		mCombinedLongitudinalFriction = sqrt(longitudinal_slip_friction * mContactBody->GetFriction());
		mCombinedLateralFriction = sqrt(lateral_slip_friction * mContactBody->GetFriction());
	}
	else
	{
		// No collision
		mCombinedLongitudinalFriction = mCombinedLateralFriction = 0.0f;
	}
}

VehicleController *WheeledVehicleControllerSettings::ConstructController(VehicleConstraint &inConstraint) const
{
	return new WheeledVehicleController(*this, inConstraint);
}

void WheeledVehicleControllerSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	mEngine.SaveBinaryState(inStream);

	mTransmission.SaveBinaryState(inStream);

	uint32 num_differentials = (uint32)mDifferentials.size();
	inStream.Write(num_differentials);
	for (const VehicleDifferentialSettings &d : mDifferentials)
		d.SaveBinaryState(inStream);
}

void WheeledVehicleControllerSettings::RestoreBinaryState(StreamIn &inStream)
{
	mEngine.RestoreBinaryState(inStream);

	mTransmission.RestoreBinaryState(inStream);

	uint32 num_differentials = 0;
	inStream.Read(num_differentials);
	mDifferentials.resize(num_differentials);
	for (VehicleDifferentialSettings &d : mDifferentials)
		d.RestoreBinaryState(inStream);
}

WheeledVehicleController::WheeledVehicleController(const WheeledVehicleControllerSettings &inSettings, VehicleConstraint &inConstraint) :
	VehicleController(inConstraint)
{
	// Copy engine settings
	static_cast<VehicleEngineSettings &>(mEngine) = inSettings.mEngine;
	JPH_ASSERT(inSettings.mEngine.mMinRPM >= 0.0f);
	JPH_ASSERT(inSettings.mEngine.mMinRPM <= inSettings.mEngine.mMaxRPM);

	// Copy transmission settings
	static_cast<VehicleTransmissionSettings &>(mTransmission) = inSettings.mTransmission;
#ifdef JPH_ENABLE_ASSERTS
	for (float r : inSettings.mTransmission.mGearRatios)
		JPH_ASSERT(r > 0.0f);
	for (float r : inSettings.mTransmission.mReverseGearRatios)
		JPH_ASSERT(r < 0.0f);
#endif // JPH_ENABLE_ASSERTS
	JPH_ASSERT(inSettings.mTransmission.mSwitchTime >= 0.0f);
	JPH_ASSERT(inSettings.mTransmission.mShiftDownRPM > 0.0f);
	JPH_ASSERT(inSettings.mTransmission.mMode != ETransmissionMode::Auto || inSettings.mTransmission.mShiftUpRPM < inSettings.mEngine.mMaxRPM);
	JPH_ASSERT(inSettings.mTransmission.mShiftUpRPM > inSettings.mTransmission.mShiftDownRPM);

	// Copy differential settings
	mDifferentials.resize(inSettings.mDifferentials.size());
	for (uint i = 0; i < mDifferentials.size(); ++i)
	{
		const VehicleDifferentialSettings &d = inSettings.mDifferentials[i];
		mDifferentials[i] = d;
		JPH_ASSERT(d.mDifferentialRatio > 0.0f);
		JPH_ASSERT(d.mLeftRightSplit >= 0.0f && d.mLeftRightSplit <= 1.0f);
		JPH_ASSERT(d.mEngineTorqueRatio >= 0.0f);
	}
}

void WheeledVehicleController::PreCollide(float inDeltaTime, PhysicsSystem &inPhysicsSystem)
{
	JPH_PROFILE_FUNCTION();

	for (Wheel *w_base : mConstraint.GetWheels())
	{
		WheelWV *w = static_cast<WheelWV *>(w_base);

		// Set steering angle
		w->SetSteerAngle(-mRightInput * w->GetSettings()->mMaxSteerAngle);
	}
}

void WheeledVehicleController::PostCollide(float inDeltaTime, PhysicsSystem &inPhysicsSystem)
{
	JPH_PROFILE_FUNCTION();

	Wheels &wheels = mConstraint.GetWheels();

	// Update wheel angle, do this before applying torque to the wheels (as friction will slow them down again)
	for (Wheel *w_base : wheels)
	{
		WheelWV *w = static_cast<WheelWV *>(w_base);
		w->Update(inDeltaTime, mConstraint);
	}

	// Update engine with damping
	mEngine.Update(inDeltaTime);

	// In auto transmission mode, don't accelerate the engine when switching gears
	float forward_input = abs(mForwardInput);
	if (mTransmission.mMode == ETransmissionMode::Auto)
		forward_input *= mTransmission.GetClutchFriction();

	// Calculate engine torque
	float engine_torque = mEngine.GetTorque(forward_input);

	bool can_engine_apply_torque = false;
	float transmission_torque = 0.0f;
	if (mTransmission.GetCurrentGear() != 0 && mTransmission.GetClutchFriction() > 1.0e-3f)
	{
		// Calculate average wheel speed after differential
		float average_clutch_speed = 0.0f;
		float average_clutch_speed_denom = 0.0f;
		for (const VehicleDifferentialSettings &d : mDifferentials)
		{
			if (d.mLeftWheel != -1)
			{
				const Wheel *w = wheels[d.mLeftWheel];
				average_clutch_speed += w->GetAngularVelocity() * d.mDifferentialRatio;
				average_clutch_speed_denom += 1.0f;
				can_engine_apply_torque |= d.mLeftRightSplit < 1.0f && w->HasContact();
			}
			if (d.mRightWheel != -1)
			{
				const Wheel *w = wheels[d.mRightWheel];
				average_clutch_speed += w->GetAngularVelocity() * d.mDifferentialRatio;
				average_clutch_speed_denom += 1.0f;
				can_engine_apply_torque |= d.mLeftRightSplit > 0.0f && w->HasContact();
			}
		}

		if (average_clutch_speed_denom != 0.0f)
		{
			// Calculate final average speed of wheels at clutch
			float transmission_ratio = mTransmission.GetCurrentRatio();
			average_clutch_speed /= average_clutch_speed_denom;
			average_clutch_speed *= transmission_ratio;

			// Angular velocity of engine
			float engine_speed = mEngine.GetAngularVelocity();

			// Calculate torque from engine to clutch
			transmission_torque = mTransmission.GetClutchFriction() * mTransmission.mClutchStrength * (engine_speed - average_clutch_speed);
			float transmission_torque_on_differentials = transmission_ratio * transmission_torque;

			for (const VehicleDifferentialSettings &d : mDifferentials)
			{
				// Calculate torque on this differential
				float differential_torque = d.mEngineTorqueRatio * d.mDifferentialRatio * transmission_torque_on_differentials;

				// Left wheel
				if (d.mLeftWheel != -1 && d.mLeftRightSplit < 1.0f)
				{
					WheelWV *w = static_cast<WheelWV *>(wheels[d.mLeftWheel]);
					float wheel_torque = differential_torque * (1.0f - d.mLeftRightSplit);
					w->ApplyTorque(wheel_torque, inDeltaTime);
				}

				// Right wheel
				if (d.mRightWheel != -1 && d.mLeftRightSplit > 0.0f)
				{
					WheelWV *w = static_cast<WheelWV *>(wheels[d.mRightWheel]);
					float wheel_torque = differential_torque * d.mLeftRightSplit;
					w->ApplyTorque(wheel_torque, inDeltaTime);
				}
			}
		}
	}

	// Apply torque to engine rotation
	mEngine.ApplyTorque(engine_torque - transmission_torque, inDeltaTime);

	// Update transmission
	mTransmission.Update(inDeltaTime, mEngine.GetCurrentRPM(), mForwardInput, can_engine_apply_torque);
	
	// Braking
	for (Wheel *w_base : wheels)
	{
		WheelWV *w = static_cast<WheelWV *>(w_base);
		const WheelSettingsWV *settings = w->GetSettings();

		// Combine brake with hand brake torque
		float brake_torque = mBrakeInput * settings->mMaxBrakeTorque + mHandBrakeInput * settings->mMaxHandBrakeTorque;
		if (brake_torque > 0.0f)
		{
			// Calculate how much torque is needed to stop the wheels from rotating in this time step
			float brake_torque_to_lock_wheels = abs(w->GetAngularVelocity()) * settings->mInertia / inDeltaTime;
			if (brake_torque > brake_torque_to_lock_wheels)
			{
				// Wheels are locked
				w->SetAngularVelocity(0.0f);
				w->mBrakeImpulse = (brake_torque - brake_torque_to_lock_wheels) * inDeltaTime / settings->mRadius;
			}
			else
			{
				// Slow down the wheels
				w->ApplyTorque(-Sign(w->GetAngularVelocity()) * brake_torque, inDeltaTime);
				w->mBrakeImpulse = 0.0f;
			}
		}
		else
		{
			// Not braking
			w->mBrakeImpulse = 0.0f;
		}
	}
}

bool WheeledVehicleController::SolveLongitudinalAndLateralConstraints(float inDeltaTime) 
{
	bool impulse = false;

	for (Wheel *w_base : mConstraint.GetWheels())
		if (w_base->HasContact())
		{
			WheelWV *w = static_cast<WheelWV *>(w_base);
			const WheelSettingsWV *settings = w->GetSettings();

			// Calculate max impulse that we can apply on the ground
			float max_longitudinal_friction_impulse = w->mCombinedLongitudinalFriction * w->GetSuspensionLambda();

			// Calculate relative velocity between wheel contact point and floor in longitudinal direction
			Vec3 relative_velocity = mConstraint.GetVehicleBody()->GetPointVelocity(w->GetContactPosition()) - w->GetContactPointVelocity();
			float relative_longitudinal_velocity = relative_velocity.Dot(w->GetContactLongitudinal());

			// Calculate brake force to apply
			float min_longitudinal_impulse, max_longitudinal_impulse;
			if (w->mBrakeImpulse != 0.0f)
			{
				// Limit brake force by max tire friction
				float brake_impulse = min(w->mBrakeImpulse, max_longitudinal_friction_impulse);

				// Check which direction the brakes should be applied (we don't want to apply an impulse that would accelerate the vehicle)
				if (relative_longitudinal_velocity >= 0.0f)
				{
					min_longitudinal_impulse = -brake_impulse;
					max_longitudinal_impulse = 0.0f;
				}
				else
				{
					min_longitudinal_impulse = 0.0f;
					max_longitudinal_impulse = brake_impulse;
				}

				// Longitudinal impulse, note that we assume that once the wheels are locked that the brakes have more than enough torque to keep the wheels locked so we exclude any rotation deltas
				impulse |= w->SolveLongitudinalConstraintPart(mConstraint, min_longitudinal_impulse, max_longitudinal_impulse);
			}
			else
			{
				// Assume we want to apply an angular impulse that makes the delta velocity between wheel and ground zero in one time step, calculate the amount of linear impulse needed to do that
				float desired_angular_velocity = relative_longitudinal_velocity / settings->mRadius;
				float linear_impulse = (w->GetAngularVelocity() - desired_angular_velocity) * settings->mInertia / settings->mRadius;

				// Limit the impulse by max tire friction
				min_longitudinal_impulse = max_longitudinal_impulse = w->GetLongitudinalLambda() + Sign(linear_impulse) * min(abs(linear_impulse), max_longitudinal_friction_impulse);

				// Longitudinal impulse
				float prev_lambda = w->GetLongitudinalLambda();
				impulse |= w->SolveLongitudinalConstraintPart(mConstraint, min_longitudinal_impulse, max_longitudinal_impulse);

				// Update the angular velocity of the wheels according to the lambda that was applied
				w->SetAngularVelocity(w->GetAngularVelocity() - (w->GetLongitudinalLambda() - prev_lambda) * settings->mRadius / settings->mInertia);
			}
		}

	for (Wheel *w_base : mConstraint.GetWheels())
		if (w_base->HasContact())
		{
			WheelWV *w = static_cast<WheelWV *>(w_base);

			// Lateral friction
			float max_lateral_friction_impulse = w->mCombinedLateralFriction * w->GetSuspensionLambda();
			impulse |= w->SolveLateralConstraintPart(mConstraint, -max_lateral_friction_impulse, max_lateral_friction_impulse);
		}

	return impulse;
}

#ifdef JPH_DEBUG_RENDERER

void WheeledVehicleController::Draw(DebugRenderer *inRenderer) const 
{
	// Draw RPM
	Body *body = mConstraint.GetVehicleBody();
	Vec3 rpm_meter_up = body->GetRotation() * mConstraint.GetLocalUp();
	Vec3 rpm_meter_pos = body->GetPosition() + body->GetRotation() * mRPMMeterPosition;
	Vec3 rpm_meter_fwd = body->GetRotation() * mConstraint.GetLocalForward();
	mEngine.DrawRPM(inRenderer, rpm_meter_pos, rpm_meter_fwd, rpm_meter_up, mRPMMeterSize, mTransmission.mShiftDownRPM, mTransmission.mShiftUpRPM);

	// Draw current vehicle state
	String status = StringFormat("Forward: %.1f, Right: %.1f, Brake: %.1f, HandBrake: %.1f\n"
								 "Gear: %d, Clutch: %.1f, EngineRPM: %.0f, V: %.1f km/h", 
								 (double)mForwardInput, (double)mRightInput, (double)mBrakeInput, (double)mHandBrakeInput, 
								 mTransmission.GetCurrentGear(), (double)mTransmission.GetClutchFriction(), (double)mEngine.GetCurrentRPM(), (double)body->GetLinearVelocity().Length() * 3.6);
	inRenderer->DrawText3D(body->GetPosition(), status, Color::sWhite, mConstraint.GetDrawConstraintSize());

	for (const Wheel *w_base : mConstraint.GetWheels())
	{
		const WheelWV *w = static_cast<const WheelWV *>(w_base);
		const WheelSettings *settings = w->GetSettings();

		// Calculate where the suspension attaches to the body in world space
		Vec3 ws_position = body->GetCenterOfMassPosition() + body->GetRotation() * (settings->mPosition - body->GetShape()->GetCenterOfMass());

		if (w->HasContact())
		{
			// Draw contact
			inRenderer->DrawLine(ws_position, w->GetContactPosition(), w->HasHitHardPoint()? Color::sRed : Color::sGreen); // Red if we hit the 'max up' limit
			inRenderer->DrawLine(w->GetContactPosition(), w->GetContactPosition() + w->GetContactNormal(), Color::sYellow);
			inRenderer->DrawLine(w->GetContactPosition(), w->GetContactPosition() + w->GetContactLongitudinal(), Color::sRed);
			inRenderer->DrawLine(w->GetContactPosition(), w->GetContactPosition() + w->GetContactLateral(), Color::sBlue);

			DebugRenderer::sInstance->DrawText3D(w->GetContactPosition(), StringFormat("W: %.1f, S: %.2f, FrLateral: %.1f, FrLong: %.1f", (double)w->GetAngularVelocity(), (double)w->GetSuspensionLength(), (double)w->mCombinedLateralFriction, (double)w->mCombinedLongitudinalFriction), Color::sWhite, 0.1f);
		}
		else
		{
			// Draw 'no hit'
			Vec3 max_droop = body->GetRotation() * settings->mDirection * (settings->mSuspensionMaxLength + settings->mRadius);
			inRenderer->DrawLine(ws_position, ws_position + max_droop, Color::sYellow);

			DebugRenderer::sInstance->DrawText3D(ws_position + max_droop, StringFormat("W: %.1f", (double)w->GetAngularVelocity()), Color::sRed, 0.1f);
		}
	}
}

#endif // JPH_DEBUG_RENDERER

void WheeledVehicleController::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mForwardInput);
	inStream.Write(mRightInput);
	inStream.Write(mBrakeInput);
	inStream.Write(mHandBrakeInput);

	mEngine.SaveState(inStream);
	mTransmission.SaveState(inStream);
}

void WheeledVehicleController::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mForwardInput);
	inStream.Read(mRightInput);
	inStream.Read(mBrakeInput);
	inStream.Read(mHandBrakeInput);

	mEngine.RestoreState(inStream);
	mTransmission.RestoreState(inStream);
}

JPH_NAMESPACE_END

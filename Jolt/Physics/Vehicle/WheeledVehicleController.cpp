// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Math/DynMatrix.h>
#include <Jolt/Math/GaussianElimination.h>
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
	JPH_ADD_ATTRIBUTE(WheeledVehicleControllerSettings, mDifferentialLimitedSlipRatio)
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
		float relative_longitudinal_velocity_denom = Sign(relative_longitudinal_velocity) * max(1.0e-3f, abs(relative_longitudinal_velocity)); // Ensure we don't divide by zero
		mLongitudinalSlip = abs((mAngularVelocity * settings->mRadius - relative_longitudinal_velocity) / relative_longitudinal_velocity_denom);
		float longitudinal_slip_friction = settings->mLongitudinalFriction.GetValue(mLongitudinalSlip);

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
		mLongitudinalSlip = 0.0f;
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

	inStream.Write(mDifferentialLimitedSlipRatio);
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

	inStream.Read(mDifferentialLimitedSlipRatio);
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
	JPH_ASSERT(inSettings.mTransmission.mClutchStrength > 0.0f);

	// Copy differential settings
	mDifferentials.resize(inSettings.mDifferentials.size());
	for (uint i = 0; i < mDifferentials.size(); ++i)
	{
		const VehicleDifferentialSettings &d = inSettings.mDifferentials[i];
		mDifferentials[i] = d;
		JPH_ASSERT(d.mDifferentialRatio > 0.0f);
		JPH_ASSERT(d.mLeftRightSplit >= 0.0f && d.mLeftRightSplit <= 1.0f);
		JPH_ASSERT(d.mEngineTorqueRatio >= 0.0f);
		JPH_ASSERT(d.mLimitedSlipRatio > 1.0f);
	}

	mDifferentialLimitedSlipRatio = inSettings.mDifferentialLimitedSlipRatio;
	JPH_ASSERT(mDifferentialLimitedSlipRatio > 1.0f);
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

	// Remember old RPM so we're increasing or decreasing
	float old_engine_rpm = mEngine.GetCurrentRPM();

	Wheels &wheels = mConstraint.GetWheels();

	// Update wheel angle, do this before applying torque to the wheels (as friction will slow them down again)
	for (Wheel *w_base : wheels)
	{
		WheelWV *w = static_cast<WheelWV *>(w_base);
		w->Update(inDeltaTime, mConstraint);
	}

	// In auto transmission mode, don't accelerate the engine when switching gears
	float forward_input = abs(mForwardInput);
	if (mTransmission.mMode == ETransmissionMode::Auto)
		forward_input *= mTransmission.GetClutchFriction();

	// Apply damping if there is no acceleration
	if (forward_input < 1.0e-3f)
		mEngine.ApplyDamping(inDeltaTime);

	// Calculate engine torque
	float engine_torque = mEngine.GetTorque(forward_input);

	// Define a struct that contains information about driven differentials (i.e. that have wheels connected)
	struct DrivenDifferential
	{
		const VehicleDifferentialSettings *	mDifferential;
		float								mAngularVelocity;
		float								mClutchToDifferentialTorqueRatio;
		float								mTempTorqueFactor;
	};

	// Collect driven differentials and their speeds
	Array<DrivenDifferential> driven_differentials;
	driven_differentials.reserve(mDifferentials.size());
	float differential_omega_min = FLT_MAX, differential_omega_max = 0.0f;
	for (const VehicleDifferentialSettings &d : mDifferentials)
	{
		float avg_omega = 0.0f;
		int avg_omega_denom = 0;
		int indices[] = { d.mLeftWheel, d.mRightWheel };
		for (int idx : indices)
			if (idx != -1)
			{
				avg_omega += wheels[idx]->GetAngularVelocity();
				avg_omega_denom++;
			}

		if (avg_omega_denom > 0)
		{
			avg_omega = abs(avg_omega * d.mDifferentialRatio / float(avg_omega_denom)); // ignoring that the differentials may be rotating in different directions
			driven_differentials.push_back({ &d, avg_omega, d.mEngineTorqueRatio, 0 });

			// Remember min and max velocity
			differential_omega_min = min(differential_omega_min, avg_omega);
			differential_omega_max = max(differential_omega_max, avg_omega);
		}
	}

	if (mDifferentialLimitedSlipRatio < FLT_MAX					// Limited slip differential needs to be turned on
		&& differential_omega_max > differential_omega_min)		// There needs to be a velocity difference
	{
		// Calculate factor based on relative speed of a differential
		float sum_factor = 0.0f;
		for (DrivenDifferential &d : driven_differentials)
		{
			// Differential with max velocity gets factor 0, differential with min velocity 1
			d.mTempTorqueFactor = (differential_omega_max - d.mAngularVelocity) / (differential_omega_max - differential_omega_min);
			sum_factor += d.mTempTorqueFactor;
		}

		// Normalize the result
		for (DrivenDifferential &d : driven_differentials)
			d.mTempTorqueFactor /= sum_factor;

		// Prevent div by zero
		differential_omega_min = max(1.0e-3f, differential_omega_min);
		differential_omega_max = max(1.0e-3f, differential_omega_max);

		// Map into a value that is 0 when the wheels are turning at an equal rate and 1 when the wheels are turning at mDifferentialLimitedSlipRatio
		float alpha = min((differential_omega_max / differential_omega_min - 1.0f) / (mDifferentialLimitedSlipRatio - 1.0f), 1.0f);
		JPH_ASSERT(alpha >= 0.0f);
		float one_min_alpha = 1.0f - alpha;

		// Update torque ratio for all differentials
		for (DrivenDifferential &d : driven_differentials)
			d.mClutchToDifferentialTorqueRatio = one_min_alpha * d.mClutchToDifferentialTorqueRatio + alpha * d.mTempTorqueFactor;
	}

#ifdef JPH_ENABLE_ASSERTS
	// Assert the values add up to 1
	float sum_torque_factors = 0.0f;
	for (DrivenDifferential &d : driven_differentials)
		sum_torque_factors += d.mClutchToDifferentialTorqueRatio;
	JPH_ASSERT(abs(sum_torque_factors - 1.0f) < 1.0e-6f);
#endif // JPH_ENABLE_ASSERTS

	// Define a struct that collects information about the wheels that connect to the engine
	struct DrivenWheel
	{
		WheelWV *				mWheel;
		float					mClutchToWheelRatio;
		float					mClutchToWheelTorqueRatio;
	};
	Array<DrivenWheel> driven_wheels;
	driven_wheels.reserve(wheels.size());

	// Collect driven wheels
	float transmission_ratio = mTransmission.GetCurrentRatio();
	for (const DrivenDifferential &dd : driven_differentials)
	{
		VehicleDifferentialSettings d = *dd.mDifferential;

		WheelWV *wl = d.mLeftWheel != -1? static_cast<WheelWV *>(wheels[d.mLeftWheel]) : nullptr;
		WheelWV *wr = d.mRightWheel != -1? static_cast<WheelWV *>(wheels[d.mRightWheel]) : nullptr;

		float clutch_to_wheel_ratio = transmission_ratio * d.mDifferentialRatio;

		if (wl != nullptr && wr != nullptr)
		{
			// Calculate torque ratio
			float ratio_l, ratio_r;
			d.CalculateTorqueRatio(wl->GetAngularVelocity(), wr->GetAngularVelocity(), ratio_l, ratio_r);

			// Add both wheels
			driven_wheels.push_back({ wl, clutch_to_wheel_ratio, dd.mClutchToDifferentialTorqueRatio * ratio_l });
			driven_wheels.push_back({ wr, clutch_to_wheel_ratio, dd.mClutchToDifferentialTorqueRatio * ratio_r });
		}
		else if (wl != nullptr)
		{
			// Only left wheel, all power to left
			driven_wheels.push_back({ wl, clutch_to_wheel_ratio, dd.mClutchToDifferentialTorqueRatio });
		}
		else if (wr != nullptr)
		{
			// Only right wheel, all power to right
			driven_wheels.push_back({ wr, clutch_to_wheel_ratio, dd.mClutchToDifferentialTorqueRatio });
		}
	}

	bool solved = false;
	if (!driven_wheels.empty())
	{
		// Define the torque at the clutch at time t as:
		// 
		// tc(t):=S*(we(t)-sum(R(j)*ww(j,t),j,1,N)/N)
		//
		// Where:
		// S is the total strength of clutch (= friction * strength)
		// we(t) is the engine angular velocity at time t
		// R(j) is the total gear ratio of clutch to wheel for wheel j
		// ww(j,t) is the angular velocity of wheel j at time t
		// N is the amount of wheels
		//
		// The torque that increases the engine angular velocity at time t is:
		// 
		// te(t):=TE-tc(t)
		//
		// Where:
		// TE is the torque delivered by the engine
		//
		// The torque that increases the wheel angular velocity for wheel i at time t is:
		//
		// tw(i,t):=TW(i)+R(i)*F(i)*tc(t)
		//
		// Where:
		// TW(i) is the torque applied to the wheel outside of the engine (brake + torque due to friction with the ground)
		// F(i) is the fraction of the engine torque applied from engine to wheel i
		//
		// Because the angular accelaration and torque are connected through: Torque = I * dw/dt
		//
		// We have the angular acceleration of the engine at time t:
		//
		// ddt_we(t):=te(t)/Ie
		//
		// Where:
		// Ie is the inertia of the engine
		//
		// We have the angular acceleration of wheel i at time t:
		//
		// ddt_ww(i,t):=tw(i,t)/Iw(i)
		//
		// Where:
		// Iw(i) is the inertia of wheel i
		//
		// We could take a simple Euler step to calculate the resulting accelerations but because the system is very stiff this turns out to be unstable, so we need to use implicit Euler instead:
		//
		// we(t+dt)=we(t)+dt*ddt_we(t+dt)
		//
		// and:
		//
		// ww(i,t+dt)=ww(i,t)+dt*ddt_ww(i,t+dt)
		//
		// Expanding both equations (the equations above are in wxMaxima format and this can easily be done by expand(%)):
		//
		// For wheel:
		// 
		// ww(i,t+dt) + (S*dt*F(i)*R(i)*sum(R(j)*ww(j,t+dt),j,1,N))/(N*Iw(i)) - (S*dt*F(i)*R(i)*we(t+dt))/Iw(i) = ww(i,t)+(dt*TW(i))/Iw(i)
		//
		// For engine:
		//
		// we(t+dt) + (S*dt*we(t+dt))/Ie - (S*dt*sum(R(j)*ww(j,t+dt),j,1,N))/(Ie*N) = we(t)+(TE*dt)/Ie
		//
		// Defining a vector w(t) = (ww(1, t), ww(2, t), ..., ww(N, t), we(t)) we can write both equations as a matrix multiplication:
		//
		// a * w(t + dt) = b
		//
		// We then invert the matrix to get the new angular velocities.
		//
		// Note that currently we set TW(i) = 0 so that the wheels will accelerate as if no external force was applied to them. These external forces are applied later and will slow down the wheel before the end of the time step.

		// Dimension of matrix is N + 1
		int n = (int)driven_wheels.size() + 1;

		// Last column of w is for the engine angular velocity
		int engine = n - 1;

		// Define a and b
		DynMatrix a(n, n);
		DynMatrix b(n, 1);

		// Get number of driven wheels as a float
		float num_driven_wheels_float = float(driven_wheels.size());
	
		// Angular velocity of engine
		float w_engine = mEngine.GetAngularVelocity();

		// Calculate the total strength of the clutch
		float clutch_strength = transmission_ratio != 0.0f? mTransmission.GetClutchFriction() * mTransmission.mClutchStrength : 0.0f;

		// dt / Ie
		float dt_div_ie = inDeltaTime / mEngine.mInertia;

		// Iterate the rows for the wheels
		for (int i = 0; i < (int)driven_wheels.size(); ++i)
		{
			const DrivenWheel &w_i = driven_wheels[i];

			// dt / Iw
			float dt_div_iw = inDeltaTime / w_i.mWheel->GetSettings()->mInertia;

			// S * R(i)
			float s_r = clutch_strength * w_i.mClutchToWheelRatio;

			// dt * S * R(i) * F(i) / Iw
			float dt_s_r_f_div_iw = dt_div_iw * s_r * w_i.mClutchToWheelTorqueRatio;

			// Fill in the columns of a for wheel j
			for (int j = 0; j < (int)driven_wheels.size(); ++j)
			{
				const DrivenWheel &w_j = driven_wheels[j];
				a(i, j) = dt_s_r_f_div_iw * w_j.mClutchToWheelRatio / num_driven_wheels_float;
			}

			// Add ww(i, t+dt)
			a(i, i) += 1.0f;

			// Add the column for the engine
			a(i, engine) = -dt_s_r_f_div_iw;

			// Fill in the constant b
			b(i, 0) = w_i.mWheel->GetAngularVelocity(); // + dt_div_iw * (brake and tire torques)

			// To avoid looping over the wheels again, we also fill in the wheel columns of the engine row here
			a(engine, i) = -dt_div_ie * s_r / num_driven_wheels_float;
		}

		// Finalize the engine row
		a(engine, engine) = (1.0f + dt_div_ie * clutch_strength);
		b(engine, 0) = w_engine + dt_div_ie * engine_torque;

		// Solve the linear equation
		if (GaussianElimination(a, b))
		{
			// Update the angular velocities for the wheels
			for (int i = 0; i < (int)driven_wheels.size(); ++i)
			{
				DrivenWheel &dw1 = driven_wheels[i];
				dw1.mWheel->SetAngularVelocity(b(i, 0));
			}

			// Update the engine RPM
			mEngine.SetCurrentRPM(b(engine, 0) * VehicleEngine::cAngularVelocityToRPM);

			// The speeds have been solved
			solved = true;
		}
		else
		{
			JPH_ASSERT(false, "New engine/wheel speeds could not be calculated!");
		}
	}

	if (!solved)
	{
		// Engine not connected to wheels, apply all torque to engine rotation
		mEngine.ApplyTorque(engine_torque, inDeltaTime);
	}

	// Calculate if any of the wheels are slipping, this is used to prevent gear switching
	bool wheels_slipping = false;
	for (const DrivenWheel &w : driven_wheels)
		wheels_slipping |= w.mClutchToWheelTorqueRatio > 0.0f && (!w.mWheel->HasContact() || w.mWheel->mLongitudinalSlip > 0.1f);

	// Only allow shifting up when we're not slipping and we're increasing our RPM.
	// After a jump, we have a very high engine RPM but once we hit the ground the RPM should be decreasing and we don't want to shift up
	// during that time.
	bool can_shift_up = !wheels_slipping && mEngine.GetCurrentRPM() >= old_engine_rpm;

	// Update transmission
	mTransmission.Update(inDeltaTime, mEngine.GetCurrentRPM(), mForwardInput, can_shift_up);
	
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
	String status = StringFormat("Forward: %.1f, Right: %.1f\nBrake: %.1f, HandBrake: %.1f\n"
								 "Gear: %d, Clutch: %.1f\nEngineRPM: %.0f, V: %.1f km/h", 
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

			DebugRenderer::sInstance->DrawText3D(w->GetContactPosition(), StringFormat("W: %.1f, S: %.2f\nSlip: %.2f, FrLateral: %.1f, FrLong: %.1f", (double)w->GetAngularVelocity(), (double)w->GetSuspensionLength(), (double)w->mLongitudinalSlip, (double)w->mCombinedLateralFriction, (double)w->mCombinedLongitudinalFriction), Color::sWhite, 0.1f);
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

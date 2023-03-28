// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(MotorcycleControllerSettings)
{
	JPH_ADD_BASE_CLASS(MotorcycleControllerSettings, VehicleControllerSettings)

	JPH_ADD_ATTRIBUTE(MotorcycleControllerSettings, mMaxLeanAngle)
	JPH_ADD_ATTRIBUTE(MotorcycleControllerSettings, mLeanSpringConstant)
	JPH_ADD_ATTRIBUTE(MotorcycleControllerSettings, mLeanSpringDamping)
}

VehicleController *MotorcycleControllerSettings::ConstructController(VehicleConstraint &inConstraint) const
{
	return new MotorcycleController(*this, inConstraint);
}

void MotorcycleControllerSettings::SaveBinaryState(StreamOut &inStream) const
{
	WheeledVehicleControllerSettings::SaveBinaryState(inStream);
}

void MotorcycleControllerSettings::RestoreBinaryState(StreamIn &inStream)
{
	WheeledVehicleControllerSettings::RestoreBinaryState(inStream);
}

MotorcycleController::MotorcycleController(const MotorcycleControllerSettings &inSettings, VehicleConstraint &inConstraint) :
	WheeledVehicleController(inSettings, inConstraint),
	mMaxLeanAngle(inSettings.mMaxLeanAngle),
	mLeanSpringConstant(inSettings.mLeanSpringConstant),
	mLeanSpringDamping(inSettings.mLeanSpringDamping)
{
}

float MotorcycleController::GetWheelBase() const
{
	float low = FLT_MAX, high = -FLT_MAX;

	for (const Wheel *w : mConstraint.GetWheels())
	{
		const WheelSettings *s = w->GetSettings();

		// Measure distance along the forward axis by looking at the fully extended suspension
		float value = (s->mPosition + s->mDirection * s->mSuspensionMaxLength).Dot(mConstraint.GetLocalForward());

		// Update min and max
		low = min(low, value);
		high = max(high, value);
	}

	return high - low;
}

void MotorcycleController::PreCollide(float inDeltaTime, PhysicsSystem &inPhysicsSystem)
{
	WheeledVehicleController::PreCollide(inDeltaTime, inPhysicsSystem);

	Vec3 gravity = inPhysicsSystem.GetGravity();
	float gravity_len = gravity.Length();
	Vec3 world_up = -gravity / gravity_len;

	Body *body = mConstraint.GetVehicleBody();
	Vec3 forward = body->GetRotation() * mConstraint.GetLocalForward();
	float wheel_base = GetWheelBase();
	float velocity = body->GetLinearVelocity().Dot(forward);
	float velocity_sq = Square(velocity);

	// Calculate max steering angle based on the max lean angle we're willing to take
	// See: https://en.wikipedia.org/wiki/Bicycle_and_motorcycle_dynamics#Leaning
	// LeanAngle = Atan(Velocity^2 / (Gravity * TurnRadius))
	// And: https://en.wikipedia.org/wiki/Turning_radius (we're ignoring the tire width)
	// TurnRadius = WheelBase / Sin(SteerAngle)
	// => SteerAngle = ASin(WheelBase * Tan(LeanAngle) * Gravity / Velocity^2)
	float max_steer_angle = FLT_MAX;
	if (velocity_sq > 1.0e-6f)
		max_steer_angle = ASin(wheel_base * Tan(mMaxLeanAngle) * gravity_len / velocity_sq);

	// Decompose steering into sign and direction
	float steer_strength = abs(mRightInput);
	float steer_sign = -Sign(mRightInput);

	float all_wheel_steer_angle = 0.0f;
	for (Wheel *w_base : mConstraint.GetWheels())
	{
		WheelWV *w = static_cast<WheelWV *>(w_base);

		// Clamp steer angle to max steer angle
		float steer_angle = min(steer_strength * w->GetSettings()->mMaxSteerAngle, max_steer_angle);

		// Store steer angle across all wheels for calculating the lean angle later
		all_wheel_steer_angle = max(all_wheel_steer_angle, steer_angle);

		// Set steering angle
		w->SetSteerAngle(steer_sign * steer_angle);
	}

	// Calculate the desired lean angle
	if (all_wheel_steer_angle < 1.0e-6f)
	{
		// Not steering -> no leaning
		mTargetLean = world_up;
	}
	else
	{
		// Using the formulas above, calculate the resulting lean angle
		float turn_radius = -wheel_base / Sin(steer_sign * all_wheel_steer_angle);
		float lean_angle = ATan(velocity_sq / (gravity_len * turn_radius));
		Vec3 world_right = forward.Cross(world_up).NormalizedOr(world_up.GetNormalizedPerpendicular());
		mTargetLean = world_right * Sin(lean_angle) + world_up * Cos(lean_angle);
	}
}

void MotorcycleController::WarmStartVelocityConstraint(float inWarmStartImpulseRatio)
{
	WheeledVehicleController::WarmStartVelocityConstraint(inWarmStartImpulseRatio);

	mAppliedImpulse *= inWarmStartImpulseRatio;

	// Apply impulse from previous frame
	Body *body = mConstraint.GetVehicleBody();
	Vec3 forward = body->GetRotation() * mConstraint.GetLocalForward();
	body->AddAngularImpulse(mAppliedImpulse * forward);
}

bool MotorcycleController::SolveLongitudinalAndLateralConstraints(float inDeltaTime) 
{
	bool impulse = WheeledVehicleController::SolveLongitudinalAndLateralConstraints(inDeltaTime);

	Body *body = mConstraint.GetVehicleBody();
	Vec3 forward = body->GetRotation() * mConstraint.GetLocalForward();
	Vec3 up = body->GetRotation() * mConstraint.GetLocalUp();

	// Calculate delta to target angle and derivative
	float d_angle = -Sign(mTargetLean.Cross(up).Dot(forward)) * ACos(mTargetLean.Dot(up));
	float ddt_angle = body->GetAngularVelocity().Dot(forward);

	// Calculate impulse to apply to get to target lean angle
	float total_impulse = (mLeanSpringConstant * d_angle - mLeanSpringDamping * ddt_angle) * inDeltaTime;

	// Apply impulse taking into account the impulse we've applied earlier
	float delta_impulse = total_impulse - mAppliedImpulse;
	body->AddAngularImpulse(delta_impulse * forward);
	mAppliedImpulse = total_impulse;

	// Return true if we applied an impulse
	impulse |= delta_impulse != 0.0f;

	return impulse;
}

#ifdef JPH_DEBUG_RENDERER

void MotorcycleController::Draw(DebugRenderer *inRenderer) const 
{
	WheeledVehicleController::Draw(inRenderer);

	// Calculate average impulse on the wheel, this should match with the calculated lean angle but is too jittery to use in practice
	Vec3 impulse = Vec3::sZero();
	for (const Wheel *w : mConstraint.GetWheels())
		if (w->HasContact())
			impulse += w->GetContactNormal() * w->GetSuspensionLambda() + w->GetContactLateral() * w->GetLateralLambda();
	impulse = impulse.NormalizedOr(Vec3::sZero());

	// Draw current and desired lean angle
	Body *body = mConstraint.GetVehicleBody();
	RVec3 center_of_mass = body->GetCenterOfMassPosition();
	Vec3 up = body->GetRotation() * mConstraint.GetLocalUp();
	inRenderer->DrawArrow(center_of_mass, center_of_mass + up, Color::sYellow, 0.1f);
	inRenderer->DrawArrow(center_of_mass, center_of_mass + mTargetLean, Color::sOrange, 0.1f);
	inRenderer->DrawArrow(center_of_mass, center_of_mass + impulse, Color::sRed, 0.1f);
}

#endif // JPH_DEBUG_RENDERER

void MotorcycleController::SaveState(StateRecorder &inStream) const
{
	WheeledVehicleController::SaveState(inStream);
}

void MotorcycleController::RestoreState(StateRecorder &inStream)
{
	WheeledVehicleController::RestoreState(inStream);
}

JPH_NAMESPACE_END

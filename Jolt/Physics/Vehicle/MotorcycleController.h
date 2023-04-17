// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>

JPH_NAMESPACE_BEGIN

/// Settings of a two wheeled motorcycle (adds a spring to balance the motorcycle)
/// Note: The motor cycle controller is still in development and may need a lot of tweaks/hacks to work properly!
class JPH_EXPORT MotorcycleControllerSettings : public WheeledVehicleControllerSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(JPH_EXPORT, MotorcycleControllerSettings)

	// See: VehicleControllerSettings
	virtual VehicleController *	ConstructController(VehicleConstraint &inConstraint) const override;
	virtual void				SaveBinaryState(StreamOut &inStream) const override;
	virtual void				RestoreBinaryState(StreamIn &inStream) override;

	/// How far we're willing to make the bike lean over in turns (in radians)
	float						mMaxLeanAngle = DegreesToRadians(45.0f);

	/// Spring constant for the lean spring
	float						mLeanSpringConstant = 5000.0f;

	/// Spring damping constant for the lean spring
	float						mLeanSpringDamping = 1000.0f;

	/// How much to smooth the lean angle (0 = no smoothing, 1 = lean angle never changes)
	/// Note that this is frame rate dependent because the formula is: smoothing_factor * previous + (1 - smoothing_factor) * current
	float						mLeanSmoothingFactor = 0.8f;
};

/// Runtime controller class
class JPH_EXPORT MotorcycleController : public WheeledVehicleController
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
								MotorcycleController(const MotorcycleControllerSettings &inSettings, VehicleConstraint &inConstraint);

	/// Get the distance between the front and back wheels
	float						GetWheelBase() const;

protected:
	// See: VehicleController
	virtual void				PreCollide(float inDeltaTime, PhysicsSystem &inPhysicsSystem) override;
	virtual bool				SolveLongitudinalAndLateralConstraints(float inDeltaTime) override;
	virtual void				SaveState(StateRecorder& inStream) const override;
	virtual void				RestoreState(StateRecorder& inStream) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				Draw(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER

	// Configuration properties
	float						mMaxLeanAngle;
	float						mLeanSpringConstant;
	float						mLeanSpringDamping;
	float						mLeanSmoothingFactor;

	// Run-time calculated target lean vector
	Vec3						mTargetLean = Vec3::sZero();

	// Run-time total angular impulse applied to turn the cycle towards the target lean angle
	float						mAppliedImpulse = 0.0f;
};

JPH_NAMESPACE_END

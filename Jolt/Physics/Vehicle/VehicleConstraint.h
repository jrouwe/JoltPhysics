// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/Constraint.h>
#include <Jolt/Physics/PhysicsStepListener.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AngleConstraintPart.h>
#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Vehicle/VehicleAntiRollBar.h>
#include <Jolt/Physics/Vehicle/Wheel.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
class VehicleController;
class VehicleControllerSettings;

/// Configuration for constraint that simulates a wheeled vehicle.
/// 
/// The properties in this constraint are largely based on "Car Physics for Games" by Marco Monster.
/// See: https://www.asawicki.info/Mirror/Car%20Physics%20for%20Games/Car%20Physics%20for%20Games.html
class VehicleConstraintSettings : public ConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(VehicleConstraintSettings)

	/// Saves the contents of the constraint settings in binary form to inStream.
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	Vec3						mUp { 0, 1, 0 };							///< Vector indicating the up direction of the vehicle (in local space to the body)
	Vec3						mForward { 0, 0, 1 };						///< Vector indicating forward direction of the vehicle (in local space to the body)
	float						mMaxPitchRollAngle = JPH_PI;				///< Defines the maximum pitch/roll angle (rad), can be used to avoid the car from getting upside down. The vehicle up direction will stay within a cone centered around the up axis with half top angle mMaxPitchRollAngle, set to pi to turn off.
	vector<Ref<WheelSettings>>	mWheels;									///< List of wheels and their properties
	vector<VehicleAntiRollBar>	mAntiRollBars;								///< List of anti rollbars and their properties
	Ref<VehicleControllerSettings> mController;								///< Defines how the vehicle can accelerate / decellerate

protected:
	/// This function should not be called directly, it is used by sRestoreFromBinaryState.
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// Constraint that simulates a vehicle
/// Note: Don't forget to register the constraint as a StepListener with the PhysicsSystem!
class VehicleConstraint : public Constraint, public PhysicsStepListener
{
public:
	/// Constructor / destructor
								VehicleConstraint(Body &inVehicleBody, const VehicleConstraintSettings &inSettings);
	virtual						~VehicleConstraint() override;

	/// Get the type of a constraint
	virtual EConstraintSubType	GetSubType() const override					{ return EConstraintSubType::Vehicle; }

	/// Defines the maximum pitch/roll angle (rad), can be used to avoid the car from getting upside down. The vehicle up direction will stay within a cone centered around the up axis with half top angle mMaxPitchRollAngle, set to pi to turn off.
	void						SetMaxPitchRollAngle(float inMaxPitchRollAngle) { mCosMaxPitchRollAngle = cos(inMaxPitchRollAngle); }
	
	/// Set the interface that tests collision between wheel and ground
	void						SetVehicleCollisionTester(const VehicleCollisionTester *inTester) { mVehicleCollisionTester = inTester; }

	/// Get the local space forward vector of the vehicle
	Vec3						GetLocalForward() const						{ return mForward; }

	/// Get the local space up vector of the vehicle
	Vec3						GetLocalUp() const							{ return mUp; }

	/// Access to the vehicle body
	Body *						GetVehicleBody() const						{ return mBody; }

	/// Access to the vehicle controller interface (determines acceleration / decelleration)
	const VehicleController *	GetController() const						{ return mController; }

	/// Access to the vehicle controller interface (determines acceleration / decelleration)
	VehicleController *			GetController()								{ return mController; }

	/// Get the state of the wheels
	const Wheels &				GetWheels() const							{ return mWheels; }

	/// Get the state of a wheels (writable interface, allows you to make changes to the configuration which will take effect the next time step)
	Wheels &					GetWheels()									{ return mWheels; }

	/// Get the transform of a wheel in local space to the vehicle body, returns a matrix that transforms a cylinder aligned with the Y axis in body space (not COM space)
	/// @param inWheelIndex Index of the wheel to fetch
	/// @param inWheelRight Unit vector that indicates right in model space of the wheel (so if you only have 1 wheel model, you probably want to specify the opposite direction for the left and right wheels)
	/// @param inWheelUp Unit vector that indicates up in model space of the wheel
	Mat44						GetWheelLocalTransform(uint inWheelIndex, Vec3Arg inWheelRight, Vec3Arg inWheelUp) const;

	/// Get the transform of a wheel in world space, returns a matrix that transforms a cylinder aligned with the Y axis in world space
	/// @param inWheelIndex Index of the wheel to fetch
	/// @param inWheelRight Unit vector that indicates right in model space of the wheel (so if you only have 1 wheel model, you probably want to specify the opposite direction for the left and right wheels)
	/// @param inWheelUp Unit vector that indicates up in model space of the wheel
	Mat44						GetWheelWorldTransform(uint inWheelIndex, Vec3Arg inWheelRight, Vec3Arg inWheelUp) const;

	// Generic interface of a constraint
	virtual bool				IsActive() const override					{ return mIsActive && Constraint::IsActive(); }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
	virtual void				BuildIslands(uint32 inConstraintIndex, IslandBuilder &ioBuilder, BodyManager &inBodyManager) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
	virtual void				DrawConstraintLimits(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual Ref<ConstraintSettings> GetConstraintSettings() const override;

private:
	// See: PhysicsStepListener
	virtual void				OnStep(float inDeltaTime, PhysicsSystem &inPhysicsSystem) override;

	// Calculate the contact positions of the wheel in world space, relative to the center of mass of both bodies
	void						CalculateWheelContactPoint(Mat44Arg inBodyTransform, const Wheel &inWheel, Vec3 &outR1PlusU, Vec3 &outR2) const;

	// Calculate the constraint properties for mPitchRollPart
	void						CalculatePitchRollConstraintProperties(float inDeltaTime, Mat44Arg inBodyTransform);

	// Simluation information
	Body *						mBody;										///< Body of the vehicle
	Vec3						mForward;									///< Local space forward vector for the vehicle
	Vec3						mUp;										///< Local space up vector for the vehicle
	Wheels						mWheels;									///< Wheel states of the vehicle
	vector<VehicleAntiRollBar>	mAntiRollBars;								///< Anti rollbars of the vehicle
	VehicleController *			mController;								///< Controls the acceleration / declerration of the vehicle
	bool						mIsActive = false;							///< If this constraint is active

	// Prevent vehicle from toppling over
	float						mCosMaxPitchRollAngle;						///< Cos of the max pitch/roll angle
	float						mCosPitchRollAngle;							///< Cos of the current pitch/roll angle
	Vec3						mPitchRollRotationAxis { 0, 1, 0 };			///< Current axis along which to apply torque to prevent the car from toppling over
	AngleConstraintPart			mPitchRollPart;								///< Constraint part that prevents the car from toppling over

	// Interfaces
	RefConst<VehicleCollisionTester> mVehicleCollisionTester;				///< Class that performs testing of collision for the wheels
};

JPH_NAMESPACE_END

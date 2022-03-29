// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AxisConstraintPart.h>
#include <Jolt/ObjectStream/SerializableObject.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

class VehicleConstraint;

/// Base class for wheel settings, each VehicleController can implement a derived class of this
class WheelSettings : public SerializableObject, public RefTarget<WheelSettings>, public NonCopyable
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(WheelSettings)

	/// Saves the contents in binary form to inStream.
	virtual void			SaveBinaryState(StreamOut &inStream) const;

	/// Restores the contents in binary form to inStream.
	virtual void			RestoreBinaryState(StreamIn &inStream);

	Vec3					mPosition { 0, 0, 0 };						///< Attachment point of wheel suspension in local space of the body
	Vec3					mDirection { 0, -1, 0 };					///< Direction of the suspension in local space of the body
	float					mSuspensionMinLength = 0.3f;				///< How long the suspension is in max raised position relative to the attachment point (m)
	float					mSuspensionMaxLength = 0.5f;				///< How long the suspension is in max droop position relative to the attachment point (m)
	float					mSuspensionPreloadLength = 0.0f;			///< The natural length (m) of the suspension spring is defined as mSuspensionMaxLength + mSuspensionPreloadLength. Can be used to preload the suspension as the spring is compressed by mSuspensionPreloadLength when the suspension is in max droop position. Note that this means when the vehicle touches the ground there is a discontinuity so it will also make the vehicle more bouncy as we're updating with discrete time steps.
	float					mSuspensionFrequency = 1.5f;				///< Natural frequency of the suspension spring (Hz)
	float					mSuspensionDamping = 0.5f;					///< Damping factor of the suspension spring (0 = no damping, 1 = critical damping)
	float					mRadius = 0.3f;								///< Radius of the wheel (m)
	float					mWidth = 0.1f;								///< Width of the wheel (m)
};

/// Base class for runtime data for a wheel, each VehicleController can implement a derived class of this
class Wheel
{
public:
	/// Constructor / destructor
	explicit				Wheel(const WheelSettings &inSettings);
	virtual					~Wheel() = default;

	/// Get settings for the wheel
	const WheelSettings *	GetSettings() const							{ return mSettings; }

	/// Get the angular velocity (rad/s) for this wheel, note that positive means the wheel is rotating such that the car moves forward
	float					GetAngularVelocity() const					{ return mAngularVelocity; }

	/// Update the angular velocity (rad/s)
	void					SetAngularVelocity(float inVel)				{ mAngularVelocity = inVel; }

	/// Get the current rotation angle of the wheel in radians [0, 2 pi]
	float					GetRotationAngle() const					{ return mAngle; }

	/// Set the current rotation angle of the wheel in radians [0, 2 pi]
	void					SetRotationAngle(float inAngle)				{ mAngle = inAngle; }

	/// Get the current steer angle of the wheel in radians [-pi, pi], positive is to the left
	float					GetSteerAngle() const						{ return mSteerAngle; }

	/// Set the current steer angle of the wheel in radians [-pi, pi]
	void					SetSteerAngle(float inAngle)				{ mSteerAngle = inAngle; }

	/// Returns true if the wheel is touching an object
	bool					HasContact() const							{ return mContactBody != nullptr; }

	/// Returns the body ID of the body that this wheel is touching
	BodyID					GetContactBodyID() const					{ return mContactBodyID; }

	/// Returns the sub shape ID where we're contacting the body
	SubShapeID				GetContactSubShapeID() const				{ return mContactSubShapeID; }

	/// Returns the current contact position in world space (note by the time you call this the vehicle has moved)
	Vec3					GetContactPosition() const					{ JPH_ASSERT(mContactBody != nullptr); return mContactPosition; }

	/// Velocity of the contact point (m / s, not relative to the wheel but in world space)
	Vec3					GetContactPointVelocity() const				{ JPH_ASSERT(mContactBody != nullptr); return mContactPointVelocity; }

	/// Returns the current contact mormal in world space (note by the time you call this the vehicle has moved)
	Vec3					GetContactNormal() const					{ JPH_ASSERT(mContactBody != nullptr); return mContactNormal; }

	/// Returns longitudinal direction (direction along the wheel relative to floor) in world space (note by the time you call this the vehicle has moved)
	Vec3					GetContactLongitudinal() const				{ JPH_ASSERT(mContactBody != nullptr); return mContactLongitudinal; }

	/// Returns lateral direction (sideways direction) in world space (note by the time you call this the vehicle has moved)
	Vec3					GetContactLateral() const					{ JPH_ASSERT(mContactBody != nullptr); return mContactLateral; }

	/// Get the length of the suspension for a wheel (m) relative to the suspension attachment point (hard point)
	float					GetSuspensionLength() const					{ return mContactLength - mSettings->mRadius; }

	/// Check if the suspension hit its upper limit
	bool					HasHitHardPoint() const						{ return mSuspensionMaxUpPart.IsActive(); }

	/// Get the total impulse (N s) that was applied by the suspension
	float					GetSuspensionLambda() const					{ return mSuspensionPart.GetTotalLambda() + mSuspensionMaxUpPart.GetTotalLambda(); }

	/// Get total impulse (N s) applied along the forward direction of the wheel
	float					GetLongitudinalLambda() const				{ return mLongitudinalPart.GetTotalLambda(); }

	/// Get total impulse (N s) applied along the sideways direction of the wheel
	float					GetLateralLambda() const					{ return mLateralPart.GetTotalLambda(); }

	/// Internal function that should only be called by the controller. Used to apply impulses in the forward direction of the vehicle.
	bool					SolveLongitudinalConstraintPart(const VehicleConstraint &inConstraint, float inMinImpulse, float inMaxImpulse);

	/// Internal function that should only be called by the controller. Used to apply impulses in the sideways direction of the vehicle.
	bool					SolveLateralConstraintPart(const VehicleConstraint &inConstraint, float inMinImpulse, float inMaxImpulse);

protected:
	friend class VehicleConstraint;

	RefConst<WheelSettings>	mSettings;									///< Configuration settings for this wheel
	BodyID					mContactBodyID;								///< ID of body for ground
	SubShapeID				mContactSubShapeID;							///< Sub shape ID for ground
	Body *					mContactBody = nullptr;						///< Body for ground
	float					mContactLength;								///< Length between attachment point and ground
	Vec3					mContactPosition;							///< Position of the contact point between wheel and ground
	Vec3					mContactPointVelocity;						///< Velocity of the contact point (m / s, not relative to the wheel but in world space)
	Vec3					mContactNormal;								///< Normal of the contact point between wheel and ground
	Vec3					mContactLongitudinal;						///< Vector perpendicular to normal in the forward direction
	Vec3					mContactLateral;							///< Vector perpendicular to normal and longitudinal direction in the right direction
	Vec3					mWSDirection;								///< Suspension spring direction in world space
	float					mAntiRollBarImpulse = 0.0f;					///< Amount of impulse applied to the suspension from the anti-rollbars

	float					mSteerAngle = 0.0f;							///< Rotation around the suspension direction, positive is to the left
	float					mAngularVelocity = 0.0f;					///< Rotation speed of wheel, positive when the wheels cause the vehicle to move forwards (rad/s)
	float					mAngle = 0.0f;								///< Current rotation of the wheel (rad, [0, 2 pi])

	AxisConstraintPart		mSuspensionPart;							///< Controls movement up/down
	AxisConstraintPart		mSuspensionMaxUpPart;						///< Adds a hard limit when reaching the minimal suspension length
	AxisConstraintPart		mLongitudinalPart;							///< Controls movement forward/backward
	AxisConstraintPart		mLateralPart;								///< Controls movement sideways (slip)
};

using Wheels = vector<Wheel *>;

JPH_NAMESPACE_END

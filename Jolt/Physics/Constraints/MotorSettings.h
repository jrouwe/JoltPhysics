// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/Reference.h>
#include <ObjectStream/SerializableObject.h>

namespace JPH {

class StreamIn;
class StreamOut;

enum class EMotorState
{
	Off,																///< Motor is off
	Velocity,															///< Motor will drive to target velocity
	Position															///< Motor will drive to target position
};

/// Class that contains the settings for a constraint motor
class MotorSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(MotorSettings)

	/// Constructor
							MotorSettings() = default;
							MotorSettings(const MotorSettings &inRHS) = default;
							MotorSettings(float inFrequency, float inDamping) : mFrequency(inFrequency), mDamping(inDamping) { JPH_ASSERT(IsValid()); }
							MotorSettings(float inFrequency, float inDamping, float inForceLimit, float inTorqueLimit) : mFrequency(inFrequency), mDamping(inDamping), mMinForceLimit(-inForceLimit), mMaxForceLimit(inForceLimit), mMinTorqueLimit(-inTorqueLimit), mMaxTorqueLimit(inTorqueLimit) { JPH_ASSERT(IsValid()); }

	/// Set asymmetric force limits
	void					SetForceLimits(float inMin, float inMax)	{ JPH_ASSERT(inMin <= inMax); mMinForceLimit = inMin; mMaxForceLimit = inMax; }

	/// Set asymmetric torque limits
	void					SetTorqueLimits(float inMin, float inMax)	{ JPH_ASSERT(inMin <= inMax); mMinTorqueLimit = inMin; mMaxTorqueLimit = inMax; }

	/// Set symmetric force limits
	void					SetForceLimit(float inLimit)				{ mMinForceLimit = -inLimit; mMaxForceLimit = inLimit; }

	/// Set symmetric torque limits
	void					SetTorqueLimit(float inLimit)				{ mMinTorqueLimit = -inLimit; mMaxTorqueLimit = inLimit; }

	/// Check if settings are valid
	bool					IsValid() const								{ return mFrequency >= 0.0f && mDamping >= 0.0f && mMinForceLimit <= mMaxForceLimit && mMinTorqueLimit <= mMaxTorqueLimit; }

	/// Saves the contents of the motor settings in binary form to inStream.
	void					SaveBinaryState(StreamOut &inStream) const;

	/// Restores contents from the binary stream inStream.
	void					RestoreBinaryState(StreamIn &inStream);

	// Settings
	float					mFrequency = 2.0f;							///< Oscillation frequency when solving position target (Hz)
	float					mDamping = 1.0f;							///< Damping when solving position target (0 = minimal damping, 1 = critical damping)
	float					mMinForceLimit = -FLT_MAX;					///< Minimum force to apply in case of a linear constraint (N)
	float					mMaxForceLimit = FLT_MAX;					///< Maximum force to apply in case of a linear constraint (N)
	float					mMinTorqueLimit = -FLT_MAX;					///< Minimum torque to apply in case of a angular constraint (N m)
	float					mMaxTorqueLimit = FLT_MAX;					///< Maximum torque to apply in case of a angular constraint (N m)
};

} // JPH
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <ObjectStream/SerializableObject.h>
#include <Core/LinearCurve.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>
#include <Physics/StateRecorder.h>

namespace JPH {

/// Generic properties for a vehicle engine
class VehicleEngineSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(VehicleEngineSettings)

	/// Constructor
							VehicleEngineSettings();

	/// Saves the contents in binary form to inStream.
	void					SaveBinaryState(StreamOut &inStream) const;

	/// Restores the contents in binary form to inStream.
	void					RestoreBinaryState(StreamIn &inStream);

	float					mMaxTorque = 500.0f;						///< Max amount of torque (Nm) that the engine can deliver
	float					mMinRPM = 1000.0f;							///< Min amount of revolutions per minute (rpm) the engine can produce without stalling
	float					mMaxRPM = 6000.0f;							///< Max amount of revolutions per minute (rpm) the engine can generate
	LinearCurve				mNormalizedTorque;							///< Curve that describes a ratio of the max torque the engine can produce vs the fraction of the max RPM of the engine
};

/// Runtime data for engine
class VehicleEngine : public VehicleEngineSettings
{
public:
	/// Current rotation speed of engine in rounds per minute
	float					GetCurrentRPM() const						{ return mCurrentRPM; }

	/// Update rotation speed of engine in rounds per minute
	void					SetCurrentRPM(float inRPM)					{ mCurrentRPM = inRPM; }

	/// Saving state for replay
	void					SaveState(StateRecorder &inStream) const;
	void					RestoreState(StateRecorder &inStream);

private:
	float					mCurrentRPM = 1000.0f;						///< Current rotation speed of engine in rounds per minute
};

} // JPH
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Vehicle/VehicleEngine.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(VehicleEngineSettings)
{
	JPH_ADD_ATTRIBUTE(VehicleEngineSettings, mMaxTorque)
	JPH_ADD_ATTRIBUTE(VehicleEngineSettings, mMinRPM)
	JPH_ADD_ATTRIBUTE(VehicleEngineSettings, mMaxRPM)
	JPH_ADD_ATTRIBUTE(VehicleEngineSettings, mNormalizedTorque)
}

VehicleEngineSettings::VehicleEngineSettings()
{
	mNormalizedTorque.Reserve(3);
	mNormalizedTorque.AddPoint(0.0f, 0.8f);
	mNormalizedTorque.AddPoint(0.66f, 1.0f);
	mNormalizedTorque.AddPoint(1.0f, 0.8f);
}

void VehicleEngineSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mMaxTorque);
	inStream.Write(mMinRPM);
	inStream.Write(mMaxRPM);
	mNormalizedTorque.SaveBinaryState(inStream);
}

void VehicleEngineSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mMaxTorque);
	inStream.Read(mMinRPM);
	inStream.Read(mMaxRPM);
	mNormalizedTorque.RestoreBinaryState(inStream);
}

void VehicleEngine::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mCurrentRPM);
}

void VehicleEngine::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mCurrentRPM);
}

} // JPH
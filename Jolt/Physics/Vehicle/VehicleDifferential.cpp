// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/VehicleDifferential.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(VehicleDifferentialSettings)
{
	JPH_ADD_ATTRIBUTE(VehicleDifferentialSettings, mLeftWheel)
	JPH_ADD_ATTRIBUTE(VehicleDifferentialSettings, mRightWheel)
	JPH_ADD_ATTRIBUTE(VehicleDifferentialSettings, mDifferentialRatio)
	JPH_ADD_ATTRIBUTE(VehicleDifferentialSettings, mLeftRightSplit)
	JPH_ADD_ATTRIBUTE(VehicleDifferentialSettings, mEngineTorqueRatio)
}

void VehicleDifferentialSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mLeftWheel);
	inStream.Write(mRightWheel);
	inStream.Write(mDifferentialRatio);
	inStream.Write(mLeftRightSplit);
	inStream.Write(mEngineTorqueRatio);
}

void VehicleDifferentialSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mLeftWheel);
	inStream.Read(mRightWheel);
	inStream.Read(mDifferentialRatio);
	inStream.Read(mLeftRightSplit);
	inStream.Read(mEngineTorqueRatio);
}

JPH_NAMESPACE_END

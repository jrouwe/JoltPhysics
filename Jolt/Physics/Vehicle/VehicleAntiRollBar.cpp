// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Vehicle/VehicleAntiRollBar.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(VehicleAntiRollBar)
{
	JPH_ADD_ATTRIBUTE(VehicleAntiRollBar, mLeftWheel)
	JPH_ADD_ATTRIBUTE(VehicleAntiRollBar, mRightWheel)
	JPH_ADD_ATTRIBUTE(VehicleAntiRollBar, mStiffness)
}

void VehicleAntiRollBar::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mLeftWheel);
	inStream.Write(mRightWheel);
	inStream.Write(mStiffness);
}

void VehicleAntiRollBar::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mLeftWheel);
	inStream.Read(mRightWheel);
	inStream.Read(mStiffness);
}

} // JPH
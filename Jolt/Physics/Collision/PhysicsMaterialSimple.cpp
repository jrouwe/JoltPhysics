// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/PhysicsMaterialSimple.h>
#include <ObjectStream/TypeDeclarations.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(PhysicsMaterialSimple)
{
	JPH_ADD_BASE_CLASS(PhysicsMaterialSimple, PhysicsMaterial)

	JPH_ADD_ATTRIBUTE(PhysicsMaterialSimple, mDebugName)
	JPH_ADD_ATTRIBUTE(PhysicsMaterialSimple, mDebugColor)
}

void PhysicsMaterialSimple::SaveBinaryState(StreamOut &inStream) const
{
	PhysicsMaterial::SaveBinaryState(inStream);

	inStream.Write(mDebugName);
	inStream.Write(mDebugColor);
}

void PhysicsMaterialSimple::RestoreBinaryState(StreamIn &inStream)
{
	PhysicsMaterial::RestoreBinaryState(inStream);

	inStream.Read(mDebugName);
	inStream.Read(mDebugColor);
}

} // JPH
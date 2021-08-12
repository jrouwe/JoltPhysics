// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/PhysicsMaterial.h>
#include <Physics/Collision/PhysicsMaterialSimple.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>
#include <Core/Factory.h>

namespace JPH {

RefConst<PhysicsMaterial> PhysicsMaterial::sDefault = new PhysicsMaterialSimple("Default", Color::sGrey);

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(PhysicsMaterial)
{
	JPH_ADD_BASE_CLASS(PhysicsMaterial, SerializableObject)
}

void PhysicsMaterial::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(GetRTTI()->GetHash());
}

void PhysicsMaterial::RestoreBinaryState(StreamIn &inStream)
{
}

PhysicsMaterial::PhysicsMaterialResult PhysicsMaterial::sRestoreFromBinaryState(StreamIn &inStream)
{
	PhysicsMaterialResult result;

	// Read the type of the material
	uint32 hash;
	inStream.Read(hash);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read type hash");
		return result;
	}

	// Get the RTTI for the material
	const RTTI *rtti = Factory::sInstance.Find(hash);
	if (rtti == nullptr)
	{
		result.SetError("Failed to create instance of material");
		return result;
	}

	// Construct and read the data of the material
	Ref<PhysicsMaterial> material = reinterpret_cast<PhysicsMaterial *>(rtti->CreateObject());
	material->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore material");
		return result;
	}

	result.Set(material);
	return result;
}

} // JPH
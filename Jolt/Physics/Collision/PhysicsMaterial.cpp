// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>

JPH_NAMESPACE_BEGIN

RefConst<PhysicsMaterial> PhysicsMaterial::sDefault;

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
	// RTTI hash is read in sRestoreFromBinaryState
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
	const RTTI *rtti = Factory::sInstance->Find(hash);
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

void PhysicsMaterial::sSaveMaterialList(StreamOut &inStream, const PhysicsMaterialList &inMaterials, MaterialToIDMap &ioMaterialMap)
{
	// Write the materials
	inStream.Write(inMaterials.size());
	for (const PhysicsMaterial *mat : inMaterials)
	{
		if (mat == nullptr)
		{
			// Write nullptr
			inStream.Write(~uint32(0));
		}
		else
		{
			MaterialToIDMap::const_iterator material_id = ioMaterialMap.find(mat);
			if (material_id == ioMaterialMap.end())
			{
				// New material, write the ID
				uint32 new_material_id = (uint32)ioMaterialMap.size();
				ioMaterialMap[mat] = new_material_id;
				inStream.Write(new_material_id);

				// Write the material
				mat->SaveBinaryState(inStream);
			}
			else
			{
				// Known material, just write the ID
				inStream.Write(material_id->second);
			}
		}
	}
}

PhysicsMaterial::MLResult PhysicsMaterial::sRestoreMaterialList(StreamIn &inStream, IDToMaterialMap &ioMaterialMap)
{
	MLResult result;

	size_t len;
	inStream.Read(len);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read stream");
		return result;
	}

	PhysicsMaterialList materials;
	materials.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		Ref<PhysicsMaterial> material;

		uint32 material_id;
		inStream.Read(material_id);

		// Check nullptr
		if (material_id != ~uint32(0))
		{
			if (material_id >= ioMaterialMap.size())
			{
				// New material, restore material
				PhysicsMaterial::PhysicsMaterialResult material_result = PhysicsMaterial::sRestoreFromBinaryState(inStream);
				if (material_result.HasError())
				{
					result.SetError(material_result.GetError());
					return result;
				}
				material = material_result.Get();
				JPH_ASSERT(material_id == ioMaterialMap.size());
				ioMaterialMap.push_back(material);
			}
			else
			{
				// Existing material
				material = ioMaterialMap[material_id];
			}
		}

		materials.push_back(material);
	}

	result.Set(materials);
	return result;
}

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodySharedSettings::Vertex)
{
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Vertex, mPosition)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Vertex, mVelocity)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Vertex, mInvMass)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodySharedSettings::Face)
{
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Face, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Face, mMaterialIndex)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodySharedSettings::Edge)
{
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Edge, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Edge, mRestLength)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Edge, mCompliance)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodySharedSettings::Volume)
{
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Volume, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Volume, mSixRestVolume)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings::Volume, mCompliance)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodySharedSettings)
{
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings, mVertices)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings, mFaces)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings, mEdgeConstraints)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings, mVolumeConstraints)
	JPH_ADD_ATTRIBUTE(SoftBodySharedSettings, mMaterials)
}

void SoftBodySharedSettings::CalculateEdgeLengths()
{
	for (Edge &e : mEdgeConstraints)
	{
		e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
		JPH_ASSERT(e.mRestLength > 0.0f);
	}
}

void SoftBodySharedSettings::CalculateVolumeConstraintVolumes()
{
	for (Volume &v : mVolumeConstraints)
	{
		Vec3 x1(mVertices[v.mVertex[0]].mPosition);
		Vec3 x2(mVertices[v.mVertex[1]].mPosition);
		Vec3 x3(mVertices[v.mVertex[2]].mPosition);
		Vec3 x4(mVertices[v.mVertex[3]].mPosition);

		Vec3 x1x2 = x2 - x1;
		Vec3 x1x3 = x3 - x1;
		Vec3 x1x4 = x4 - x1;

		v.mSixRestVolume = abs(x1x2.Cross(x1x3).Dot(x1x4));
	}
}

void SoftBodySharedSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mVertices);
	inStream.Write(mFaces);
	inStream.Write(mEdgeConstraints);
	inStream.Write(mVolumeConstraints);
}

void SoftBodySharedSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mVertices);
	inStream.Read(mFaces);
	inStream.Read(mEdgeConstraints);
	inStream.Read(mVolumeConstraints);
}

void SoftBodySharedSettings::SaveWithMaterials(StreamOut &inStream, SharedSettingsToIDMap &ioSettingsMap, MaterialToIDMap &ioMaterialMap) const
{
	SharedSettingsToIDMap::const_iterator settings_iter = ioSettingsMap.find(this);
	if (settings_iter == ioSettingsMap.end())
	{
		// Write settings ID
		uint32 settings_id = (uint32)ioSettingsMap.size();
		ioSettingsMap[this] = settings_id;
		inStream.Write(settings_id);

		// Write the settings
		SaveBinaryState(inStream);

		// Write materials
		StreamUtils::SaveObjectArray(inStream, mMaterials, &ioMaterialMap);
	}
	else
	{
		// Known settings, just write the ID
		inStream.Write(settings_iter->second);
	}
}

SoftBodySharedSettings::SettingsResult SoftBodySharedSettings::sRestoreWithMaterials(StreamIn &inStream, IDToSharedSettingsMap &ioSettingsMap, IDToMaterialMap &ioMaterialMap)
{
	SettingsResult result;

	// Read settings id
	uint32 settings_id;
	inStream.Read(settings_id);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read settings id");
		return result;
	}

	// Check nullptr settings
	if (settings_id == ~uint32(0))
	{
		result.Set(nullptr);
		return result;
	}

	// Check if we already read this settings
	if (settings_id < ioSettingsMap.size())
	{
		result.Set(ioSettingsMap[settings_id]);
		return result;
	}

	// Create new object
	Ref<SoftBodySharedSettings> settings = new SoftBodySharedSettings;

	// Read state
	settings->RestoreBinaryState(inStream);

	// Read materials
	Result mlresult = StreamUtils::RestoreObjectArray<PhysicsMaterialList>(inStream, ioMaterialMap);
	if (mlresult.HasError())
	{
		result.SetError(mlresult.GetError());
		return result;
	}
	settings->mMaterials = mlresult.Get();

	// Add the settings to the map
	ioSettingsMap.push_back(settings);

	result.Set(settings);
	return result;
}

JPH_NAMESPACE_END

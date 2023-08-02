// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodyParticleSettings.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyParticleSettings::Vertex)
{
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Vertex, mPosition)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Vertex, mVelocity)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Vertex, mInvMass)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyParticleSettings::Face)
{
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Face, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Face, mMaterialIndex)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyParticleSettings::Edge)
{
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Edge, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Edge, mRestLength)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Edge, mCompliance)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyParticleSettings::Volume)
{
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Volume, mVertex)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Volume, mSixRestVolume)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings::Volume, mCompliance)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyParticleSettings)
{
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings, mVertices)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings, mFaces)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings, mEdgeConstraints)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings, mVolumeConstraints)
	JPH_ADD_ATTRIBUTE(SoftBodyParticleSettings, mMaterials)
}

void SoftBodyParticleSettings::CalculateEdgeLengths()
{
	for (Edge &e : mEdgeConstraints)
	{
		e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
		JPH_ASSERT(e.mRestLength > 0.0f);
	}
}

void SoftBodyParticleSettings::CalculateVolumeConstraintVolumes()
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

void SoftBodyParticleSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mVertices);
	inStream.Write(mFaces);
	inStream.Write(mEdgeConstraints);
	inStream.Write(mVolumeConstraints);
}

void SoftBodyParticleSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mVertices);
	inStream.Read(mFaces);
	inStream.Read(mEdgeConstraints);
	inStream.Read(mVolumeConstraints);
}

JPH_NAMESPACE_END

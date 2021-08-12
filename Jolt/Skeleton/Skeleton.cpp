// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Skeleton/Skeleton.h>
#include <ObjectStream/TypeDeclarations.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(Skeleton::Joint)
{
	JPH_ADD_ATTRIBUTE(Joint, mName)
	JPH_ADD_ATTRIBUTE(Joint, mParentName)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(Skeleton)
{
	JPH_ADD_ATTRIBUTE(Skeleton, mJoints)
}

int Skeleton::GetJointIndex(const string &inName) const
{
	for (int i = 0; i < (int)mJoints.size(); ++i)
		if (mJoints[i].mName == inName)
			return i;

	return -1;
}

void Skeleton::CalculateParentJointIndices()
{
	for (Joint &j : mJoints)
		j.mParentJointIndex = GetJointIndex(j.mParentName);
}

void Skeleton::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write((uint32)mJoints.size());
	for (const Joint &j : mJoints)
	{
		inStream.Write(j.mName);
		inStream.Write(j.mParentJointIndex);
		inStream.Write(j.mParentName);
	}
}

Skeleton::SkeletonResult Skeleton::sRestoreFromBinaryState(StreamIn &inStream)
{
	Ref<Skeleton> skeleton = new Skeleton;

	uint32 len = 0;
	inStream.Read(len);
	skeleton->mJoints.resize(len);
	for (Joint &j : skeleton->mJoints)
	{
		inStream.Read(j.mName);
		inStream.Read(j.mParentJointIndex);
		inStream.Read(j.mParentName);
	}

	SkeletonResult result;
	if (inStream.IsEOF() || inStream.IsFailed())
		result.SetError("Failed to read skeleton from stream");
	else
		result.Set(skeleton);
	return result;
}

} // JPH
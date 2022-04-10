// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Skeleton/SkeletonPose.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

void SkeletonPose::SetSkeleton(const Skeleton *inSkeleton)
{
	mSkeleton = inSkeleton;

	mJoints.resize(mSkeleton->GetJointCount());
	mJointMatrices.resize(mSkeleton->GetJointCount());
}

void SkeletonPose::CalculateJointMatrices()
{
	for (int i = 0; i < (int)mJoints.size(); ++i)
	{
		mJointMatrices[i] = mJoints[i].ToMatrix();

		int parent = mSkeleton->GetJoint(i).mParentJointIndex;
		if (parent >= 0)
		{
			JPH_ASSERT(parent < i, "Bones must be ordered: parents first");
			mJointMatrices[i] = mJointMatrices[parent] * mJointMatrices[i];
		}
	}
}

#ifdef JPH_DEBUG_RENDERER
void SkeletonPose::Draw(const DrawSettings &inDrawSettings, DebugRenderer *inRenderer) const
{
	const Skeleton::JointVector &joints = mSkeleton->GetJoints();

	for (int b = 0; b < mSkeleton->GetJointCount(); ++b)
	{
		if (inDrawSettings.mDrawJoints)
		{
			int parent = joints[b].mParentJointIndex;
			if (parent >= 0)
				inRenderer->DrawLine(mJointMatrices[parent].GetTranslation(), mJointMatrices[b].GetTranslation(), Color::sGreen);
		}

		if (inDrawSettings.mDrawJointOrientations)
			inRenderer->DrawCoordinateSystem(mJointMatrices[b], 0.05f);

		if (inDrawSettings.mDrawJointNames)
			inRenderer->DrawText3D(mJointMatrices[b].GetTranslation(), joints[b].mName, Color::sWhite, 0.05f);
	}
}
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_END

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Skeleton/Skeleton.h>
#include <Jolt/Skeleton/SkeletalAnimation.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// Instance of a skeleton, contains the pose the current skeleton is in
class SkeletonPose
{
public:
	using JointState = SkeletalAnimation::JointState;
	using JointStateVector = vector<JointState>;
	using Mat44Vector = vector<Mat44>;

	///@name Skeleton
	///@{
	void						SetSkeleton(const Skeleton *inSkeleton);
	const Skeleton *			GetSkeleton() const														{ return mSkeleton; }
	///@}

	///@name Properties of the joints
	///@{
	const JointStateVector &	GetJoints() const														{ return mJoints; }
	JointStateVector &			GetJoints()																{ return mJoints; }
	const JointState &			GetJoint(int inJoint) const												{ return mJoints[inJoint]; }
	JointState &				GetJoint(int inJoint)													{ return mJoints[inJoint]; }
	///@}

	///@name Joint matrices
	///@{
	void						CalculateJointMatrices();
	const Mat44Vector &			GetJointMatrices() const												{ return mJointMatrices; }
	Mat44Vector &				GetJointMatrices()														{ return mJointMatrices; }
	const Mat44 &				GetJointMatrix(int inJoint) const										{ return mJointMatrices[inJoint]; }
	Mat44 &						GetJointMatrix(int inJoint)												{ return mJointMatrices[inJoint]; }
	///@}

#ifdef JPH_DEBUG_RENDERER
	/// Draw settings
	struct DrawSettings
	{
		bool					mDrawJoints = true;
		bool					mDrawJointOrientations = true;
		bool					mDrawJointNames = false;
	};

	/// Draw current pose
	void						Draw(const DrawSettings &inDrawSettings, DebugRenderer *inRenderer) const;
#endif // JPH_DEBUG_RENDERER

private:
	RefConst<Skeleton>			mSkeleton;																///< Skeleton definition
	JointStateVector			mJoints;																///< Local joint orientations (local to parent Joint)
	Mat44Vector					mJointMatrices;															///< Local joint matrices (local to world matrix)
};

JPH_NAMESPACE_END

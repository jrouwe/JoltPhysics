// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/Result.h>
#include <Jolt/ObjectStream/SerializableObject.h>

JPH_NAMESPACE_BEGIN

class StreamIn;
class StreamOut;

/// Resource that contains the joint hierarchy for a skeleton
class Skeleton : public RefTarget<Skeleton>
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(Skeleton)

	using SkeletonResult = Result<Ref<Skeleton>>;

	/// Declare internal structure for a joint
	class Joint
	{
	public:
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(Joint)

							Joint() = default;
							Joint(const string_view &inName, const string_view &inParentName, int inParentJointIndex) : mName(inName), mParentName(inParentName), mParentJointIndex(inParentJointIndex) { }

		string				mName;																		///< Name of the joint
		string				mParentName;																///< Name of parent joint
		int					mParentJointIndex = -1;														///< Index of parent joint (in mJoints) or -1 if it has no parent
	};

	using JointVector = vector<Joint>;

	///@name Access to the joints
	///@{
	const JointVector &		GetJoints() const															{ return mJoints; }
	JointVector &			GetJoints()																	{ return mJoints; }
	int						GetJointCount() const														{ return (int)mJoints.size(); }
	const Joint &			GetJoint(int inJoint) const													{ return mJoints[inJoint]; }
	Joint &					GetJoint(int inJoint)														{ return mJoints[inJoint]; }
	uint					AddJoint(const string_view &inName, const string_view &inParentName = string_view()) { mJoints.emplace_back(inName, inParentName, -1); return (uint)mJoints.size() - 1; }
	uint					AddJoint(const string_view &inName, int inParentIndex)						{ mJoints.emplace_back(inName, inParentIndex >= 0? mJoints[inParentIndex].mName : string(), inParentIndex); return (uint)mJoints.size() - 1; }
	///@}

	/// Find joint by name
	int						GetJointIndex(const string_view &inName) const;

	/// Fill in parent joint indices based on name
	void					CalculateParentJointIndices();

	/// Saves the state of this object in binary form to inStream.
	void					SaveBinaryState(StreamOut &inStream) const;

	/// Restore the state of this object from inStream.
	static SkeletonResult	sRestoreFromBinaryState(StreamIn &inStream);

private:
	/// Joints
	JointVector				mJoints;
};

JPH_NAMESPACE_END

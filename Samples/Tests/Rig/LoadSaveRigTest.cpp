// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/LoadSaveRigTest.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Utils/Log.h>
#include <Utils/RagdollLoader.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSaveRigTest)
{
	JPH_ADD_BASE_CLASS(LoadSaveRigTest, Test)
}

LoadSaveRigTest::~LoadSaveRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void LoadSaveRigTest::Initialize()
{
	// Floor
	CreateFloor();

	stringstream data;

	{
		// Load ragdoll
		Ref<RagdollSettings> settings = RagdollLoader::sLoad("Human.tof", EMotionType::Dynamic);

		// Add an additional constraint between the left and right arm to test loading/saving of additional constraints
		const Skeleton *skeleton = settings->GetSkeleton();
		int left_arm = skeleton->GetJointIndex("L_Wrist_sjnt_0");
		int right_arm = skeleton->GetJointIndex("R_Wrist_sjnt_0");
		Ref<DistanceConstraintSettings> constraint = new DistanceConstraintSettings;
		constraint->mSpace = EConstraintSpace::LocalToBodyCOM;
		constraint->mMaxDistance = 0.1f;
		constraint->mMinDistance = 0.1f;
		settings->mAdditionalConstraints.push_back(RagdollSettings::AdditionalConstraint(left_arm, right_arm , constraint));

		// Write ragdoll
		if (!ObjectStreamOut::sWriteObject(data, ObjectStream::EStreamType::Text, *settings))
			FatalError("Failed to save ragdoll");
	}

	// Read ragdoll back in
	Ref<RagdollSettings> settings;
	if (!ObjectStreamIn::sReadObject(data, settings))
		FatalError("Failed to load ragdoll");

	// Parent joint indices are not stored so need to be calculated again
	settings->GetSkeleton()->CalculateParentJointIndices();

	// Create ragdoll
	mRagdoll = settings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);
}

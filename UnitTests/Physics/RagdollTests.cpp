// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Core/StreamWrapper.h>

TEST_SUITE("RagdollTests")
{
	static constexpr int cNumParts = 4;
	static constexpr float cHalfExtent = 0.25f;

	// Rest pose positions of the parts
	static const Vec3 cPositions[cNumParts] = { Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(2, 0, 0), Vec3(-1, 0, 0) };

	// Create a ragdoll with the following structure:
	//
	// 3 -- 0 -- 1 -- 2
	//
	// 0 is the root, 1 and 3 are connected to 0 with a swing twist constraint, 2 is connected to 1 with a hinge constraint
	static Ref<RagdollSettings> sCreateRagdollSettings(EMotionType inMotionType, const float *inDensities = nullptr)
	{
		Ref<Skeleton> skeleton = new Skeleton;
		skeleton->AddJoint("Root");
		skeleton->AddJoint("Child1", 0);
		skeleton->AddJoint("Child2", 1);
		skeleton->AddJoint("Child3", 0);

		Ref<RagdollSettings> settings = new RagdollSettings;
		settings->mSkeleton = skeleton;
		settings->mParts.resize(cNumParts);
		for (int p = 0; p < cNumParts; ++p)
		{
			RagdollSettings::Part &part = settings->mParts[p];
			part.SetShape(new BoxShape(Vec3::sReplicate(cHalfExtent), 0.05f));
			part.mPosition = RVec3(cPositions[p]);
			part.mMotionType = inMotionType;
			part.mObjectLayer = Layers::MOVING;
			if (inDensities != nullptr)
			{
				part.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				part.mMassPropertiesOverride.mMass = inDensities[p] * Cubed(2.0f * cHalfExtent);
			}
		}

		// Swing twist constraints between 0 - 1 and 0 - 3
		for (int p : { 1, 3 })
		{
			Ref<SwingTwistConstraintSettings> constraint = new SwingTwistConstraintSettings;
			constraint->mPosition1 = constraint->mPosition2 = RVec3(0.5f * (cPositions[0] + cPositions[p]));
			constraint->mTwistAxis1 = constraint->mTwistAxis2 = (cPositions[p] - cPositions[0]).Normalized();
			constraint->mPlaneAxis1 = constraint->mPlaneAxis2 = Vec3::sAxisY();
			constraint->mNormalHalfConeAngle = DegreesToRadians(80.0f);
			constraint->mPlaneHalfConeAngle = DegreesToRadians(80.0f);
			constraint->mTwistMinAngle = DegreesToRadians(-80.0f);
			constraint->mTwistMaxAngle = DegreesToRadians(80.0f);
			settings->mParts[p].mToParent = constraint;
		}

		// Hinge constraint between 1 - 2
		Ref<HingeConstraintSettings> hinge = new HingeConstraintSettings;
		hinge->mPoint1 = hinge->mPoint2 = RVec3(0.5f * (cPositions[1] + cPositions[2]));
		hinge->mHingeAxis1 = hinge->mHingeAxis2 = Vec3::sAxisZ();
		hinge->mNormalAxis1 = hinge->mNormalAxis2 = Vec3::sAxisX();
		settings->mParts[2].mToParent = hinge;

		settings->CalculateBodyIndexToConstraintIndex();
		settings->CalculateConstraintIndexToBodyIdxPair();

		return settings;
	}

	TEST_CASE("TestRagdollStabilizeNoJoints")
	{
		RagdollSettings settings;
		settings.mSkeleton = new Skeleton;
		CHECK(settings.Stabilize());
	}

	TEST_CASE("TestRagdollStabilize")
	{
		// Create a ragdoll with large mass differences
		const float cDensities[cNumParts] = { 1000.0f, 10.0f, 1000.0f, 500.0f };
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic, cDensities);

		// Calculate the original mass properties
		float original_total_mass = 0.0f;
		MassProperties original[cNumParts];
		for (int p = 0; p < cNumParts; ++p)
		{
			original[p] = settings->mParts[p].GetMassProperties();
			original_total_mass += original[p].mMass;
		}

		CHECK(settings->Stabilize());

		// Check that mass properties are now provided
		float total_mass = 0.0f;
		for (const RagdollSettings::Part &p : settings->mParts)
		{
			CHECK(p.mOverrideMassProperties == EOverrideMassProperties::MassAndInertiaProvided);
			total_mass += p.mMassPropertiesOverride.mMass;
		}

		// Total mass should be preserved
		CHECK_APPROX_EQUAL(total_mass, original_total_mass, 1.0e-3f * original_total_mass);

		// The mass ratio between parent and child should be limited
		for (int p = 1; p < cNumParts; ++p)
		{
			int parent = settings->mSkeleton->GetJoint(p).mParentJointIndex;
			float ratio = settings->mParts[p].mMassPropertiesOverride.mMass / settings->mParts[parent].mMassPropertiesOverride.mMass;
			CHECK(ratio >= 0.8f - 1.0e-5f); // cMinMassRatio
			CHECK(ratio <= 1.2f + 1.0e-5f); // cMaxMassRatio
		}

		// Inertia should be at least the inertia scaled by the new mass, for the leaves it should be equal to that
		for (int p = 0; p < cNumParts; ++p)
		{
			const RagdollSettings::Part &part = settings->mParts[p];
			Vec3 scaled_inertia = original[p].mInertia.GetDiagonal3() * (part.mMassPropertiesOverride.mMass / original[p].mMass);
			Vec3 inertia = part.mMassPropertiesOverride.mInertia.GetDiagonal3();
			if (p == 2 || p == 3)
				CHECK_APPROX_EQUAL(inertia, scaled_inertia, 1.0e-3f * scaled_inertia.Length());
			else
				CHECK(Vec3::sGreaterOrEqual(inertia, scaled_inertia * (1.0f - 1.0e-5f)).TestAllXYZTrue());
		}

		// The root has 2 children, its inertia should have been increased
		Vec3 root_scaled_inertia = original[0].mInertia.GetDiagonal3() * (settings->mParts[0].mMassPropertiesOverride.mMass / original[0].mMass);
		CHECK(Vec3::sGreater(settings->mParts[0].mMassPropertiesOverride.mInertia.GetDiagonal3(), 1.1f * root_scaled_inertia).TestAllXYZTrue());
	}

	TEST_CASE("TestRagdollStabilizeStaticRoot")
	{
		const float cDensities[cNumParts] = { 1000.0f, 10.0f, 1000.0f, 500.0f };
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic, cDensities);
		settings->mParts[0].mMotionType = EMotionType::Static;
		MassProperties part3 = settings->mParts[3].GetMassProperties();

		CHECK(settings->Stabilize());

		// The static root should not have been touched
		CHECK(settings->mParts[0].mOverrideMassProperties == EOverrideMassProperties::CalculateInertia);

		// Part 3 forms a chain on its own and cannot redistribute mass
		CHECK(settings->mParts[3].mOverrideMassProperties == EOverrideMassProperties::MassAndInertiaProvided);
		CHECK_APPROX_EQUAL(settings->mParts[3].mMassPropertiesOverride.mMass, part3.mMass);
		CHECK_APPROX_EQUAL(settings->mParts[3].mMassPropertiesOverride.mInertia, part3.mInertia);

		// Part 1 and 2 form a chain, the mass ratio should be limited
		float ratio = settings->mParts[2].mMassPropertiesOverride.mMass / settings->mParts[1].mMassPropertiesOverride.mMass;
		CHECK_APPROX_EQUAL(ratio, 1.2f, 1.0e-5f);
	}

	TEST_CASE("TestRagdollCalculateConstraintPriorities")
	{
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
		settings->mAdditionalConstraints.emplace_back(1, 3, new DistanceConstraintSettings);

		constexpr uint32 cBasePriority = 10;
		settings->CalculateConstraintPriorities(cBasePriority);

		// Constraints closer to the root should have a higher priority
		CHECK(settings->mParts[1].mToParent->mConstraintPriority == cBasePriority + 1);
		CHECK(settings->mParts[2].mToParent->mConstraintPriority == cBasePriority);
		CHECK(settings->mParts[3].mToParent->mConstraintPriority == cBasePriority);

		// Additional constraint takes the minimum of the connected bodies
		CHECK(settings->mAdditionalConstraints[0].mConstraint->mConstraintPriority == cBasePriority);
	}

	TEST_CASE("TestRagdollDisableParentChildCollisions")
	{
		auto get_filter = [](const RagdollSettings *inSettings) {
			const GroupFilterTable *filter = static_cast<const GroupFilterTable *>(inSettings->mParts[0].mCollisionGroup.GetGroupFilter());
			for (int p = 0; p < cNumParts; ++p)
			{
				// All parts should share the same filter and have their joint index as sub group ID
				CHECK(inSettings->mParts[p].mCollisionGroup.GetGroupFilter() == filter);
				CHECK(inSettings->mParts[p].mCollisionGroup.GetSubGroupID() == CollisionGroup::SubGroupID(p));
			}
			return filter;
		};

		{
			// Without a pose, only parent / child collisions should be disabled
			Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
			settings->DisableParentChildCollisions();
			const GroupFilterTable *filter = get_filter(settings);
			CHECK(!filter->IsCollisionEnabled(0, 1));
			CHECK(!filter->IsCollisionEnabled(1, 2));
			CHECK(!filter->IsCollisionEnabled(0, 3));
			CHECK(filter->IsCollisionEnabled(0, 2));
			CHECK(filter->IsCollisionEnabled(1, 3));
			CHECK(filter->IsCollisionEnabled(2, 3));
		}

		// Create a pose where 2 overlaps with 0 and 3 is close to 1
		const Mat44 cJointMatrices[cNumParts] = {
			Mat44::sTranslation(Vec3(0, 0, 0)),
			Mat44::sTranslation(Vec3(1, 0, 0)),
			Mat44::sTranslation(Vec3(0, 0.4f, 0)),
			Mat44::sTranslation(Vec3(1, 2.0f * cHalfExtent + 0.1f, 0))
		};

		{
			// Overlapping parts should get their collision disabled
			Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
			settings->DisableParentChildCollisions(cJointMatrices, 0.0f);
			const GroupFilterTable *filter = get_filter(settings);
			CHECK(!filter->IsCollisionEnabled(0, 1));
			CHECK(!filter->IsCollisionEnabled(1, 2));
			CHECK(!filter->IsCollisionEnabled(0, 3));
			CHECK(!filter->IsCollisionEnabled(0, 2));
			CHECK(filter->IsCollisionEnabled(1, 3));
			CHECK(filter->IsCollisionEnabled(2, 3));
		}

		{
			// Parts that are closer than the separation distance should get their collision disabled too.
			// Make the root static to test static vs dynamic.
			Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
			settings->mParts[0].mMotionType = EMotionType::Static;
			settings->DisableParentChildCollisions(cJointMatrices, 0.2f);
			const GroupFilterTable *filter = get_filter(settings);
			CHECK(!filter->IsCollisionEnabled(0, 1));
			CHECK(!filter->IsCollisionEnabled(1, 2));
			CHECK(!filter->IsCollisionEnabled(0, 3));
			CHECK(!filter->IsCollisionEnabled(0, 2));
			CHECK(!filter->IsCollisionEnabled(1, 3));
			CHECK(filter->IsCollisionEnabled(2, 3));
		}

		{
			// When both parts are static, no collision test is done
			Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Static);
			settings->DisableParentChildCollisions(cJointMatrices, 0.2f);
			const GroupFilterTable *filter = get_filter(settings);
			CHECK(filter->IsCollisionEnabled(0, 2));
			CHECK(filter->IsCollisionEnabled(1, 3));
		}
	}

	TEST_CASE("TestRagdollSaveRestore")
	{
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
		settings->mAdditionalConstraints.emplace_back(2, 3, new DistanceConstraintSettings);
		settings->DisableParentChildCollisions();

		stringstream data;

		// Write the ragdoll
		StreamOutWrapper stream_out(data);
		settings->SaveBinaryState(stream_out, true, true);

		// Read it back
		StreamInWrapper stream_in(data);
		RagdollSettings::RagdollResult result = RagdollSettings::sRestoreFromBinaryState(stream_in);
		CHECK(result.IsValid());
		Ref<RagdollSettings> restored = result.Get();

		// Check skeleton
		CHECK(restored->GetSkeleton()->GetJointCount() == cNumParts);
		for (int j = 0; j < cNumParts; ++j)
		{
			CHECK(restored->GetSkeleton()->GetJoint(j).mName == settings->GetSkeleton()->GetJoint(j).mName);
			CHECK(restored->GetSkeleton()->GetJoint(j).mParentJointIndex == settings->GetSkeleton()->GetJoint(j).mParentJointIndex);
		}

		// Check parts
		CHECK(restored->mParts.size() == cNumParts);
		for (int p = 0; p < cNumParts; ++p)
		{
			const RagdollSettings::Part &original_part = settings->mParts[p];
			const RagdollSettings::Part &restored_part = restored->mParts[p];
			CHECK(restored_part.mPosition == original_part.mPosition);
			CHECK(restored_part.mMotionType == original_part.mMotionType);
			CHECK(restored_part.GetShape()->GetSubType() == EShapeSubType::Box);
			CHECK(restored_part.mCollisionGroup.GetSubGroupID() == original_part.mCollisionGroup.GetSubGroupID());
			CHECK(restored_part.mCollisionGroup.GetGroupFilter() != nullptr);
			CHECK(restored_part.mCollisionGroup.GetGroupFilter() == restored->mParts[0].mCollisionGroup.GetGroupFilter());
			if (original_part.mToParent == nullptr)
				CHECK(restored_part.mToParent == nullptr);
			else
				CHECK(restored_part.mToParent->GetRTTI() == original_part.mToParent->GetRTTI());
		}

		// Check that the group filter was restored correctly
		const GroupFilterTable *filter = static_cast<const GroupFilterTable *>(restored->mParts[0].mCollisionGroup.GetGroupFilter());
		CHECK(!filter->IsCollisionEnabled(0, 1));
		CHECK(!filter->IsCollisionEnabled(1, 2));
		CHECK(!filter->IsCollisionEnabled(0, 3));
		CHECK(filter->IsCollisionEnabled(0, 2));

		// Check additional constraints
		CHECK(restored->mAdditionalConstraints.size() == 1);
		CHECK(restored->mAdditionalConstraints[0].mBodyIdx[0] == 2);
		CHECK(restored->mAdditionalConstraints[0].mBodyIdx[1] == 3);
		CHECK(restored->mAdditionalConstraints[0].mConstraint->GetRTTI() == JPH_RTTI(DistanceConstraintSettings));

		// Check that mapping tables were calculated
		CHECK(restored->GetBodyIndexToConstraintIndex() == Array<int>({ -1, 0, 1, 2 }));
		CHECK(restored->GetConstraintIndexToBodyIdxPair() == Array<RagdollSettings::BodyIdxPair>({ { 0, 1 }, { 1, 2 }, { 0, 3 }, { 2, 3 } }));
		CHECK(restored->GetConstraintIndexForBodyIndex(2) == 1);
		CHECK(restored->GetBodyIndicesForConstraintIndex(3) == RagdollSettings::BodyIdxPair(2, 3));
	}

	TEST_CASE("TestRagdollOutOfBodies")
	{
		// Create a system that can only hold 2 bodies
		PhysicsTestContext c(1.0f / 60.0f, 1, 0, 2);

		// Creating a ragdoll with 4 bodies should fail
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
		Ref<Ragdoll> ragdoll = settings->CreateRagdoll(0, 0, c.GetSystem());
		CHECK(ragdoll == nullptr);
		CHECK(c.GetSystem()->GetNumBodies() == 0);
	}

	TEST_CASE("TestRagdollLifeCycle")
	{
		PhysicsTestContext c;
		c.ZeroGravity();
		PhysicsSystem *system = c.GetSystem();
		BodyInterface &bi = c.GetBodyInterface();

		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
		settings->mAdditionalConstraints.emplace_back(2, 3, new DistanceConstraintSettings);
		settings->CalculateConstraintIndexToBodyIdxPair();

		constexpr uint64 cUserData = 0x1234567890abcdefULL;
		Ref<Ragdoll> ragdoll = settings->CreateRagdoll(5, cUserData, system);
		CHECK(ragdoll->GetRagdollSettings() == settings);
		CHECK(ragdoll->GetBodyCount() == cNumParts);
		CHECK(ragdoll->GetBodyIDs().size() == cNumParts);
		CHECK(ragdoll->GetConstraintCount() == cNumParts);
		CHECK(system->GetNumBodies() == cNumParts);

		// Check that the bodies were set up correctly
		for (int p = 0; p < cNumParts; ++p)
		{
			BodyID id = ragdoll->GetBodyID(p);
			CHECK(bi.GetUserData(id) == cUserData);
			CHECK(bi.GetPosition(id) == RVec3(cPositions[p]));
			CHECK(!bi.IsAdded(id));
		}

		// Check that the constraints connect the right bodies
		for (int i = 0; i < (int)ragdoll->GetConstraintCount(); ++i)
		{
			RagdollSettings::BodyIdxPair pair = settings->GetBodyIndicesForConstraintIndex(i);
			const TwoBodyConstraint *constraint = ragdoll->GetConstraint(i);
			CHECK(constraint->GetBody1()->GetID() == ragdoll->GetBodyID(pair.first));
			CHECK(constraint->GetBody2()->GetID() == ragdoll->GetBodyID(pair.second));
		}
		CHECK(ragdoll->GetConstraint(1)->GetSubType() == EConstraintSubType::Hinge);

		// Check group ID
		{
			BodyLockRead lock(system->GetBodyLockInterface(), ragdoll->GetBodyID(2));
			CHECK(lock.GetBody().GetCollisionGroup().GetGroupID() == 5);
		}
		ragdoll->SetGroupID(6);
		for (BodyID id : ragdoll->GetBodyIDs())
		{
			BodyLockRead lock(system->GetBodyLockInterface(), id);
			CHECK(lock.GetBody().GetCollisionGroup().GetGroupID() == 6);
		}
		ragdoll->SetGroupID(7, false);
		{
			BodyLockRead lock(system->GetBodyLockInterface(), ragdoll->GetBodyID(3));
			CHECK(lock.GetBody().GetCollisionGroup().GetGroupID() == 7);
		}

		// Add without activating
		ragdoll->AddToPhysicsSystem(EActivation::DontActivate);
		for (BodyID id : ragdoll->GetBodyIDs())
			CHECK(bi.IsAdded(id));
		CHECK(system->GetConstraints().size() == cNumParts);
		CHECK(!ragdoll->IsActive(true)); // locking
		CHECK(!ragdoll->IsActive(false)); // non-locking

		// Activate
		ragdoll->Activate();
		CHECK(ragdoll->IsActive(true));
		CHECK(ragdoll->IsActive(false));

		// Check root transform
		RVec3 root_pos;
		Quat root_rot;
		ragdoll->GetRootTransform(root_pos, root_rot);
		CHECK(root_pos == RVec3(cPositions[0]));
		CHECK(root_rot == Quat::sIdentity());

		// Check bounds
		AABox bounds = ragdoll->GetWorldSpaceBounds();
		CHECK_APPROX_EQUAL(bounds.mMin, Vec3(-1.0f - cHalfExtent, -cHalfExtent, -cHalfExtent));
		CHECK_APPROX_EQUAL(bounds.mMax, Vec3(2.0f + cHalfExtent, cHalfExtent, cHalfExtent));

		// Set velocities
		const Vec3 cLinearVelocity(1, 2, 3);
		const Vec3 cAngularVelocity(0.1f, 0.2f, 0.3f);
		ragdoll->SetLinearAndAngularVelocity(cLinearVelocity, cAngularVelocity);
		for (BodyID id : ragdoll->GetBodyIDs())
		{
			CHECK(bi.GetLinearVelocity(id) == cLinearVelocity);
			CHECK(bi.GetAngularVelocity(id) == cAngularVelocity);
		}
		ragdoll->SetLinearVelocity(2.0f * cLinearVelocity);
		for (BodyID id : ragdoll->GetBodyIDs())
			CHECK(bi.GetLinearVelocity(id) == 2.0f * cLinearVelocity);
		ragdoll->AddLinearVelocity(cLinearVelocity);
		for (BodyID id : ragdoll->GetBodyIDs())
			CHECK_APPROX_EQUAL(bi.GetLinearVelocity(id), 3.0f * cLinearVelocity);
		ragdoll->SetLinearVelocity(Vec3::sZero());
		const Vec3 cImpulse(10, 0, 0);
		ragdoll->AddImpulse(cImpulse);
		for (BodyID id : ragdoll->GetBodyIDs())
		{
			BodyLockRead lock(system->GetBodyLockInterface(), id);
			CHECK_APPROX_EQUAL(lock.GetBody().GetLinearVelocity(), cImpulse * lock.GetBody().GetMotionProperties()->GetInverseMass());
		}

		// Reset warm start and simulate a step to see that nothing breaks
		ragdoll->ResetWarmStart();
		c.SimulateSingleStep();

		// Remove from the system
		ragdoll->RemoveFromPhysicsSystem();
		for (BodyID id : ragdoll->GetBodyIDs())
			CHECK(!bi.IsAdded(id));
		CHECK(system->GetConstraints().empty());

		// Add and remove again without locking
		ragdoll->AddToPhysicsSystem(EActivation::Activate, false);
		CHECK(ragdoll->IsActive());
		ragdoll->RemoveFromPhysicsSystem(false);
		CHECK(system->GetConstraints().empty());

		// Destroying the ragdoll should destroy the bodies
		ragdoll = nullptr;
		CHECK(system->GetNumBodies() == 0);
	}

	TEST_CASE("TestRagdollSetGetPose")
	{
		PhysicsTestContext c;
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
		Ref<Ragdoll> ragdoll = settings->CreateRagdoll(0, 0, c.GetSystem());
		ragdoll->AddToPhysicsSystem(EActivation::DontActivate);

		// Create a pose
		SkeletonPose pose;
		pose.SetSkeleton(settings->GetSkeleton());
		pose.SetRootOffset(RVec3(5, 6, 7));
		pose.GetJoint(0).mRotation = Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI);
		pose.GetJoint(1).mTranslation = Vec3(1, 0, 0);
		pose.GetJoint(1).mRotation = Quat::sRotation(Vec3::sAxisZ(), 0.2f * JPH_PI);
		pose.GetJoint(2).mTranslation = Vec3(1, 0, 0);
		pose.GetJoint(2).mRotation = Quat::sRotation(Vec3::sAxisZ(), 0.3f * JPH_PI);
		pose.GetJoint(3).mTranslation = Vec3(-1, 0, 0);
		pose.GetJoint(3).mRotation = Quat::sRotation(Vec3::sAxisX(), 0.4f * JPH_PI);
		pose.CalculateJointMatrices();

		for (bool lock_bodies : { true, false })
		{
			// Set the pose
			ragdoll->SetPose(pose, lock_bodies);
			for (int p = 0; p < cNumParts; ++p)
			{
				RVec3 position;
				Quat rotation;
				c.GetBodyInterface().GetPositionAndRotation(ragdoll->GetBodyID(p), position, rotation);
				CHECK_APPROX_EQUAL(position, pose.GetRootOffset() + pose.GetJointMatrix(p).GetTranslation(), 1.0e-5f);
				CHECK_APPROX_EQUAL(rotation, pose.GetJointMatrix(p).GetQuaternion(), 1.0e-5f);
			}

			// Get the pose back
			SkeletonPose result;
			result.SetSkeleton(settings->GetSkeleton());
			ragdoll->GetPose(result, lock_bodies);
			CHECK_APPROX_EQUAL(result.GetRootOffset(), pose.GetRootOffset(), 1.0e-5f);
			for (int p = 0; p < cNumParts; ++p)
				CHECK_APPROX_EQUAL(result.GetJointMatrix(p), pose.GetJointMatrix(p), 1.0e-5f);

			// Reset the pose
			ragdoll->SetPose(RVec3::sZero(), Array<Mat44>(cNumParts, Mat44::sIdentity()).data(), lock_bodies);
			RVec3 root_pos;
			Quat root_rot;
			ragdoll->GetRootTransform(root_pos, root_rot, lock_bodies);
			CHECK(root_pos == RVec3::sZero());
			CHECK(root_rot == Quat::sIdentity());
		}

		ragdoll->RemoveFromPhysicsSystem();
	}

	TEST_CASE("TestRagdollDriveToPoseUsingKinematics")
	{
		PhysicsTestContext c;
		Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Kinematic);
		Ref<Ragdoll> ragdoll = settings->CreateRagdoll(0, 0, c.GetSystem());
		ragdoll->AddToPhysicsSystem(EActivation::Activate);

		// Create a pose that moves and rotates the ragdoll
		SkeletonPose pose;
		pose.SetSkeleton(settings->GetSkeleton());
		pose.SetRootOffset(RVec3(1, 2, 3));
		pose.GetJoint(0).mRotation = Quat::sRotation(Vec3::sAxisY(), 0.1f * JPH_PI);
		for (int p = 1; p < cNumParts; ++p)
			pose.GetJoint(p).mTranslation = cPositions[p] - cPositions[settings->GetSkeleton()->GetJoint(p).mParentJointIndex];
		pose.GetJoint(2).mRotation = Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI);
		pose.CalculateJointMatrices();

		for (bool lock_bodies : { true, false })
		{
			// Drive to the pose in a single step
			if (lock_bodies)
				ragdoll->DriveToPoseUsingKinematics(pose, c.GetDeltaTime());
			else
				ragdoll->DriveToPoseUsingKinematics(pose.GetRootOffset(), pose.GetJointMatrices().data(), c.GetDeltaTime(), false);
			c.SimulateSingleStep();

			// Check that we reached the pose
			for (int p = 0; p < cNumParts; ++p)
			{
				RVec3 position;
				Quat rotation;
				c.GetBodyInterface().GetPositionAndRotation(ragdoll->GetBodyID(p), position, rotation);
				CHECK_APPROX_EQUAL(position, pose.GetRootOffset() + pose.GetJointMatrix(p).GetTranslation(), 1.0e-4f);
				CHECK_APPROX_EQUAL(rotation, pose.GetJointMatrix(p).GetQuaternion(), 1.0e-4f);
			}

			// Move the pose for the next iteration
			pose.SetRootOffset(pose.GetRootOffset() + RVec3(0, 1, 0));
		}

		ragdoll->RemoveFromPhysicsSystem();
	}

	TEST_CASE("TestRagdollDriveToPoseUsingMotors")
	{
		for (bool use_velocity : { false, true })
		{
			PhysicsTestContext c;
			c.ZeroGravity();

			// Create ragdoll with a static root
			Ref<RagdollSettings> settings = sCreateRagdollSettings(EMotionType::Dynamic);
			settings->mParts[0].mMotionType = EMotionType::Static;
			settings->DisableParentChildCollisions();

			// Default motor settings are too weak to drive the ragdoll to the target pose, so we increase them
			const MotorSettings cMotorSettings(10.0f, 1.0f);
			for (RagdollSettings::Part &part : settings->mParts)
			{
				part.mAllowSleeping = false;
				if (SwingTwistConstraintSettings *st = DynamicCast<SwingTwistConstraintSettings>(part.mToParent))
					st->mSwingMotorSettings = st->mTwistMotorSettings = cMotorSettings;
				else if (HingeConstraintSettings *h = DynamicCast<HingeConstraintSettings>(part.mToParent))
					h->mMotorSettings = cMotorSettings;
			}

			Ref<Ragdoll> ragdoll = settings->CreateRagdoll(0, 0, c.GetSystem());
			ragdoll->AddToPhysicsSystem(EActivation::Activate);

			// Create the target pose
			SkeletonPose target;
			target.SetSkeleton(settings->GetSkeleton());
			target.GetJoint(1).mRotation = Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(30.0f));
			target.GetJoint(2).mRotation = Quat::sRotation(Vec3::sAxisZ(), DegreesToRadians(45.0f));
			target.GetJoint(3).mRotation = Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(20.0f));

			if (use_velocity)
			{
				// Interpolate from the rest pose to the target pose over 1 second, then simulate another second to allow the system to settle
				SkeletonPose prev_pose;
				prev_pose.SetSkeleton(settings->GetSkeleton());
				const int cNumSteps = int(round(1.0f / c.GetDeltaTime()));
				for (int step = 1; step <= 2 * cNumSteps; ++step)
				{
					SkeletonPose pose;
					pose.SetSkeleton(settings->GetSkeleton());
					float fraction = min(1.0f, float(step) / cNumSteps);
					for (int p = 0; p < cNumParts; ++p)
						pose.GetJoint(p).mRotation = Quat::sIdentity().SLERP(target.GetJoint(p).mRotation, fraction);
					ragdoll->DriveToPoseUsingMotors(prev_pose, pose, c.GetDeltaTime());
					c.SimulateSingleStep();
					prev_pose = pose;
				}
			}
			else
			{
				// Drive directly to the target pose
				ragdoll->DriveToPoseUsingMotors(target);
				c.Simulate(3.0f);
			}

			// Check that the motors have been turned on
			const SwingTwistConstraint *swing_twist = static_cast<const SwingTwistConstraint *>(ragdoll->GetConstraint(settings->GetConstraintIndexForBodyIndex(1)));
			EMotorState expected_state = use_velocity? EMotorState::PositionAndVelocity : EMotorState::Position;
			CHECK(swing_twist->GetSwingMotorState() == expected_state);
			CHECK(swing_twist->GetTwistMotorState() == expected_state);
			const HingeConstraint *hinge = static_cast<const HingeConstraint *>(ragdoll->GetConstraint(settings->GetConstraintIndexForBodyIndex(2)));
			CHECK(hinge->GetMotorState() == expected_state);

			// Check that we reached the target pose
			SkeletonPose result;
			result.SetSkeleton(settings->GetSkeleton());
			ragdoll->GetPose(result);
			result.CalculateJointStates();
			for (int p = 1; p < cNumParts; ++p)
				CHECK_APPROX_EQUAL(result.GetJoint(p).mRotation, target.GetJoint(p).mRotation, 1.0e-2f);

			ragdoll->RemoveFromPhysicsSystem();
		}
	}
}

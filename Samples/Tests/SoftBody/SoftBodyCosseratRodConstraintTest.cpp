// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyCosseratRodConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyCosseratRodConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyCosseratRodConstraintTest, Test)
}

void SoftBodyCosseratRodConstraintTest::Initialize()
{
	CreateFloor();

	// Create a hanging helix
	{
		constexpr float cRadius = 0.5f;
		constexpr int cNumVertices = 128;
		constexpr float cHeight = 5.0f;
		constexpr float cNumCycles = 10;

		Ref<SoftBodySharedSettings> helix_settings = new SoftBodySharedSettings;
		for (int i = 0; i < cNumVertices; ++i)
		{
			float fraction = float(i) / (cNumVertices - 1);

			SoftBodySharedSettings::Vertex v;
			float alpha = cNumCycles * 2.0f * JPH_PI * fraction;
			v.mPosition = Float3(cRadius * Sin(alpha), 0.5f * (1.0f - fraction * cHeight), cRadius * Cos(alpha));
			v.mInvMass = i == 0? 0.0f : 1.0e-2f;
			helix_settings->mVertices.push_back(v);

			if (i > 0)
				helix_settings->mRodStretchShearConstraints.push_back(SoftBodySharedSettings::RodStretchShear(i - 1, i));

			if (i > 1)
				helix_settings->mRodBendTwistConstraints.push_back(SoftBodySharedSettings::RodBendTwist(i - 2, i - 1));
		}

		helix_settings->CalculateRodProperties();
		helix_settings->Optimize();

		SoftBodyCreationSettings helix(helix_settings, RVec3(0, 10, 0), Quat::sIdentity(), Layers::MOVING);
		mSoftBodies.push_back(mBodyInterface->CreateAndAddSoftBody(helix, EActivation::Activate));
	}

	// Create a tree with a static root
	{
		// Root particle
		Ref<SoftBodySharedSettings> tree_settings = new SoftBodySharedSettings;
		SoftBodySharedSettings::Vertex v;
		v.mPosition = Float3(0, 0, 0);
		v.mInvMass = 0.0f;
		tree_settings->mVertices.push_back(v);

		// Create branches
		struct Branch
		{
			uint32	mPreviousVertex;
			uint32	mPreviousRod;
			Vec3	mDirection;
			uint32	mDepth;
		};
		Array<Branch> branches;
		branches.push_back({ 0, uint32(-1), Vec3::sAxisY(), 0 });
		while (!branches.empty())
		{
			// Take the next branch
			Branch branch = branches.front();
			branches.erase(branches.begin());

			// Create vertex
			SoftBodySharedSettings::Vertex &previous_vertex = tree_settings->mVertices[branch.mPreviousVertex];
			(Vec3(previous_vertex.mPosition) + branch.mDirection).StoreFloat3(&v.mPosition);
			v.mInvMass = branch.mDepth > 0? 2.0f * previous_vertex.mInvMass : 1.0e-3f;
			uint32 new_vertex = uint32(tree_settings->mVertices.size());
			tree_settings->mVertices.push_back(v);

			// Create rod
			uint32 new_rod = uint32(tree_settings->mRodStretchShearConstraints.size());
			tree_settings->mRodStretchShearConstraints.push_back(SoftBodySharedSettings::RodStretchShear(branch.mPreviousVertex, new_vertex));
			if (branch.mPreviousRod != uint32(-1))
				tree_settings->mRodBendTwistConstraints.push_back(SoftBodySharedSettings::RodBendTwist(branch.mPreviousRod, new_rod));

			// Create sub branches
			if (branch.mDepth < 10)
				for (uint32 i = 0; i < 2; ++i)
				{
					// Determine new child direction
					float angle = DegreesToRadians(-15.0f + i * 30.0f);
					Vec3 new_direction = Quat::sRotation(branch.mDepth & 1? Vec3::sAxisZ() : Vec3::sAxisX(), angle) * branch.mDirection;

					// Create new branch
					branches.push_back({ new_vertex, new_rod, new_direction, branch.mDepth + 1 });
				}
		}

		tree_settings->CalculateRodProperties();
		tree_settings->Optimize();

		SoftBodyCreationSettings tree(tree_settings, RVec3(10, 0, 0), Quat::sIdentity(), Layers::MOVING);
		mSoftBodies.push_back(mBodyInterface->CreateAndAddSoftBody(tree, EActivation::Activate));
	}

	// Create a weed like structure
	{
		// Root particle
		Ref<SoftBodySharedSettings> weed_settings = new SoftBodySharedSettings;

		constexpr int cNumVertices = 64;
		constexpr int cNumStrands = 50;

		default_random_engine random;
		uniform_real_distribution<float> radius_distribution(0, 1.0f);
		uniform_real_distribution<float> phase_distribution(0, 2.0f * JPH_PI);

		for (int strand = 0; strand < cNumStrands; ++strand)
		{
			// Place at a random location
			float radius = radius_distribution(random);
			float theta = phase_distribution(random);
			Vec3 root_pos = Vec3(radius * Sin(theta), 0, radius * Cos(theta));

			// Randomize the phase of the wave
			float phase1 = phase_distribution(random);
			float phase2 = phase_distribution(random);

			uint32 first_vertex = uint32(weed_settings->mVertices.size());
			for (int i = 0; i < cNumVertices; ++i)
			{
				// Generate a wavy pattern
				float amplitude = 0.1f * Sin(phase1 + i * 2.0f * JPH_PI / 8);
				Vec3 pos = root_pos + Vec3(Sin(phase2) * amplitude, 0.1f * i, Cos(phase2) * amplitude);

				SoftBodySharedSettings::Vertex v;
				pos.StoreFloat3(&v.mPosition);
				v.mInvMass = i == 0? 0.0f : 0.1f;
				weed_settings->mVertices.push_back(v);
			}

			uint32 first_rod = uint32(weed_settings->mRodStretchShearConstraints.size());
			for (int i = 0; i < cNumVertices - 1; ++i)
				weed_settings->mRodStretchShearConstraints.push_back(SoftBodySharedSettings::RodStretchShear(first_vertex + i, first_vertex + i + 1));

			for (int i = 0; i < cNumVertices - 2; ++i)
				weed_settings->mRodBendTwistConstraints.push_back(SoftBodySharedSettings::RodBendTwist(first_rod + i, first_rod + i + 1));
		}

		weed_settings->CalculateRodProperties();
		weed_settings->Optimize();

		SoftBodyCreationSettings weed(weed_settings, RVec3(20, 0, 0), Quat::sIdentity(), Layers::MOVING);
		weed.mGravityFactor = 0.8f;
		mSoftBodies.push_back(mBodyInterface->CreateAndAddSoftBody(weed, EActivation::Activate));
	}
}

void SoftBodyCosseratRodConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Draw the soft body rods
	for (BodyID id : mSoftBodies)
	{
		BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (lock.Succeeded())
		{
			const Body &body = lock.GetBody();
			const SoftBodyMotionProperties *mp = static_cast<const SoftBodyMotionProperties *>(body.GetMotionProperties());
			RMat44 com = body.GetCenterOfMassTransform();

			for (const SoftBodySharedSettings::RodStretchShear &r : mp->GetSettings()->mRodStretchShearConstraints)
			{
				RVec3 x0 = com * mp->GetVertex(r.mVertex[0]).mPosition;
				RVec3 x1 = com * mp->GetVertex(r.mVertex[1]).mPosition;
				mDebugRenderer->DrawLine(x0, x1, Color::sWhite);
			}
		}
	}
}

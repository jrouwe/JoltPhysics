// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyBendConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyBendConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyBendConstraintTest, Test)
}

void SoftBodyBendConstraintTest::Initialize()
{
	CreateFloor();

	default_random_engine random;
	uniform_real_distribution<float> random_float(-0.1f, 0.1f);

	auto inv_mass = [](uint, uint inZ) { return inZ < 2? 0.0f : 1.0f; };
	auto perturbation = [&random, &random_float](uint, uint inZ) { return Vec3(random_float(random), (inZ & 1)? 0.1f : -0.1f, random_float(random)); };

	{
		random.seed(1234);

		// Cloth without bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, perturbation, SoftBodySharedSettings::EBendType::None);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(-5.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		random.seed(1234);

		// Cloth with distance bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, perturbation, SoftBodySharedSettings::EBendType::Distance);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(0.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		random.seed(1234);

		// Cloth with dihedral bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, perturbation, SoftBodySharedSettings::EBendType::Dihedral);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(5.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		random.seed(1234);

		// Cloth with Cosserat rod constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, perturbation, SoftBodySharedSettings::EBendType::None);

		// Get rid of created edges, we're replacing them with rods
		cloth_settings->mEdgeConstraints.clear();

		// Copy of SoftBodyCreator::CreateCloth: Function to get the vertex index of a point on the cloth
		auto vertex_index = [](uint inX, uint inY)
		{
			return inX + inY * cNumVerticesX;
		};

		// Create bend twist constraints
		constexpr float cCompliance = 1.0e-5f;
		auto get_rod = [&constraints = cloth_settings->mRodStretchShearConstraints, vertex_index, cCompliance](uint inX1, uint inZ1, uint inX2, uint inZ2)
		{
			uint32 v0 = vertex_index(inX1, inZ1);
			uint32 v1 = vertex_index(inX2, inZ2);
			JPH_ASSERT(v0 < v1);

			for (uint i = 0; i < uint(constraints.size()); ++i)
				if (constraints[i].mVertex[0] == v0 && constraints[i].mVertex[1] == v1)
					return i;

			constraints.emplace_back(v0, v1, cCompliance);
			return uint(constraints.size() - 1);
		};
		for (uint z = 1; z < cNumVerticesZ - 1; ++z)
			for (uint x = 0; x < cNumVerticesX - 1; ++x)
			{
				if (z > 1 && x < cNumVerticesX - 2)
					cloth_settings->mRodBendTwistConstraints.emplace_back(get_rod(x, z, x + 1, z), get_rod(x + 1, z, x + 2, z), cCompliance);
				if (z < cNumVerticesZ - 2)
					cloth_settings->mRodBendTwistConstraints.emplace_back(get_rod(x, z, x, z + 1), get_rod(x, z + 1, x, z + 2), cCompliance);
				if (x < cNumVerticesX - 2 && z < cNumVerticesZ - 2)
				{
					cloth_settings->mRodBendTwistConstraints.emplace_back(get_rod(x, z, x + 1, z + 1), get_rod(x + 1, z + 1, x + 2, z + 2), cCompliance);
					cloth_settings->mRodBendTwistConstraints.emplace_back(get_rod(x + 2, z, x + 1, z + 1), get_rod(x + 1, z + 1, x, z + 2), cCompliance);
				}
			}
		cloth_settings->CalculateRodProperties();

		// Optimize the settings
		cloth_settings->Optimize();
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(10.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		// Create sphere
		SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(1.0f, 10, 20, SoftBodySharedSettings::EBendType::None), RVec3(-5.0f, 5.0f, 10.0f), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}

	{
		// Create sphere with distance bend constraints
		SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(1.0f, 10, 20, SoftBodySharedSettings::EBendType::Distance), RVec3(0.0f, 5.0f, 10.0f), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}

	{
		// Create sphere with dihedral bend constraints
		SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(1.0f, 10, 20, SoftBodySharedSettings::EBendType::Dihedral), RVec3(5.0f, 5.0f, 10.0f), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}

	{
		// Create sphere with Cosserat rod constraints
		uint cNumTheta = 10;
		uint cNumPhi = 20;
		Ref<SoftBodySharedSettings> sphere_settings = SoftBodyCreator::CreateSphere(1.0f, cNumTheta, cNumPhi, SoftBodySharedSettings::EBendType::None);

		// Get rid of created edges, we're replacing them with rods
		sphere_settings->mEdgeConstraints.clear();

		// Copy of SoftBodyCreator::CreateSphere: Function to get the vertex index of a point on the sphere
		auto vertex_index = [cNumTheta, cNumPhi](uint inTheta, uint inPhi) -> uint
		{
			if (inTheta == 0)
				return 0;
			else if (inTheta == cNumTheta - 1)
				return 1;
			else
				return 2 + (inTheta - 1) * cNumPhi + inPhi % cNumPhi;
		};

		// Create bend twist constraints
		constexpr float cCompliance = 1.0e-4f;
		auto get_rod = [&constraints = sphere_settings->mRodStretchShearConstraints, vertex_index, cCompliance](uint inTheta1, uint inPhi1, uint inTheta2, uint inPhi2)
		{
			uint32 v0 = vertex_index(inTheta1, inPhi1);
			uint32 v1 = vertex_index(inTheta2, inPhi2);
			JPH_ASSERT(v0 != v1);

			for (uint i = 0; i < uint(constraints.size()); ++i)
				if ((constraints[i].mVertex[0] == v0 && constraints[i].mVertex[1] == v1)
					|| (constraints[i].mVertex[0] == v1 && constraints[i].mVertex[1] == v0))
					return i;

			constraints.emplace_back(v0, v1, cCompliance);
			return uint(constraints.size() - 1);
		};

		// Rings along the side
		for (uint phi = 0; phi < cNumPhi; ++phi)
			for (uint theta = 0; theta < cNumTheta - 1; ++theta)
			{
				if (theta < cNumTheta - 2)
					sphere_settings->mRodBendTwistConstraints.emplace_back(get_rod(theta, phi, theta + 1, phi), get_rod(theta + 1, phi, theta + 2, phi), cCompliance);
				if (theta > 0 && phi < cNumPhi - 1)
					sphere_settings->mRodBendTwistConstraints.emplace_back(get_rod(theta, phi, theta, phi + 1), get_rod(theta, phi + 1, theta, (phi + 2) % cNumPhi), cCompliance);
			}

		// Close the caps
		for (uint phi1 = 0, phi2 = cNumPhi / 2; phi1 < cNumPhi / 2; ++phi1, phi2 = (phi2 + 1) % cNumPhi)
		{
			sphere_settings->mRodBendTwistConstraints.emplace_back(get_rod(0, phi1, 1, phi1), get_rod(0, phi2, 1, phi2), cCompliance);
			sphere_settings->mRodBendTwistConstraints.emplace_back(get_rod(cNumTheta - 2, phi1, cNumTheta - 1, phi1), get_rod(cNumTheta - 2, phi2, cNumTheta - 1, phi2), cCompliance);
		}

		sphere_settings->CalculateRodProperties();

		// Optimize the settings
		sphere_settings->Optimize();
		SoftBodyCreationSettings sphere(sphere_settings, RVec3(10.0f, 5.0f, 10.0f), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);
	}
}

void SoftBodyBendConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mDebugRenderer->DrawText3D(RVec3(-5.0f, 7.5f, 0), "No bend constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(0.0f, 7.5f, 0), "Distance bend constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(5.0f, 7.5f, 0), "Dihedral angle bend constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(10.0f, 7.5f, 0), "Cosserat rod constraints", Color::sWhite);
}

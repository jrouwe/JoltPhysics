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
}

void SoftBodyBendConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mDebugRenderer->DrawText3D(RVec3(-5.0f, 7.5f, 0), "No bend constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(0.0f, 7.5f, 0), "Distance bend constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(5.0f, 7.5f, 0), "Dihedral angle bend constraints", Color::sWhite);
}

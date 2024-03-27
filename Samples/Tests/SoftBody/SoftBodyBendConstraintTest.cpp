// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyBendConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyBendConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyBendConstraintTest, Test)
}

void SoftBodyBendConstraintTest::Initialize()
{
	CreateFloor();

	auto inv_mass = [](uint, uint inZ) { return inZ < 2? 0.0f : 1.0f; };

	{
		// Cloth without bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, SoftBodySharedSettings::EBendType::None);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(-5.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		// Cloth with edge bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, SoftBodySharedSettings::EBendType::Distance);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(0.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}

	{
		// Cloth with dihedral bend constraints
		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, SoftBodySharedSettings::EBendType::Dihedral);
		SoftBodyCreationSettings cloth(cloth_settings, RVec3(5.0f, 5.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}
}

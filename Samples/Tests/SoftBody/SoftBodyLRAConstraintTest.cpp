// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyLRAConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyLRAConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyLRAConstraintTest, Test)
}

void SoftBodyLRAConstraintTest::Initialize()
{
	CreateFloor();

	// Cloth without LRA constraints
	auto inv_mass = [](uint, uint inZ) { return inZ == 0? 0.0f : 1.0f; };
	Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass);
	for (SoftBodySharedSettings::Edge &e : cloth_settings->mEdgeConstraints)
		e.mCompliance = 1.0e-3f; // Soften the edges a bit so that the effect of the LRA constraints is more visible
	SoftBodyCreationSettings cloth(cloth_settings, RVec3(-10.0f, 25.0f, 0), Quat::sIdentity(), Layers::MOVING);
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// Cloth with LRA constraints
	Ref<SoftBodySharedSettings> lra_cloth_settings = cloth_settings->Clone();
	auto get_vertex = [](uint inX, uint inZ) { return inX + inZ * cNumVerticesX; };
	for (int z = 1; z < cNumVerticesZ; ++z)
		for (int x = 0; x < cNumVerticesX; ++x)
			lra_cloth_settings->mLRAConstraints.push_back(SoftBodySharedSettings::LRA(get_vertex(x, 0), get_vertex(x, z), 0.0f));
	lra_cloth_settings->CalculateLRALengths();
	SoftBodyCreationSettings lra_cloth(lra_cloth_settings, RVec3(10.0f, 25.0f, 0), Quat::sIdentity(), Layers::MOVING);
	mBodyInterface->CreateAndAddSoftBody(lra_cloth, EActivation::Activate);
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyLRAConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyLRAConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyLRAConstraintTest, Test)
}

void SoftBodyLRAConstraintTest::Initialize()
{
	CreateFloor();

	for (int i = 0; i < 2; ++i)
	{
		auto inv_mass = [](uint, uint inZ) { return inZ == 0? 0.0f : 1.0f; };
		auto perturbation = [](uint, uint) { return Vec3::sZero(); };

		SoftBodySharedSettings::VertexAttributes va;
		va.mShearCompliance = va.mCompliance = 1.0e-3f; // Soften the edges a bit so that the effect of the LRA constraints is more visible
		va.mLRAType = i == 0? SoftBodySharedSettings::ELRAType::None : SoftBodySharedSettings::ELRAType::EuclideanDistance;

		Ref<SoftBodySharedSettings> cloth_settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass, perturbation, SoftBodySharedSettings::EBendType::None, va);

		SoftBodyCreationSettings cloth(cloth_settings, RVec3(-10.0f + i * 20.0f, 25.0f, 0), Quat::sIdentity(), Layers::MOVING);
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
	}
}

void SoftBodyLRAConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	mDebugRenderer->DrawText3D(RVec3(-10, 26, -0.5f * cNumVerticesZ * cVertexSpacing), "Without LRA constraints", Color::sWhite);
	mDebugRenderer->DrawText3D(RVec3(10, 26, -0.5f * cNumVerticesZ * cVertexSpacing), "With LRA constraints", Color::sWhite);
}

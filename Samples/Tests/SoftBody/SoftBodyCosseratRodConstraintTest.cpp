// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyCosseratRodConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
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

	constexpr float cRadius = 0.5f;
	constexpr int cNumVertices = 128;
	constexpr float cHeight = 5.0f;
	constexpr float cNumCycles = 10;

	// Create a helix
	Ref<SoftBodySharedSettings> settings = new SoftBodySharedSettings;
	for (int i = 0; i < cNumVertices; ++i)
	{
		float fraction = float(i) / (cNumVertices - 1);

		SoftBodySharedSettings::Vertex v;
		float alpha = cNumCycles * 2.0f * JPH_PI * fraction;
		v.mPosition = Float3(cRadius * Sin(alpha), 0.5f * (1.0f - fraction * cHeight), cRadius * Cos(alpha));
		v.mInvMass = i == 0? 0.0f : 1.0f;
		settings->mVertices.push_back(v);

		if (i > 0)
			settings->mRodStretchShearConstraints.push_back(SoftBodySharedSettings::RodStretchShear(i - 1, i));

		if (i > 1)
			settings->mRodBendTwistConstraints.push_back(SoftBodySharedSettings::RodBendTwist(i - 2, i - 1));
	}

	settings->CalculateRodProperties();
	settings->Optimize();

	SoftBodyCreationSettings helix(settings, RVec3(0, 10, 0), Quat::sIdentity(), Layers::MOVING);
	mBodyInterface->CreateAndAddSoftBody(helix, EActivation::Activate);
}

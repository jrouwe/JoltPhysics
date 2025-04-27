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

	constexpr float cRadius = 2.0f;
	constexpr int cNumVertices = 50;

	// Create a helix
	Ref<SoftBodySharedSettings> settings = new SoftBodySharedSettings;
	for (int i = 0; i < cNumVertices; ++i)
	{
		SoftBodySharedSettings::Vertex v;
		float alpha = float(i) * 2.0f * JPH_PI / 10.0f;
		v.mPosition = Float3(cRadius * Sin(alpha), 2.5f - i * 0.1f, cRadius * Cos(alpha));
		v.mInvMass = i == 0? 0.0f : 1.0f;
		settings->mVertices.push_back(v);

		if (i > 0)
		{
			SoftBodySharedSettings::Rod r;
			r.mVertex[0] = i - 1;
			r.mVertex[1] = i;
			settings->mRods.push_back(r);
		}

		if (i > 1)
		{
			SoftBodySharedSettings::RodConstraint r;
			r.mRods[0] = i - 2;
			r.mRods[1] = i - 1;
			settings->mRodConstraints.push_back(r);
		}
	}

	// Bilateral interleaving, see Position and Orientation Based Cosserat Rods
	for (size_t i = 1; i < settings->mRodConstraints.size() / 2; i += 2)
		std::swap(settings->mRodConstraints[i], settings->mRodConstraints[settings->mRodConstraints.size() - 1 - i]);

	settings->CalculateRodProperties();
	settings->Optimize();

	SoftBodyCreationSettings helix(settings, RVec3(0, 10, 0), Quat::sIdentity(), Layers::MOVING);
	helix.mAllowSleeping = false;
	mBodyInterface->CreateAndAddSoftBody(helix, EActivation::Activate);
}

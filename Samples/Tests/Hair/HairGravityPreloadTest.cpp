// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Hair/HairGravityPreloadTest.h>
#include <Layers.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HairGravityPreloadTest)
{
	JPH_ADD_BASE_CLASS(HairGravityPreloadTest, Test)
}

const char *HairGravityPreloadTest::sScenes[] =
{
	"Zig Zag",
	"Helix",
	"Horizontal Bar",
};

const char *HairGravityPreloadTest::sSceneName = "Zig Zag";

void HairGravityPreloadTest::Initialize()
{
	// Load shaders
	mHairShaders.Init(mComputeSystem);

	Array<HairSettings::SVertex> hair_vertices;
	Array<HairSettings::SStrand> hair_strands;

	if (strcmp(sSceneName, "Zig Zag") == 0)
	{
		// Create a hanging zig zag
		constexpr float cHoriz = 0.05f;
		constexpr int cNumVertices = 128;
		constexpr float cHeight = 0.5f;
		for (int j = 0; j < 2; ++j)
			for (int i = 0; i < cNumVertices; ++i)
			{
				float fraction = float(i) / (cNumVertices - 1);

				HairSettings::SVertex v;
				v.mPosition = Float3((j == 0? -0.1f : 0.1f) + (i & 1? cHoriz : -cHoriz), (1.0f - fraction) * cHeight, 0);
				v.mInvMass = i == 0? 0.0f : 1.0f;
				hair_vertices.push_back(v);
			}
		hair_strands = { HairSettings::SStrand(0, cNumVertices, 0), HairSettings::SStrand(cNumVertices, 2 * cNumVertices, 1) };
	}
	else if (strcmp(sSceneName, "Helix") == 0)
	{
		// Create a hanging helix
		constexpr float cRadius = 0.05f;
		constexpr int cNumVertices = 128;
		constexpr float cHeight = 0.5f;
		constexpr float cNumCycles = 10;
		for (int j = 0; j < 2; ++j)
			for (int i = 0; i < cNumVertices; ++i)
			{
				float fraction = float(i) / (cNumVertices - 1);

				HairSettings::SVertex v;
				float alpha = cNumCycles * 2.0f * JPH_PI * fraction;
				v.mPosition = Float3((j == 0? -0.1f : 0.1f) + cRadius * Sin(alpha), (1.0f - fraction) * cHeight, cRadius * Cos(alpha));
				v.mInvMass = i == 0? 0.0f : 1.0f;
				hair_vertices.push_back(v);
			}
		hair_strands = { HairSettings::SStrand(0, cNumVertices, 0), HairSettings::SStrand(cNumVertices, 2 * cNumVertices, 1) };
	}
	else if (strcmp(sSceneName, "Horizontal Bar") == 0)
	{
		// Create horizontal bar
		constexpr int cNumVertices = 10;
		for (int j = 0; j < 2; ++j)
			for (int i = 0; i < cNumVertices; ++i)
			{
				HairSettings::SVertex v;
				v.mPosition = Float3(j == 0? -0.1f : 0.1f, 0, 1.0f * float(i));
				v.mInvMass = i == 0? 0.0f : 1.0f;
				hair_vertices.push_back(v);
			}

		hair_strands = { HairSettings::SStrand(0, cNumVertices, 0), HairSettings::SStrand(cNumVertices, 2 * cNumVertices, 0) };
	}

	mHairSettings = new HairSettings;
	HairSettings::Material m;
	m.mGlobalPose = HairSettings::Gradient(0, 0);
	m.mEnableLRA = false; // We're testing gravity preloading, so disable LRA to avoid hitting the stretch limits
	m.mBendCompliance = 1e-8f;
	m.mStretchCompliance = 1e-10f;
	m.mBendComplianceMultiplier = { 1, 100, 100, 1 }; // Non uniform
	m.mGridVelocityFactor = { 0.0f, 0.0f }; // Don't let the grid affect the simulation
	m.mGravityPreloadFactor = 0.0f;
	m.mGravityFactor = { 1.0f, 0.5f, 0.2f, 0.8f }; // Non uniform
	m.mSimulationStrandsFraction = 1.0f;
	mHairSettings->mMaterials.push_back(m);
	m.mGravityPreloadFactor = 1.0f;
	mHairSettings->mMaterials.push_back(m);
	mHairSettings->mSimulationBoundsPadding = Vec3::sReplicate(1.0f);
	mHairSettings->InitRenderAndSimulationStrands(hair_vertices, hair_strands);
	float max_dist_sq = 0.0f;
	mHairSettings->Init(max_dist_sq);
	mHairSettings->InitCompute(mComputeSystem);
	mHair = new Hair(mHairSettings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING); // Ensure hair is rotated
	mHair->Init(mComputeSystem);
	mHair->Update(0.0f, Mat44::sIdentity(), nullptr, *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	mHair->ReadBackGPUState(mComputeQueue);
}

void HairGravityPreloadTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
#ifdef JPH_DEBUG_RENDERER
	Hair::DrawSettings settings;
	settings.mDrawRods = true;
	settings.mDrawUnloadedRods = true;
	mHair->Draw(settings, mDebugRenderer);
#endif // JPH_DEBUG_RENDERER

	// Update the hair
	mHair->Update(inParams.mDeltaTime, Mat44::sIdentity(), nullptr, *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	mComputeQueue->ExecuteAndWait();
	mHair->ReadBackGPUState(mComputeQueue);
}

void HairGravityPreloadTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

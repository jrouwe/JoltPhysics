// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyStressTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Utils/SoftBodyCreator.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyStressTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyStressTest, Test)
}

const char *SoftBodyStressTest::sScenes[] =
{
	"SpheresVsBoxes",
	"LargeCloth"
};

const char *SoftBodyStressTest::sSceneName = "SpheresVsBoxes";

void SoftBodyStressTest::Initialize()
{
	if (strcmp(sSceneName, "SpheresVsBoxes") == 0)
	{
		// Floor
		CreateMeshTerrain();

		// Pressurized sphere settings
		SoftBodyCreationSettings sphere(SoftBodyCreator::CreateSphere(), RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		sphere.mPressure = 2000.0f;

		// Box settings
		BodyCreationSettings box(new BoxShape(Vec3::sReplicate(1.0f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		box.mMassPropertiesOverride.mMass = 100.0f;

		for (int x = 0; x <= 10; ++x)
			for (int z = 0; z <= 10; ++z)
			{
				sphere.mPosition = RVec3(-20.0_r + 4.0_r * x, 5.0_r, -20.0_r + 4.0_r * z);
				mBodyInterface->CreateAndAddSoftBody(sphere, EActivation::Activate);

				box.mPosition = sphere.mPosition + RVec3(0, 4, 0);
				mBodyInterface->CreateAndAddBody(box, EActivation::Activate);
			}
	}
	else if (strcmp(sSceneName, "LargeCloth") == 0)
	{
		// Floor
		CreateFloor();

		// Create cloth that's fixated at the corners
		SoftBodyCreationSettings cloth(SoftBodyCreator::CreateCloth(100, 0.25f), RVec3(0, 15.0f, 0), Quat::sIdentity(), Layers::MOVING);
		cloth.mUpdatePosition = false; // Don't update the position of the cloth as it is fixed to the world
		mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

		// Box settings
		BodyCreationSettings box(new BoxShape(Vec3::sReplicate(0.5f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		box.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		box.mMassPropertiesOverride.mMass = 10.0f;

		// Create a number of boxes that fall on the cloth
		for (int x = 0; x <= 10; ++x)
			for (int z = 0; z <= 10; ++z)
			{
				box.mPosition = cloth.mPosition + RVec3(-10.0_r + 2.0_r * x, 2.0_r, -10.0_r + 2.0_r * z);
				mBodyInterface->CreateAndAddBody(box, EActivation::Activate);
			}
	}
}

void SoftBodyStressTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyVertexRadiusTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyVertexRadiusTest)
{
	JPH_ADD_BASE_CLASS(SoftBodyVertexRadiusTest, Test)
}

void SoftBodyVertexRadiusTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create sphere
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new SphereShape(2.0f), RVec3(0, 0, 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Create cloth with specified vertex radius
	mSharedSettings = SoftBodyCreator::CreateCloth(30, 30, 0.5f);
	mSharedSettings->mVertexRadius = sVertexRadius;
	SoftBodyCreationSettings cloth(mSharedSettings, RVec3(0, 5, 0), Quat::sRotation(Vec3::sAxisY(), 0.25f * JPH_PI), Layers::MOVING);
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);
}

void SoftBodyVertexRadiusTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateSlider(inSubMenu, "Vertex Radius", sVertexRadius, 0.0f, 0.5f, 0.01f, [this](float inValue) { sVertexRadius = inValue; mSharedSettings->mVertexRadius = inValue; });
}

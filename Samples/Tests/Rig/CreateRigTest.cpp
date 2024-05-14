// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/CreateRigTest.h>
#include <Application/DebugUI.h>
#include <Utils/RagdollLoader.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(CreateRigTest)
{
	JPH_ADD_BASE_CLASS(CreateRigTest, Test)
}

CreateRigTest::~CreateRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void CreateRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create ragdoll
	Ref<RagdollSettings> settings = RagdollLoader::sCreate();
	mRagdoll = settings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);
}

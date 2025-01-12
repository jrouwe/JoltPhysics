// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/LoadRigTest.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadRigTest)
{
	JPH_ADD_BASE_CLASS(LoadRigTest, Test)
}

LoadRigTest::ConstraintNameAndType LoadRigTest::sTypes[] =
{
	{ "Fixed",		EConstraintOverride::TypeFixed },
	{ "Point",		EConstraintOverride::TypePoint },
	{ "Hinge",		EConstraintOverride::TypeHinge },
	{ "Slider",		EConstraintOverride::TypeSlider },
	{ "Cone",		EConstraintOverride::TypeCone },
	{ "Ragdoll",	EConstraintOverride::TypeRagdoll },
};

EConstraintOverride LoadRigTest::sConstraintType = EConstraintOverride::TypeRagdoll;

LoadRigTest::~LoadRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void LoadRigTest::Initialize()
{
	// Floor
	CreateFloor();

	// Load ragdoll
	mRagdollSettings = RagdollLoader::sLoad("Human.tof", EMotionType::Dynamic, sConstraintType);

	// Create ragdoll
	mRagdoll = mRagdollSettings->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);
}

void LoadRigTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Constraint Type", [this, inUI]() {
		UIElement *constraint_type = inUI->CreateMenu();
		for (uint i = 0; i < size(sTypes); ++i)
			inUI->CreateTextButton(constraint_type, sTypes[i].mName, [this, i]() { sConstraintType = sTypes[i].mType; RestartTest(); });
		inUI->ShowMenu(constraint_type);
	});
}

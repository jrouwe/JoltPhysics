// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>
#include <Utils/RagdollLoader.h>

// This test loads a ragdoll from disc and simulates it
class LoadRigTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(LoadRigTest)

	// Destructor
	virtual							~LoadRigTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float					GetWorldScale() const override								{ return 0.2f; }

	virtual void					Initialize() override;

	// Optional settings menu
	virtual bool					HasSettingsMenu() const override							{ return true; }
	virtual void					CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// Our ragdoll
	Ref<RagdollSettings>			mRagdollSettings;
	Ref<Ragdoll>					mRagdoll;

	struct ConstraintNameAndType
	{
		const char *				mName;
		EConstraintOverride			mType;
	};

	// List of possible constraint types and their names
	static ConstraintNameAndType	sTypes[];

	// Type of constraints to create for this test
	static EConstraintOverride		sConstraintType;
};

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

class RigPileTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, RigPileTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Tests the performance of a pile of ragdolls on a terrain.";
	}

	// Destructor
	virtual					~RigPileTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

	virtual void			Initialize() override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;

	// Number of ragdolls per pile
	static int				sPileSize;

	// Number of piles per axis
	static int				sNumPilesPerAxis;

	// All active ragdolls
	Array<Ref<Ragdoll>>		mRagdolls;
};

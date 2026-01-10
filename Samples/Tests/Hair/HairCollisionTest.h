// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Hair/Hair.h>
#include <Jolt/Physics/Hair/HairShaders.h>

class HairCollisionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HairCollisionTest)

	// Destructor
	virtual					~HairCollisionTest() override											{ delete mHair; }

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Hair collision demo.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override											{ return 0.01f; }

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override										{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	inline static bool		sRotating = false;

	Ref<HairSettings>		mHairSettings = nullptr;
	HairShaders				mHairShaders;
	Hair *					mHair = nullptr;
	uint32					mFrame = 0;
	BodyID					mMovingBodyID;
};

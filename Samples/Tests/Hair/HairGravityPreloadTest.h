// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Hair/Hair.h>
#include <Jolt/Physics/Hair/HairShaders.h>

class HairGravityPreloadTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HairGravityPreloadTest)

	// Destructor
	virtual					~HairGravityPreloadTest() override										{ delete mHair; }

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Hair gravity preloading demo. This prevents the hair from sagging at the start of the simulation.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override											{ return 0.01f; }

private:
	Ref<HairSettings>		mHairSettings = nullptr;
	HairShaders				mHairShaders;
	Hair *					mHair = nullptr;
};

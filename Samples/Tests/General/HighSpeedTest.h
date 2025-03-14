// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class HighSpeedTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HighSpeedTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Spawns a number of high speed objects to check that they don't tunnel through geometry.";
	}

	// See: Test
	virtual void		Initialize() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float		GetWorldScale() const override									{ return sSelectedScene == 0? 1.0f : 0.2f; }

	// Optional settings menu
	virtual bool		HasSettingsMenu() const override								{ return true; }
	virtual void		CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	void				CreateDynamicObject(RVec3Arg inPosition, Vec3Arg inVelocity, Shape *inShape, EMotionQuality inMotionQuality = EMotionQuality::LinearCast);
	void				CreateDominoBlocks(RVec3Arg inOffset, int inNumWalls, float inDensity, float inRadius);
	void				CreateFastSmallConvexObjects();
	void				CreateSimpleScene();
	void				CreateConvexOnLargeTriangles();
	void				CreateConvexOnTerrain1();

	static const char *	sScenes[];

	static int			sSelectedScene;
};

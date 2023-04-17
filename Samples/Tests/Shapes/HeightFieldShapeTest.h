// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

class HeightFieldShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HeightFieldShapeTest)

	// Initialize the test
	virtual void		Initialize() override;

	// Update the test, called before the physics update
	virtual void		PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void		GetInitialCamera(CameraState &ioState) const override;

	// Optional settings menu
	virtual bool		HasSettingsMenu() const	override							{ return true; }
	virtual void		CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	// Original (uncompressed) terrain
	Array<float>		mTerrain;
	PhysicsMaterialList mMaterials;
	Array<uint8>		mMaterialIndices;
	uint				mTerrainSize;
	Vec3				mTerrainOffset;
	Vec3				mTerrainScale;

	// Block size = 1 << sBlockSizeShift
	inline static int	sBlockSizeShift = 2;

	// Bits per sample
	inline static int	sBitsPerSample = 8;

	// Draw the terrain
	inline static bool	sShowOriginalTerrain = false;

	RefConst<HeightFieldShape> mHeightField;

	RVec3				mHitPos = RVec3::sZero();
};

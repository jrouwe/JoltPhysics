// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Physics/Collision/Shape/HeightFieldShape.h>

class HeightFieldShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(HeightFieldShapeTest)

	// Initialize the test
	virtual void	Initialize() override;

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void	GetInitialCamera(CameraState &ioState) const override;

	// Optional settings menu
	virtual bool	HasSettingsMenu() const	override							{ return true; }
	virtual void	CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

	RefConst<HeightFieldShape> mHeightField;
	Vec3			mHitPos = Vec3::sZero();
};
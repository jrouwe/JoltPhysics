// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

// This test will make a dynamic body cyle through various shapes
class ChangeShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ChangeShapeTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	bool					mActivateAfterSwitch = true;
	BodyID					mBodyID;
	Array<RefConst<Shape>> 	mShapes;
	float					mTime = 0.0f;
	int						mShapeIdx = 0;
};

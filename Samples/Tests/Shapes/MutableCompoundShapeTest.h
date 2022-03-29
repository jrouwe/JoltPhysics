// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class MutableCompoundShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(MutableCompoundShapeTest)

	// See: Test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;

private:
	BodyIDVector			mBodyIDs;

	RefConst<Shape>			mSubCompound;

	int						mFrameNumber = 0;
};

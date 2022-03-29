// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>

class SwingTwistConstraintFrictionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(SwingTwistConstraintFrictionTest)

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void			SaveState(StateRecorder &inStream) const override;
	virtual void			RestoreState(StateRecorder &inStream) override;
	
private:
	float					mTime = 0.0f;
	SwingTwistConstraint *	mConstraint = nullptr;
};

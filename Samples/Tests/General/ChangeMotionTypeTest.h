// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/Body.h>

// This test will switch a body between static, kinematic and dynamic
class ChangeMotionTypeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ChangeMotionTypeTest)

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void	SaveState(StateRecorder &inStream) const override;
	virtual void	RestoreState(StateRecorder &inStream) override;

private:
	Body *			mBody = nullptr;
	float			mTime = 0.0f;
};

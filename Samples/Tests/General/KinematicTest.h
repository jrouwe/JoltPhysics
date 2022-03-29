// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/Body.h>

// This test tests kinematic objects against a pile of dynamic boxes
class KinematicTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(KinematicTest)

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	Body *			mKinematic[2];
};

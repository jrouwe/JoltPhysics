// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Does a very long capsule vs rotated embedded box test, this was a repro for a bug and can be used to test bug regression
class CapsuleVsBoxTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(CapsuleVsBoxTest)

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
};

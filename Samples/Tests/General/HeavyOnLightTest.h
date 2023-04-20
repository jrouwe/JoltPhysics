// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test spawns a number of heavy boxes (with increasing weight) on smaller boxes to see how the simulation handles this
class HeavyOnLightTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, HeavyOnLightTest)

	// See: Test
	virtual void		Initialize() override;
};

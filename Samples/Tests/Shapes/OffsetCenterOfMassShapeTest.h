// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Tests the OffsetCenterOfMass shape
class OffsetCenterOfMassShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(OffsetCenterOfMassShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

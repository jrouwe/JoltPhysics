// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ScaledCapsuleShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ScaledCapsuleShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

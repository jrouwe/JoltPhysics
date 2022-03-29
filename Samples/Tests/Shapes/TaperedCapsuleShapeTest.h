// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class TaperedCapsuleShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(TaperedCapsuleShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

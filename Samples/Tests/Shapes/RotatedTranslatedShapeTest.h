// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Tests the RotatedTranslated shape
class RotatedTranslatedShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(RotatedTranslatedShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

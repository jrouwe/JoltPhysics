// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Tests the RotatedTranslated shape
class RotatedTranslatedShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, RotatedTranslatedShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

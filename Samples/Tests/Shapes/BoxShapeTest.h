// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class BoxShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(BoxShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

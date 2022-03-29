// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ScaledStaticCompoundShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ScaledStaticCompoundShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class StaticCompoundShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(StaticCompoundShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

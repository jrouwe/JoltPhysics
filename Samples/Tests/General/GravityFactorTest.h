// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests various gravity factors
class GravityFactorTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(GravityFactorTest)

	// See: Test
	virtual void		Initialize() override;
};

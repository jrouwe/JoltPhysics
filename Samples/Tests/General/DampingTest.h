// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests various values for linear and angular damping
class DampingTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(DampingTest)

	// See: Test
	virtual void		Initialize() override;
};

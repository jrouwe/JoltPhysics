// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test demonstrates the use of a gear constraint
class GearConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(GearConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

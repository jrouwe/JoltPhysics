// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Demonstrates the pulley constraint
class PulleyConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(PulleyConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

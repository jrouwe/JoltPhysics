// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test stacks a number of boxes to see if the simulation is stable
class StackTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(StackTest)

	// See: Test
	virtual void		Initialize() override;
};

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests various values for friction
class FrictionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(FrictionTest)

	// See: Test
	virtual void		Initialize() override;
};

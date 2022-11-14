// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This is a test that shows how to create a 2D simulation
class TwoDFunnelTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(TwoDFunnelTest)

	// See: Test
	virtual void		Initialize() override;
};

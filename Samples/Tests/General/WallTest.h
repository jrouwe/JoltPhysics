// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests a large pile of boxes to check stacking and performance behavior.
class WallTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(WallTest)

	// See: Test
	virtual void		Initialize() override;
};

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class HingeConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(HingeConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ConeConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ConeConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

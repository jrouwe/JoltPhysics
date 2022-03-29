// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests various different restitution values
class RestitutionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(RestitutionTest)

	// See: Test
	virtual void		Initialize() override;
};

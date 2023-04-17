// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test demonstrates the use of a rack and pinion constraint
class RackAndPinionConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, RackAndPinionConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

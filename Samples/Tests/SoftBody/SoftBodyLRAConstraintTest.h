// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test shows the effect of LRA constraints in a soft body which can help reduce stretch of the cloth. The left cloth uses no LRA constraints and the right one does.
class SoftBodyLRAConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyLRAConstraintTest)

	// See: Test
	virtual void		Initialize() override;
};

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Tests a small box falling on a big triangle to test for numerical precision errors.
class BigVsSmallTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BigVsSmallTest)

	// See: Test
	virtual void		Initialize() override;
};

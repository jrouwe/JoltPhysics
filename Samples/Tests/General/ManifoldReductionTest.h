// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test shows how many coplanar triangles are reduced to a single contact manifold
class ManifoldReductionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ManifoldReductionTest)

	// See: Test
	virtual void		Initialize() override;
};

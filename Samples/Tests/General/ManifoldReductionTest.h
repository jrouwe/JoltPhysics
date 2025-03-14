// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ManifoldReductionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ManifoldReductionTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"This test shows how many coplanar triangles are reduced to a single contact manifold.\n"
				"The static geometry in this test consists of a high density triangle grid.";
	}

	// See: Test
	virtual void		Initialize() override;
};

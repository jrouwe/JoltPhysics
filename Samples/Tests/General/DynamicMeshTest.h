// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class DynamicMeshTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, DynamicMeshTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"Drops a dynamic body with a mesh shape on a pile of boxes.\n"
				"Note that mesh vs mesh collisions are currently not supported.";
	}

	// See: Test
	virtual void		Initialize() override;
};

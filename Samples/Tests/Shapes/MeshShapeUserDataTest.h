// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class MeshShapeUserDataTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, MeshShapeUserDataTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows how to store per triangle user data in a mesh shape and how to retrieve it.";
	}

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
};

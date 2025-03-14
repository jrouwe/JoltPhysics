// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyLRAConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyLRAConstraintTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Shows the effect of Long Range Attachment (LRA) constraints in a soft body which can help reduce cloth stretching.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	// Size and spacing of the cloth
	static constexpr int	cNumVerticesX = 10;
	static constexpr int	cNumVerticesZ = 50;
	static constexpr float	cVertexSpacing = 0.5f;
};

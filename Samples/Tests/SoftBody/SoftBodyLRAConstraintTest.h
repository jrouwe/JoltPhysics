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
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	// Size and spacing of the cloth
	static constexpr int	cNumVerticesX = 10;
	static constexpr int	cNumVerticesZ = 50;
	static constexpr float	cVertexSpacing = 0.5f;
};

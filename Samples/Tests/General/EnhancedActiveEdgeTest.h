// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

/// Demonstrates the enhanced active edge mode by sliding two shapes over a mesh that has only active edges
class EnhancedActiveEdgeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, EnhancedActiveEdgeTest)

	// See: Test
	virtual void	Initialize() override;
};

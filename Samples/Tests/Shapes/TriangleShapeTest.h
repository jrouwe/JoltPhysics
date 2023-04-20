// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class TriangleShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, TriangleShapeTest)

	// See: Test
	virtual void	Initialize() override;
};

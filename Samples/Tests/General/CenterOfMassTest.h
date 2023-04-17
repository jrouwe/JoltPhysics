// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Test that spawns various shapes with the center of mass not in the center of the object
class CenterOfMassTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, CenterOfMassTest)

	// See: Test
	virtual void		Initialize() override;
};

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test spawns objects at an angle of each other to test if the contact manifold is calculated correctly
class ContactManifoldTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ContactManifoldTest)

	// See: Test
	virtual void		Initialize() override;
};

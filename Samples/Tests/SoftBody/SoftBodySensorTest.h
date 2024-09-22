// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/SoftBody/SoftBodyContactListener.h>

// This test shows interaction between a soft body and a sensor
class SoftBodySensorTest : public Test, public SoftBodyContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodySensorTest)

	// See: Test
	virtual void		Initialize() override;

	// See: SoftBodyContactListener
	virtual void		OnSoftBodyContactAdded(const Body &inSoftBody, const SoftBodyManifold &inManifold) override;
};

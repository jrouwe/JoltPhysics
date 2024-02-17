// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/SoftBody/SoftBodyContactListener.h>

// This test shows how to use contact listeners for soft bodies
class SoftBodyContactListenerTest : public Test, public SoftBodyContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyContactListenerTest)

	// See: Test
	virtual void					Initialize() override;

	// See: SoftBodyContactListener
	virtual SoftBodyValidateResult	OnSoftBodyContactValidate(const Body &inSoftBody, const Body &inOtherBody, SoftBodyContactSettings &ioSettings) override;
	virtual void					OnSoftBodyContactAdded(const Body &inSoftBody, const Body &inOtherBody, const SoftBodyManifold &inManifold) override;

private:
	BodyIDVector					mBodies;
};

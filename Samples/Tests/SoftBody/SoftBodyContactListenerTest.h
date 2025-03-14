// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/SoftBody/SoftBodyContactListener.h>

class SoftBodyContactListenerTest : public Test, public SoftBodyContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyContactListenerTest)

	// Description of the test
	virtual const char *			GetDescription() const override
	{
		return "Shows how to use contact listeners for soft bodies to affect the simulation.";
	}

	// See: Test
	virtual void					Initialize() override;
	virtual void					PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void					GetInitialCamera(CameraState &ioState) const override		{ ioState.mPos = RVec3(15, 10, 15); }

	// Test is not deterministic as it creates/removes bodies in a way that's not compatible with the determinism check
	virtual bool					IsDeterministic() const override							{ return false; }

	// See: SoftBodyContactListener
	virtual SoftBodyValidateResult	OnSoftBodyContactValidate(const Body &inSoftBody, const Body &inOtherBody, SoftBodyContactSettings &ioSettings) override;
	virtual void					OnSoftBodyContactAdded(const Body &inSoftBody, const SoftBodyManifold &inManifold) override;

private:
	void							UpdateLabel();

	void							StartCycle();

	float							mTime = 0.0f;
	int								mCycle = 0;
	BodyID							mSoftBodyID;
	BodyID							mOtherBodyID;
};

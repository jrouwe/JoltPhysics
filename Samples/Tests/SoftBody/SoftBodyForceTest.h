// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyForceTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyForceTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows how to apply a global force to a soft body.";
	}

	// See: Test
	virtual void		Initialize() override;
	virtual void		PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void		SaveState(StateRecorder &inStream) const override;
	virtual void		RestoreState(StateRecorder &inStream) override;

private:
	float				mTime = 0.0f;
	BodyID				mBodyID;
};

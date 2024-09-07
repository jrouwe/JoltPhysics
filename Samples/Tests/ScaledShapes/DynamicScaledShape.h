// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Demonstrates how you can scale a shape dynamically while a body is being simulated
class DynamicScaledShape : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, DynamicScaledShape)

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Saving / restoring state for replay
	virtual void	SaveState(StateRecorder &inStream) const override;
	virtual void	RestoreState(StateRecorder &inStream) override;

private:
	BodyID			mBodyID;
	float			mTime = 0.0f;
};

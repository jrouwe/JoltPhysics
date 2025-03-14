// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/Body.h>

class ChangeMotionQualityTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ChangeMotionQualityTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"Switches a body's motion quality from linear to discrete.\n"
				"After the switch, the high speed body passes through the wall.";
	}

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void	SaveState(StateRecorder &inStream) const override;
	virtual void	RestoreState(StateRecorder &inStream) override;

private:
	void			UpdateMotionQuality();

	Body *			mBody = nullptr;
	float			mTime = 0.0f;
};

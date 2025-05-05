// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyCosseratRodConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyCosseratRodConstraintTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return "Shows the effect of Cosserat rod constraints in a soft body that control bend, twist and shear between particles.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	BodyIDVector			mSoftBodies;
};

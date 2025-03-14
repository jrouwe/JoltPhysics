// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Ragdoll/Ragdoll.h>

class LoadSaveBinaryRigTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, LoadSaveBinaryRigTest)

	// Description of the test
	virtual const char *			GetDescription() const override
	{
		return "Loads a ragdoll from disc, writes it to a binary stream, loads it again and simulates it.";
	}

	// Destructor
	virtual							~LoadSaveBinaryRigTest() override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float					GetWorldScale() const override								{ return 0.2f; }

	virtual void					Initialize() override;

private:
	// Our ragdoll
	Ref<Ragdoll>					mRagdoll;
};

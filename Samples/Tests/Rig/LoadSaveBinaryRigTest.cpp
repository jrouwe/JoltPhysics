// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Rig/LoadSaveBinaryRigTest.h>
#include <Application/DebugUI.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Utils/Log.h>
#include <Utils/RagdollLoader.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSaveBinaryRigTest)
{
	JPH_ADD_BASE_CLASS(LoadSaveBinaryRigTest, Test)
}

LoadSaveBinaryRigTest::~LoadSaveBinaryRigTest()
{
	mRagdoll->RemoveFromPhysicsSystem();
}

void LoadSaveBinaryRigTest::Initialize()
{
	// Floor
	CreateFloor();

	stringstream data;

	{
		// Load ragdoll
		Ref<RagdollSettings> settings = RagdollLoader::sLoad("Assets/Human.tof", EMotionType::Dynamic);

		// Save it to a binary stream
		StreamOutWrapper stream_out(data);
		settings->SaveBinaryState(stream_out, true /* Save shape */, true /* Save group filter */);
	}

	StreamInWrapper stream_in(data);
	RagdollSettings::RagdollResult result = RagdollSettings::sRestoreFromBinaryState(stream_in);
	if (result.HasError())
		FatalError(result.GetError().c_str());

	// Create ragdoll
	mRagdoll = result.Get()->CreateRagdoll(0, 0, mPhysicsSystem);
	mRagdoll->AddToPhysicsSystem(EActivation::Activate);
}

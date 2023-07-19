// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ChangeObjectLayerTest.h>
#include <Layers.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ChangeObjectLayerTest)
{
	JPH_ADD_BASE_CLASS(ChangeObjectLayerTest, Test)
}

void ChangeObjectLayerTest::Initialize()
{
	// Floor
	CreateFloor();

	// A dynamic box in the MOVING layer
	mMoving = mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(5, 0.1f, 5)), RVec3(0, 1.5f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Lots of dynamic objects in the DEBRIS layer
	default_random_engine random;
	uniform_real_distribution<float> position_variation(-10, 10);
	for (int i = 0; i < 50; ++i)
	{
		RVec3 position(position_variation(random), 2.0f, position_variation(random));
		Quat rotation = Quat::sRandom(random);
		mDebris.push_back(mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3::sReplicate(0.1f)), position, rotation, EMotionType::Dynamic, Layers::DEBRIS), EActivation::Activate));
	}
}

void ChangeObjectLayerTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	const float cSwitchTime = 2.0f;

	// Increment time
	mTime += inParams.mDeltaTime;

	if (mTime >= cSwitchTime)
	{
		mIsDebris = !mIsDebris;

		// Reposition moving object
		mBodyInterface->SetPosition(mMoving, RVec3(0, 1.5f, 0), EActivation::Activate);

		default_random_engine random;
		uniform_real_distribution<float> position_variation(-7.5f, 7.5f);
		for (BodyID id : mDebris)
		{
			// Reposition debris
			RVec3 position(position_variation(random), 2.0f, position_variation(random));
			Quat rotation = Quat::sRandom(random);
			mBodyInterface->SetPositionAndRotation(id, position, rotation, EActivation::Activate);

			// And update layer
			mBodyInterface->SetObjectLayer(id, mIsDebris? Layers::DEBRIS : Layers::MOVING);
		}

		mTime = 0;
	}
}

void ChangeObjectLayerTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
	inStream.Write(mIsDebris);
}

void ChangeObjectLayerTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
	inStream.Read(mIsDebris);

	// Restore layer
	for (BodyID id : mDebris)
		mBodyInterface->SetObjectLayer(id, mIsDebris? Layers::DEBRIS : Layers::MOVING);
}

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Tools/LoadSnapshotTest.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Utils/Log.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(LoadSnapshotTest) 
{ 
	JPH_ADD_BASE_CLASS(LoadSnapshotTest, Test) 
}

void LoadSnapshotTest::Initialize()
{
	ifstream stream("snapshot.bin", ifstream::in | ifstream::binary);
	if (!stream.is_open()) 
		FatalError("Unable to open 'snapshot.bin'");

	StreamInWrapper wrapper(stream);
	PhysicsScene::PhysicsSceneResult result = PhysicsScene::sRestoreFromBinaryState(wrapper);
	if (result.HasError())
		FatalError(result.GetError().c_str());

	result.Get()->CreateBodies(mPhysicsSystem);
}

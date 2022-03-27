// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ChangeMotionTypeTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ChangeMotionTypeTest) 
{ 
	JPH_ADD_BASE_CLASS(ChangeMotionTypeTest, Test) 
}

void ChangeMotionTypeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Create body as static, but allow to become dynamic
	BodyCreationSettings settings;
	settings.SetShape(new BoxShape(Vec3(0.5f, 1.0f, 2.0f)));
	settings.mPosition = Vec3(0, 10, 0);
	settings.mMotionType = EMotionType::Static; 
	settings.mObjectLayer = Layers::MOVING; // Put in moving layer, this will result in some overhead when the body is static
	settings.mAllowDynamicOrKinematic = true;
	mBody = mBodyInterface->CreateBody(settings);
	mBodyInterface->AddBody(mBody->GetID(), EActivation::DontActivate);
}

void ChangeMotionTypeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{ 
	// Increment time
	mTime += inParams.mDeltaTime;

	// Calculate desired motion type
	static const EMotionType cycle[] = { EMotionType::Dynamic, EMotionType::Kinematic, EMotionType::Static, EMotionType::Kinematic, EMotionType::Dynamic, EMotionType::Static };
	EMotionType motion_type = cycle[int(mTime) % size(cycle)];

	// Update motion type and reactivate the body
	if (motion_type != mBody->GetMotionType())
		mBodyInterface->SetMotionType(mBody->GetID(), motion_type, EActivation::Activate);

	// Provide kinematic body a target
	if (motion_type == EMotionType::Kinematic)
		mBody->MoveKinematic(Vec3(sin(mTime), 10, 0), Quat::sRotation(Vec3::sAxisX(), cos(mTime)), inParams.mDeltaTime);
}

void ChangeMotionTypeTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void ChangeMotionTypeTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}

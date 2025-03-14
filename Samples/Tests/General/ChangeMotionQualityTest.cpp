// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/ChangeMotionQualityTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ChangeMotionQualityTest)
{
	JPH_ADD_BASE_CLASS(ChangeMotionQualityTest, Test)
}

void ChangeMotionQualityTest::Initialize()
{
	// Floor
	CreateFloor();

	// Single shape that has 4 walls to surround fast moving sphere
	BodyCreationSettings enclosing_settings;
	Ref<BoxShapeSettings> box_shape = new BoxShapeSettings(Vec3(5.0f, 1.0f, 0.1f));
	Ref<StaticCompoundShapeSettings> enclosing_shape = new StaticCompoundShapeSettings();
	enclosing_shape->AddShape(Vec3(0, 0, 5), Quat::sIdentity(), box_shape);
	enclosing_shape->AddShape(Vec3(0, 0, -5), Quat::sIdentity(), box_shape);
	enclosing_shape->AddShape(Vec3(5, 0, 0), Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI), box_shape);
	enclosing_shape->AddShape(Vec3(-5, 0, 0), Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI), box_shape);
	enclosing_settings.SetShapeSettings(enclosing_shape);
	enclosing_settings.mMotionType = EMotionType::Kinematic;
	enclosing_settings.mObjectLayer = Layers::MOVING;
	enclosing_settings.mPosition = RVec3(0, 1, 0);
	mBodyInterface->CreateAndAddBody(enclosing_settings, EActivation::Activate);

	// Create high speed sphere inside
	BodyCreationSettings settings;
	settings.SetShape(new SphereShape(1.0f));
	settings.mPosition = RVec3(0, 0.5f, 0);
	settings.mMotionType = EMotionType::Dynamic;
	settings.mLinearVelocity = Vec3(-240, 0, -120);
	settings.mFriction = 0.0f;
	settings.mRestitution = 1.0f;
	settings.mObjectLayer = Layers::MOVING;
	mBody = mBodyInterface->CreateBody(settings);
	mBodyInterface->AddBody(mBody->GetID(), EActivation::Activate);

	UpdateMotionQuality();
}

void ChangeMotionQualityTest::UpdateMotionQuality()
{
	static EMotionQuality qualities[] = { EMotionQuality::LinearCast, EMotionQuality::Discrete };
	static const char *labels[] = { "LinearCast", "Discrete" };

	// Calculate desired motion quality
	int idx = int(mTime) & 1;
	mBodyInterface->SetMotionQuality(mBody->GetID(), qualities[idx]);
	SetBodyLabel(mBody->GetID(), labels[idx]);
}

void ChangeMotionQualityTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Increment time
	mTime += inParams.mDeltaTime;

	UpdateMotionQuality();
}

void ChangeMotionQualityTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void ChangeMotionQualityTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);

	UpdateMotionQuality();
}

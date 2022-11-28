#include <TestFramework.h>

#include <Tests/General/ChangeMotionQualityTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
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

	BodyCreationSettings settings;
	settings.SetShape(new BoxShape(Vec3(0.5f, 1.0f, 2.0f)));
	settings.mPosition = Vec3(0, 10, 0);
	settings.mMotionType = EMotionType::Dynamic;
	settings.mMotionQuality = EMotionQuality::Discrete;
	settings.mObjectLayer = Layers::MOVING;
	mBody = mBodyInterface->CreateBody(settings);
	mBodyInterface->AddBody(mBody->GetID(), EActivation::Activate);
}

void ChangeMotionQualityTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Increment time
	mTime += inParams.mDeltaTime;

	// Calculate desired motion quality
	static const EMotionQuality cycle[] = { EMotionQuality::LinearCast, EMotionQuality::Discrete, EMotionQuality::LinearCast };
	EMotionQuality motion_quality = cycle[int(mTime) % size(cycle)];

	BodyLockWrite body_lock(mPhysicsSystem->GetBodyLockInterface(), mBody->GetID());
	if (body_lock.Succeeded())
	{
		// Update motion quality
		JPH::MotionProperties* motion_properties = mBody->GetMotionProperties();
		if (motion_quality != motion_properties->GetMotionQuality())
			motion_properties->SetMotionQuality(motion_quality);
	}
}

void ChangeMotionQualityTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void ChangeMotionQualityTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}

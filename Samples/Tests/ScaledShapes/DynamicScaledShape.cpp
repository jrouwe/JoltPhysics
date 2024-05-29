// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/DynamicScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DynamicScaledShape)
{
	JPH_ADD_BASE_CLASS(DynamicScaledShape, Test)
}

void DynamicScaledShape::Initialize()
{
	// Floor
	CreateHeightFieldTerrain();

	// Create scaled sphere
	RefConst<Shape> scaled_sphere_shape = new ScaledShape(new SphereShape(2.0f), Vec3::sReplicate(1.0f));
	mBodyID = mBodyInterface->CreateAndAddBody(BodyCreationSettings(scaled_sphere_shape, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}

void DynamicScaledShape::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), mBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Fetch the inner shape
		// Note that we know here that the inner shape is the original shape, but if you're scaling a CompoundShape non-uniformly the inner shape
		// may be a new compound shape with the scale baked into the children. In this case you need to keep track of your original shape yourself.
		JPH_ASSERT(body.GetShape()->GetSubType() == EShapeSubType::Scaled);
		const ScaledShape *scaled_shape = static_cast<const ScaledShape *>(body.GetShape());
		const Shape *non_scaled_shape = scaled_shape->GetInnerShape();

		// Rescale the sphere
		float new_scale = 1.0f + 0.5f * Sin(mTime);
		Shape::ShapeResult new_shape = non_scaled_shape->ScaleShape(Vec3::sReplicate(new_scale));
		JPH_ASSERT(new_shape.IsValid()); // We're uniformly scaling a sphere, this should always succeed

		// Note: Using non-locking interface here because we already have the lock
		// Also note that scaling shapes may cause simulation issues as the bodies can get stuck when they get bigger.
		// Recalculating mass every frame can also be an expensive operation.
		mPhysicsSystem->GetBodyInterfaceNoLock().SetShape(body.GetID(), new_shape.Get(), true, EActivation::Activate);
	}
}

void DynamicScaledShape::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void DynamicScaledShape::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}

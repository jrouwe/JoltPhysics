// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhase.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>

JPH_NAMESPACE_BEGIN

Body *BodyInterface::CreateBody(const BodyCreationSettings &inSettings)
{
	return mBodyManager->CreateBody(inSettings);
}

void BodyInterface::DestroyBody(const BodyID &inBodyID)
{
	mBodyManager->DestroyBodies(&inBodyID, 1);
}

void BodyInterface::DestroyBodies(const BodyID *inBodyIDs, int inNumber)
{
	mBodyManager->DestroyBodies(inBodyIDs, inNumber);
}

void BodyInterface::AddBody(const BodyID &inBodyID, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();

		// Add to broadphase
		BodyID id = inBodyID;
		BroadPhase::AddState add_state = mBroadPhase->AddBodiesPrepare(&id, 1);
		mBroadPhase->AddBodiesFinalize(&id, 1, add_state);

		// Optionally activate body
		if (inActivationMode == EActivation::Activate && !body.IsStatic())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::RemoveBody(const BodyID &inBodyID)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();

		// Deactivate body
		if (body.IsActive())
			mBodyManager->DeactivateBodies(&inBodyID, 1);
	
		// Remove from broadphase
		BodyID id = inBodyID;
		mBroadPhase->RemoveBodies(&id, 1);
	}
}

bool BodyInterface::IsAdded(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	return lock.SucceededAndIsInBroadPhase();
}

BodyID BodyInterface::CreateAndAddBody(const BodyCreationSettings &inSettings, EActivation inActivationMode)
{
	const Body *b = CreateBody(inSettings);
	if (b == nullptr)
		return BodyID(); // Out of bodies
	AddBody(b->GetID(), inActivationMode);
	return b->GetID();
}

BodyInterface::AddState BodyInterface::AddBodiesPrepare(BodyID *ioBodies, int inNumber)
{
	return mBroadPhase->AddBodiesPrepare(ioBodies, inNumber);
}

void BodyInterface::AddBodiesFinalize(BodyID *ioBodies, int inNumber, AddState inAddState, EActivation inActivationMode)
{
	BodyLockMultiWrite lock(*mBodyLockInterface, ioBodies, inNumber);

	// Add to broadphase
	mBroadPhase->AddBodiesFinalize(ioBodies, inNumber, inAddState);

	// Optionally activate bodies
	if (inActivationMode == EActivation::Activate)
		mBodyManager->ActivateBodies(ioBodies, inNumber);
}

void BodyInterface::AddBodiesAbort(BodyID *ioBodies, int inNumber, AddState inAddState)
{
	mBroadPhase->AddBodiesAbort(ioBodies, inNumber, inAddState);
}

void BodyInterface::RemoveBodies(BodyID *ioBodies, int inNumber)
{
	BodyLockMultiWrite lock(*mBodyLockInterface, ioBodies, inNumber);

	// Deactivate bodies
	mBodyManager->DeactivateBodies(ioBodies, inNumber);

	// Remove from broadphase
	mBroadPhase->RemoveBodies(ioBodies, inNumber);
}

void BodyInterface::ActivateBody(const BodyID &inBodyID)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();

		if (!body.IsActive())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::ActivateBodies(const BodyID *inBodyIDs, int inNumber)
{
	BodyLockMultiWrite lock(*mBodyLockInterface, inBodyIDs, inNumber);

	mBodyManager->ActivateBodies(inBodyIDs, inNumber);
}

void BodyInterface::DeactivateBody(const BodyID &inBodyID)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();

		if (body.IsActive())
			mBodyManager->DeactivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::DeactivateBodies(const BodyID *inBodyIDs, int inNumber)
{
	BodyLockMultiWrite lock(*mBodyLockInterface, inBodyIDs, inNumber);

	mBodyManager->DeactivateBodies(inBodyIDs, inNumber);
}

bool BodyInterface::IsActive(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	return lock.Succeeded() && lock.GetBody().IsActive();
}

TwoBodyConstraint *BodyInterface::CreateConstraint(const TwoBodyConstraintSettings *inSettings, const BodyID &inBodyID1, const BodyID &inBodyID2)
{
	BodyID constraint_bodies[] = { inBodyID1, inBodyID2 };
	BodyLockMultiWrite lock(*mBodyLockInterface, constraint_bodies, 2);

	Body *body1 = lock.GetBody(0);
	Body *body2 = lock.GetBody(1);

	JPH_ASSERT(body1 != body2);
	JPH_ASSERT(body1 != nullptr || body2 != nullptr);

	return inSettings->Create(body1 != nullptr? *body1 : Body::sFixedToWorld, body2 != nullptr? *body2 : Body::sFixedToWorld);
}

void BodyInterface::ActivateConstraint(const TwoBodyConstraint *inConstraint)
{
	BodyID bodies[] = { inConstraint->GetBody1()->GetID(), inConstraint->GetBody2()->GetID() };
	ActivateBodies(bodies, 2);
}

RefConst<Shape> BodyInterface::GetShape(const BodyID &inBodyID) const
{
	RefConst<Shape> shape;
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		shape = lock.GetBody().GetShape();
	return shape;
}

void BodyInterface::SetShape(const BodyID &inBodyID, const Shape *inShape, bool inUpdateMassProperties, EActivation inActivationMode) const
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Check if shape actually changed
		if (body.GetShape() != inShape)
		{
			// Update the shape
			body.SetShapeInternal(inShape, inUpdateMassProperties);

			// Flag collision cache invalid for this body
			mBodyManager->InvalidateContactCacheForBody(body);

			// Notify broadphase of change
			if (body.IsInBroadPhase())
			{
				BodyID id = body.GetID();
				mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
			}

			// Optionally activate body
			if (inActivationMode == EActivation::Activate && !body.IsStatic())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::NotifyShapeChanged(const BodyID &inBodyID, Vec3Arg inPreviousCenterOfMass, bool inUpdateMassProperties, EActivation inActivationMode) const
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Update center of mass, mass and inertia
		body.UpdateCenterOfMassInternal(inPreviousCenterOfMass, inUpdateMassProperties);

		// Recalculate bounding box
		body.CalculateWorldSpaceBoundsInternal();

		// Flag collision cache invalid for this body
		mBodyManager->InvalidateContactCacheForBody(body);

		// Notify broadphase of change
		if (body.IsInBroadPhase())
		{
			BodyID id = body.GetID();
			mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
		}

		// Optionally activate body
		if (inActivationMode == EActivation::Activate && !body.IsStatic())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::SetObjectLayer(const BodyID &inBodyID, ObjectLayer inLayer)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Check if layer actually changed, updating the broadphase is rather expensive
		if (body.GetObjectLayer() != inLayer)
		{
			// Update the layer on the body
			mBodyManager->SetBodyObjectLayerInternal(body, inLayer);

			// Notify broadphase of change
			if (body.IsInBroadPhase())
			{
				BodyID id = body.GetID();
				mBroadPhase->NotifyBodiesLayerChanged(&id, 1);
			}
		}
	}
}

ObjectLayer BodyInterface::GetObjectLayer(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetObjectLayer();
	else
		return cObjectLayerInvalid;
}

void BodyInterface::SetPositionAndRotation(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Update the position
		body.SetPositionAndRotationInternal(inPosition, inRotation);

		// Notify broadphase of change
		if (body.IsInBroadPhase())
		{
			BodyID id = body.GetID();
			mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
		}

		// Optionally activate body
		if (inActivationMode == EActivation::Activate && !body.IsStatic())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::SetPositionAndRotationWhenChanged(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Check if there is enough change
		if (!body.GetPosition().IsClose(inPosition)
			|| !body.GetRotation().IsClose(inRotation))
		{
			// Update the position
			body.SetPositionAndRotationInternal(inPosition, inRotation);

			// Notify broadphase of change
			if (body.IsInBroadPhase())
			{
				BodyID id = body.GetID();
				mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
			}

			// Optionally activate body
			if (inActivationMode == EActivation::Activate && !body.IsStatic())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::GetPositionAndRotation(const BodyID &inBodyID, Vec3 &outPosition, Quat &outRotation) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();
		outPosition = body.GetPosition();
		outRotation = body.GetRotation();
	}
	else
	{
		outPosition = Vec3::sZero();
		outRotation = Quat::sIdentity();
	}
}

void BodyInterface::SetPosition(const BodyID &inBodyID, Vec3Arg inPosition, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Update the position
		body.SetPositionAndRotationInternal(inPosition, body.GetRotation());

		// Notify broadphase of change
		if (body.IsInBroadPhase())
		{
			BodyID id = body.GetID();
			mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
		}

		// Optionally activate body
		if (inActivationMode == EActivation::Activate && !body.IsStatic())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

Vec3 BodyInterface::GetPosition(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetPosition();
	else
		return Vec3::sZero();
}

Vec3 BodyInterface::GetCenterOfMassPosition(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetCenterOfMassPosition();
	else
		return Vec3::sZero();
}

void BodyInterface::SetRotation(const BodyID &inBodyID, QuatArg inRotation, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Update the position
		body.SetPositionAndRotationInternal(body.GetPosition(), inRotation);

		// Notify broadphase of change
		if (body.IsInBroadPhase())
		{
			BodyID id = body.GetID();
			mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
		}

		// Optionally activate body
		if (inActivationMode == EActivation::Activate && !body.IsStatic())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

Quat BodyInterface::GetRotation(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetRotation();
	else
		return Quat::sIdentity();
}

Mat44 BodyInterface::GetWorldTransform(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetWorldTransform();
	else
		return Mat44::sIdentity();
}

Mat44 BodyInterface::GetCenterOfMassTransform(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetCenterOfMassTransform();
	else
		return Mat44::sIdentity();
}

void BodyInterface::MoveKinematic(const BodyID &inBodyID, Vec3Arg inTargetPosition, QuatArg inTargetRotation, float inDeltaTime)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		body.MoveKinematic(inTargetPosition, inTargetRotation, inDeltaTime);

		if (!body.IsActive() && (!body.GetLinearVelocity().IsNearZero() || !body.GetAngularVelocity().IsNearZero()))
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

void BodyInterface::SetLinearAndAngularVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			body.SetLinearVelocityClamped(inLinearVelocity);
			body.SetAngularVelocityClamped(inAngularVelocity);

			if (!body.IsActive() && (!inLinearVelocity.IsNearZero() || !inAngularVelocity.IsNearZero()))
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::GetLinearAndAngularVelocity(const BodyID &inBodyID, Vec3 &outLinearVelocity, Vec3 &outAngularVelocity) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			outLinearVelocity = body.GetLinearVelocity();
			outAngularVelocity = body.GetAngularVelocity();
			return;
		}
	}

	outLinearVelocity = outAngularVelocity = Vec3::sZero();
}

void BodyInterface::SetLinearVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			body.SetLinearVelocityClamped(inLinearVelocity);

			if (!body.IsActive() && !inLinearVelocity.IsNearZero())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

Vec3 BodyInterface::GetLinearVelocity(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();
		if (!body.IsStatic())
			return body.GetLinearVelocity();
	}

	return Vec3::sZero();
}

void BodyInterface::AddLinearVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			body.SetLinearVelocityClamped(body.GetLinearVelocity() + inLinearVelocity);

			if (!body.IsActive() && !body.GetLinearVelocity().IsNearZero())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddLinearAndAngularVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			body.SetLinearVelocityClamped(body.GetLinearVelocity() + inLinearVelocity);
			body.SetAngularVelocityClamped(body.GetAngularVelocity() + inAngularVelocity);

			if (!body.IsActive() && (!body.GetLinearVelocity().IsNearZero() || !body.GetAngularVelocity().IsNearZero()))
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::SetAngularVelocity(const BodyID &inBodyID, Vec3Arg inAngularVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (!body.IsStatic())
		{
			body.SetAngularVelocityClamped(inAngularVelocity);

			if (!body.IsActive() && !inAngularVelocity.IsNearZero())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

Vec3 BodyInterface::GetAngularVelocity(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();
		if (!body.IsStatic())
			return body.GetAngularVelocity();
	}

	return Vec3::sZero();
}

Vec3 BodyInterface::GetPointVelocity(const BodyID &inBodyID, Vec3Arg inPoint) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		const Body &body = lock.GetBody();
		if (!body.IsStatic())
			return body.GetPointVelocity(inPoint);
	}

	return Vec3::sZero();
}

void BodyInterface::AddForce(const BodyID &inBodyID, Vec3Arg inForce)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddForce(inForce);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddForce(const BodyID &inBodyID, Vec3Arg inForce, Vec3Arg inPoint)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddForce(inForce, inPoint);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddTorque(const BodyID &inBodyID, Vec3Arg inTorque)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddTorque(inTorque);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddForceAndTorque(const BodyID &inBodyID, Vec3Arg inForce, Vec3Arg inTorque)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddForce(inForce);
			body.AddTorque(inTorque);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddImpulse(const BodyID &inBodyID, Vec3Arg inImpulse)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddImpulse(inImpulse);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddImpulse(const BodyID &inBodyID, Vec3Arg inImpulse, Vec3Arg inPoint)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddImpulse(inImpulse, inPoint);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::AddAngularImpulse(const BodyID &inBodyID, Vec3Arg inAngularImpulse)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();
		if (body.IsDynamic())
		{
			body.AddAngularImpulse(inAngularImpulse);

			if (!body.IsActive())
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::SetPositionRotationAndVelocity(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Update the position
		body.SetPositionAndRotationInternal(inPosition, inRotation);

		// Notify broadphase of change
		if (body.IsInBroadPhase())
		{
			BodyID id = body.GetID();
			mBroadPhase->NotifyBodiesAABBChanged(&id, 1);
		}

		if (!body.IsStatic())
		{
			body.SetLinearVelocityClamped(inLinearVelocity);
			body.SetAngularVelocityClamped(inAngularVelocity);

			// Optionally activate body
			if (!body.IsActive() && (!inLinearVelocity.IsNearZero() || !inAngularVelocity.IsNearZero()))
				mBodyManager->ActivateBodies(&inBodyID, 1);
		}
	}
}

void BodyInterface::SetMotionType(const BodyID &inBodyID, EMotionType inMotionType, EActivation inActivationMode)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
	{
		Body &body = lock.GetBody();

		// Deactivate if we're making the body static
		if (body.IsActive() && inMotionType == EMotionType::Static)
			mBodyManager->DeactivateBodies(&inBodyID, 1);
			
		body.SetMotionType(inMotionType);

		// Activate body if requested
		if (inMotionType != EMotionType::Static && inActivationMode == EActivation::Activate && !body.IsActive())
			mBodyManager->ActivateBodies(&inBodyID, 1);
	}
}

Mat44 BodyInterface::GetInverseInertia(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetInverseInertia();
	else
		return Mat44::sIdentity();
}

void BodyInterface::SetRestitution(const BodyID &inBodyID, float inRestitution)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		lock.GetBody().SetRestitution(inRestitution);
}

float BodyInterface::GetRestitution(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetRestitution();
	else
		return 0.0f;
}

void BodyInterface::SetFriction(const BodyID &inBodyID, float inFriction)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		lock.GetBody().SetFriction(inFriction);
}

float BodyInterface::GetFriction(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetFriction();
	else
		return 0.0f;
}

void BodyInterface::SetGravityFactor(const BodyID &inBodyID, float inGravityFactor)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded() && lock.GetBody().GetMotionPropertiesUnchecked() != nullptr)
		lock.GetBody().GetMotionPropertiesUnchecked()->SetGravityFactor(inGravityFactor);
}

float BodyInterface::GetGravityFactor(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded() && lock.GetBody().GetMotionPropertiesUnchecked() != nullptr)
		return lock.GetBody().GetMotionPropertiesUnchecked()->GetGravityFactor();
	else
		return 1.0f;
}

TransformedShape BodyInterface::GetTransformedShape(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetTransformedShape();
	else
		return TransformedShape();
}

uint64 BodyInterface::GetUserData(const BodyID &inBodyID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetUserData();
	else
		return 0;
}

const PhysicsMaterial *BodyInterface::GetMaterial(const BodyID &inBodyID, const SubShapeID &inSubShapeID) const
{
	BodyLockRead lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		return lock.GetBody().GetShape()->GetMaterial(inSubShapeID);
	else
		return PhysicsMaterial::sDefault;
}

void BodyInterface::InvalidateContactCache(const BodyID &inBodyID)
{
	BodyLockWrite lock(*mBodyLockInterface, inBodyID);
	if (lock.Succeeded())
		mBodyManager->InvalidateContactCacheForBody(lock.GetBody());
}

JPH_NAMESPACE_END

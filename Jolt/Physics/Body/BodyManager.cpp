// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/Core/StringTools.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

#ifdef JPH_ENABLE_ASSERTS
	thread_local bool BodyManager::sOverrideAllowActivation = false;
	thread_local bool BodyManager::sOverrideAllowDeactivation = false;
#endif

// Helper class that combines a body and its motion properties
class BodyWithMotionProperties : public Body
{
public:
	MotionProperties		mMotionProperties;
};

inline void BodyManager::sDeleteBody(Body *inBody)
{
	if (inBody->mMotionProperties != nullptr)
	{
		JPH_IF_ENABLE_ASSERTS(inBody->mMotionProperties = nullptr;)
		delete static_cast<BodyWithMotionProperties *>(inBody);
	}
	else
		delete inBody;
}

BodyManager::~BodyManager()
{
	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	// Destroy any bodies that are still alive
	for (Body *b : mBodies)
		if (sIsValidBodyPointer(b))
			sDeleteBody(b);

	delete [] mActiveBodies;
}

void BodyManager::Init(uint inMaxBodies, uint inNumBodyMutexes, const BroadPhaseLayerInterface &inLayerInterface)
{
	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	// Num body mutexes must be a power of two and not bigger than our MutexMask
	uint num_body_mutexes = Clamp<uint>(GetNextPowerOf2(inNumBodyMutexes == 0? 2 * thread::hardware_concurrency() : inNumBodyMutexes), 1, sizeof(MutexMask) * 8);

	// Allocate the body mutexes
	mBodyMutexes.Init(num_body_mutexes);

	// Allocate space for bodies
	mBodies.reserve(inMaxBodies);

	// Allocate space for active bodies
	JPH_ASSERT(mActiveBodies == nullptr);
	mActiveBodies = new BodyID [inMaxBodies];

	// Allocate space for sequence numbers
	mBodySequenceNumbers.resize(inMaxBodies);

	// Keep layer interface
	mBroadPhaseLayerInterface = &inLayerInterface;
}

uint BodyManager::GetNumBodies() const
{
	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	return mNumBodies;
}

BodyManager::BodyStats BodyManager::GetBodyStats() const
{
	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	BodyStats stats;
	stats.mNumBodies = mNumBodies;
	stats.mMaxBodies = uint(mBodies.capacity());

	for (const Body *body : mBodies)
		if (sIsValidBodyPointer(body))
		{
			switch (body->GetMotionType())
			{
			case EMotionType::Static:
				stats.mNumBodiesStatic++;
				break;

			case EMotionType::Dynamic:
				stats.mNumBodiesDynamic++;
				if (body->IsActive())
					stats.mNumActiveBodiesDynamic++;
				break;

			case EMotionType::Kinematic:
				stats.mNumBodiesKinematic++;
				if (body->IsActive())
					stats.mNumActiveBodiesKinematic++;
				break;
			}
		}

	return stats;
}

Body *BodyManager::CreateBody(const BodyCreationSettings &inBodyCreationSettings)
{
	// Determine next free index
	uint32 idx;
	{
		UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

		if (mBodyIDFreeListStart != cBodyIDFreeListEnd)
		{
			// Pop an item from the freelist
			JPH_ASSERT(mBodyIDFreeListStart & cIsFreedBody);
			idx = uint32(mBodyIDFreeListStart >> cFreedBodyIndexShift);
			JPH_ASSERT(!sIsValidBodyPointer(mBodies[idx]));
			mBodyIDFreeListStart = uintptr_t(mBodies[idx]);
		}
		else
		{
			if (mBodies.size() < mBodies.capacity())
			{
				// Allocate a new entry, note that the array should not actually resize since we've reserved it at init time
				idx = uint32(mBodies.size());
				mBodies.push_back((Body *)cBodyIDFreeListEnd);
			}
			else
			{
				// Out of bodies
				return nullptr;
			}
		}

		// Update cached number of bodies
		mNumBodies++;
	}

	// Get next sequence number
	uint8 seq_no = GetNextSequenceNumber(idx);

	// Fill in basic properties
	Body *body;
	if (inBodyCreationSettings.HasMassProperties())
	{
		BodyWithMotionProperties *bmp = new BodyWithMotionProperties;
		body = bmp;
		body->mMotionProperties = &bmp->mMotionProperties;
	}
	else
	{
	 	body = new Body;
	}
	body->mID = BodyID(idx, seq_no);
	body->mShape = inBodyCreationSettings.GetShape();
	body->mUserData = inBodyCreationSettings.mUserData;
	body->SetFriction(inBodyCreationSettings.mFriction);
	body->SetRestitution(inBodyCreationSettings.mRestitution);
	body->mMotionType = inBodyCreationSettings.mMotionType;
	if (inBodyCreationSettings.mIsSensor)
		body->SetIsSensor(true);
	SetBodyObjectLayerInternal(*body, inBodyCreationSettings.mObjectLayer);
	body->mObjectLayer = inBodyCreationSettings.mObjectLayer;
	body->mCollisionGroup = inBodyCreationSettings.mCollisionGroup;
	
	if (inBodyCreationSettings.HasMassProperties())
	{
		MotionProperties *mp = body->mMotionProperties;
		mp->SetLinearDamping(inBodyCreationSettings.mLinearDamping);
		mp->SetAngularDamping(inBodyCreationSettings.mAngularDamping);
		mp->SetMaxLinearVelocity(inBodyCreationSettings.mMaxLinearVelocity);
		mp->SetMaxAngularVelocity(inBodyCreationSettings.mMaxAngularVelocity);
		mp->SetLinearVelocity(inBodyCreationSettings.mLinearVelocity); // Needs to happen after setting the max linear/angular velocity
		mp->SetAngularVelocity(inBodyCreationSettings.mAngularVelocity);
		mp->SetGravityFactor(inBodyCreationSettings.mGravityFactor);
		mp->SetMotionQuality(inBodyCreationSettings.mMotionQuality);
		mp->mAllowSleeping = inBodyCreationSettings.mAllowSleeping;
		mp->mIndexInActiveBodies = Body::cInactiveIndex;
		mp->mIslandIndex = Body::cInactiveIndex;
		JPH_IF_ENABLE_ASSERTS(mp->mCachedMotionType = body->mMotionType;)
		mp->SetMassProperties(inBodyCreationSettings.GetMassProperties());
	}

	// Position body
	body->SetPositionAndRotationInternal(inBodyCreationSettings.mPosition, inBodyCreationSettings.mRotation);

	// Add body
	mBodies[idx] = body;
	return body;
}

void BodyManager::DestroyBodies(const BodyID *inBodyIDs, int inNumber)
{
	// Don't take lock if no bodies are to be destroyed
	if (inNumber <= 0)
		return;

	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	// Update cached number of bodies
	JPH_ASSERT(mNumBodies >= (uint)inNumber);
	mNumBodies -= inNumber;

	for (const BodyID *b = inBodyIDs, *b_end = inBodyIDs + inNumber; b < b_end; b++)
	{
		// Get body
		BodyID body_id = *b;
		uint32 idx = body_id.GetIndex();
		Body *body = mBodies[idx];

		// Validate that it can be removed
		JPH_ASSERT(body->GetID() == body_id);
		JPH_ASSERT(!body->IsActive());
		JPH_ASSERT(!body->IsInBroadPhase());
	
		// Push the id onto the freelist
		mBodies[idx] = (Body *)mBodyIDFreeListStart;
		mBodyIDFreeListStart = (uintptr_t(idx) << cFreedBodyIndexShift) | cIsFreedBody;

		// Free the body
		sDeleteBody(body);
	}

#if defined(_DEBUG) && defined(JPH_ENABLE_ASSERTS)
	// Check that the freelist is correct
	size_t num_freed = 0;
	for (uintptr_t start = mBodyIDFreeListStart; start != cBodyIDFreeListEnd; start = uintptr_t(mBodies[start >> cFreedBodyIndexShift]))
	{
		JPH_ASSERT(start & cIsFreedBody);
		num_freed++;
	}
	JPH_ASSERT(mNumBodies == mBodies.size() - num_freed);
#endif // defined(_DEBUG) && _defined(JPH_ENABLE_ASSERTS)
}

void BodyManager::ActivateBodies(const BodyID *inBodyIDs, int inNumber)
{
	// Don't take lock if no bodies are to be activated
	if (inNumber <= 0)
		return;

	UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

	JPH_ASSERT(!mActiveBodiesLocked || sOverrideAllowActivation);

	for (const BodyID *b = inBodyIDs, *b_end = inBodyIDs + inNumber; b < b_end; b++)
		if (!b->IsInvalid())
		{
			BodyID body_id = *b;
			Body &body = *mBodies[body_id.GetIndex()];

			JPH_ASSERT(GetMutexForBody(body_id).is_locked(), "Assuming that body has been locked!");
			JPH_ASSERT(body.GetID() == body_id);
			JPH_ASSERT(body.IsInBroadPhase());

			if (!body.IsStatic()
				&& body.mMotionProperties->mIndexInActiveBodies == Body::cInactiveIndex)
			{
				body.mMotionProperties->mIndexInActiveBodies = mNumActiveBodies;
				body.ResetSleepTestSpheres();
				JPH_ASSERT(mNumActiveBodies < GetMaxBodies());
				mActiveBodies[mNumActiveBodies] = body_id;
				mNumActiveBodies++; // Increment atomic after setting the body ID so that PhysicsSystem::JobFindCollisions (which doesn't lock the mActiveBodiesMutex) will only read valid IDs

				// Count CCD bodies
				if (body.mMotionProperties->GetMotionQuality() == EMotionQuality::LinearCast)
					mNumActiveCCDBodies++;

				// Call activation listener
				if (mActivationListener != nullptr)
					mActivationListener->OnBodyActivated(body_id, body.GetUserData());
			}
		}
}

void BodyManager::DeactivateBodies(const BodyID *inBodyIDs, int inNumber)
{
	// Don't take lock if no bodies are to be deactivated
	if (inNumber <= 0)
		return;

	UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

	JPH_ASSERT(!mActiveBodiesLocked || sOverrideAllowDeactivation);

	for (const BodyID *b = inBodyIDs, *b_end = inBodyIDs + inNumber; b < b_end; b++)
		if (!b->IsInvalid())
		{
			BodyID body_id = *b;
			Body &body = *mBodies[body_id.GetIndex()];

			JPH_ASSERT(GetMutexForBody(body_id).is_locked(), "Assuming that body has been locked!");
			JPH_ASSERT(body.GetID() == body_id);
			JPH_ASSERT(body.IsInBroadPhase());

			if (body.mMotionProperties != nullptr
				&& body.mMotionProperties->mIndexInActiveBodies != Body::cInactiveIndex)
			{
				uint32 last_body_index = mNumActiveBodies - 1;
				if (body.mMotionProperties->mIndexInActiveBodies != last_body_index)
				{
					// This is not the last body, use the last body to fill the hole
					BodyID last_body_id = mActiveBodies[last_body_index];
					mActiveBodies[body.mMotionProperties->mIndexInActiveBodies] = last_body_id;

					// Update that body's index in the active list
					Body &last_body = *mBodies[last_body_id.GetIndex()];
					JPH_ASSERT(last_body.mMotionProperties->mIndexInActiveBodies == last_body_index);
					last_body.mMotionProperties->mIndexInActiveBodies = body.mMotionProperties->mIndexInActiveBodies;
				}

				// Mark this body as no longer active
				body.mMotionProperties->mIndexInActiveBodies = Body::cInactiveIndex;
				body.mMotionProperties->mIslandIndex = Body::cInactiveIndex;

				// Reset velocity
				body.mMotionProperties->mLinearVelocity = Vec3::sZero();
				body.mMotionProperties->mAngularVelocity = Vec3::sZero();

				// Remove unused element from active bodies list
				--mNumActiveBodies;

				// Count CCD bodies
				if (body.mMotionProperties->GetMotionQuality() == EMotionQuality::LinearCast)
					mNumActiveCCDBodies--;

				// Call activation listener
				if (mActivationListener != nullptr)
					mActivationListener->OnBodyDeactivated(body_id, body.GetUserData());
			}
		}
}

void BodyManager::GetActiveBodies(BodyIDVector &outBodyIDs) const
{
	JPH_PROFILE_FUNCTION();

	UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

	outBodyIDs.assign(mActiveBodies, mActiveBodies + mNumActiveBodies);
}

void BodyManager::GetBodyIDs(BodyIDVector &outBodies) const
{
	JPH_PROFILE_FUNCTION();

	UniqueLock lock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	// Reserve space for all bodies
	outBodies.clear();
	outBodies.reserve(mNumBodies);

	// Iterate the list and find the bodies that are not null
	for (const Body *b : mBodies)
		if (sIsValidBodyPointer(b))
			outBodies.push_back(b->GetID());

	// Validate that our reservation was correct
	JPH_ASSERT(outBodies.size() == mNumBodies);
}

void BodyManager::SetBodyActivationListener(BodyActivationListener *inListener)	
{ 
	UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

	mActivationListener = inListener; 
}

BodyManager::MutexMask BodyManager::GetMutexMask(const BodyID *inBodies, int inNumber) const
{
	JPH_ASSERT(sizeof(MutexMask) * 8 >= mBodyMutexes.GetNumMutexes(), "MutexMask must have enough bits");

	if (inNumber >= (int)mBodyMutexes.GetNumMutexes())
	{
		// Just lock everything if there are too many bodies
		return GetAllBodiesMutexMask();
	}
	else
	{
		MutexMask mask = 0;
		for (const BodyID *b = inBodies, *b_end = inBodies + inNumber; b < b_end; ++b)
			if (!b->IsInvalid())
			{
				uint32 index = mBodyMutexes.GetMutexIndex(b->GetIndex());
				mask |= (MutexMask(1) << index);
			}
		return mask;
	}
}

void BodyManager::LockRead(MutexMask inMutexMask) const
{
	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckLock(EPhysicsLockTypes::PerBody));

	int index = 0;
	for (MutexMask mask = inMutexMask; mask != 0; mask >>= 1, index++)
		if (mask & 1)
			mBodyMutexes.GetMutexByIndex(index).lock_shared();
}

void BodyManager::UnlockRead(MutexMask inMutexMask) const
{
	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckUnlock(EPhysicsLockTypes::PerBody));

	int index = 0;
	for (MutexMask mask = inMutexMask; mask != 0; mask >>= 1, index++)
		if (mask & 1)
			mBodyMutexes.GetMutexByIndex(index).unlock_shared();
}

void BodyManager::LockWrite(MutexMask inMutexMask) const
{
	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckLock(EPhysicsLockTypes::PerBody));

	int index = 0;
	for (MutexMask mask = inMutexMask; mask != 0; mask >>= 1, index++)
		if (mask & 1)
			mBodyMutexes.GetMutexByIndex(index).lock();
}

void BodyManager::UnlockWrite(MutexMask inMutexMask) const
{
	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckUnlock(EPhysicsLockTypes::PerBody));

	int index = 0;
	for (MutexMask mask = inMutexMask; mask != 0; mask >>= 1, index++)
		if (mask & 1)
			mBodyMutexes.GetMutexByIndex(index).unlock();
}

void BodyManager::LockAllBodies() const						
{ 
	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckLock(EPhysicsLockTypes::PerBody));
	mBodyMutexes.LockAll(); 

	PhysicsLock::sLock(mBodiesMutex, EPhysicsLockTypes::BodiesList);
}

void BodyManager::UnlockAllBodies() const						
{ 
	PhysicsLock::sUnlock(mBodiesMutex, EPhysicsLockTypes::BodiesList);

	JPH_IF_ENABLE_ASSERTS(PhysicsLock::sCheckUnlock(EPhysicsLockTypes::PerBody));
	mBodyMutexes.UnlockAll(); 
}

void BodyManager::SaveState(StateRecorder &inStream) const
{
	{
		LockAllBodies();

		// Count number of bodies
		size_t num_bodies = 0;
		for (const Body *b : mBodies)
			if (sIsValidBodyPointer(b) && b->IsInBroadPhase())
				++num_bodies;
		inStream.Write(num_bodies);
	
		// Write state of bodies
		for (const Body *b : mBodies)
			if (sIsValidBodyPointer(b) && b->IsInBroadPhase())
			{
				inStream.Write(b->GetID());
				b->SaveState(inStream);
			}

		UnlockAllBodies();
	}

	{
		UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

		// Write active bodies, sort because activation can come from multiple threads, so order is not deterministic
		inStream.Write(mNumActiveBodies);
		BodyIDVector sorted_active_bodies(mActiveBodies, mActiveBodies + mNumActiveBodies);
		sort(sorted_active_bodies.begin(), sorted_active_bodies.end());
		for (const BodyID &id : sorted_active_bodies)
			inStream.Write(id);

		inStream.Write(mNumActiveCCDBodies);
	}
}

bool BodyManager::RestoreState(StateRecorder &inStream)
{
	{
		LockAllBodies();

		// Read state of bodies, note this reads it in a way to be consistent with validation
		size_t old_num_bodies = 0;
		for (const Body *b : mBodies)
			if (sIsValidBodyPointer(b) && b->IsInBroadPhase())
				++old_num_bodies;
		size_t num_bodies = old_num_bodies; // Initialize to current value for validation
		inStream.Read(num_bodies);
		if (num_bodies != old_num_bodies)
		{
			JPH_ASSERT(false, "Cannot handle adding/removing bodies");
			UnlockAllBodies();
			return false;
		}

		for (Body *b : mBodies)
			if (sIsValidBodyPointer(b) && b->IsInBroadPhase())
			{
				BodyID body_id = b->GetID(); // Initialize to current value for validation
				inStream.Read(body_id);
				if (body_id != b->GetID())
				{
					JPH_ASSERT(false, "Cannot handle adding/removing bodies");
					UnlockAllBodies();
					return false;
				}
				b->RestoreState(inStream);
			}

		UnlockAllBodies();
	}

	{
		UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

		// Mark current active bodies as deactivated
		for (const BodyID *id = mActiveBodies, *id_end = mActiveBodies + mNumActiveBodies; id < id_end; ++id)
			mBodies[id->GetIndex()]->mMotionProperties->mIndexInActiveBodies = Body::cInactiveIndex;

		sort(mActiveBodies, mActiveBodies + mNumActiveBodies); // Sort for validation

		// Read active bodies
		inStream.Read(mNumActiveBodies);
		for (BodyID *id = mActiveBodies, *id_end = mActiveBodies + mNumActiveBodies; id < id_end; ++id)
		{
			inStream.Read(*id);
			mBodies[id->GetIndex()]->mMotionProperties->mIndexInActiveBodies = uint32(id - mActiveBodies);
		}

		inStream.Read(mNumActiveCCDBodies);
	}

	return true;
}

#ifdef JPH_DEBUG_RENDERER
void BodyManager::Draw(const DrawSettings &inDrawSettings, const PhysicsSettings &inPhysicsSettings, DebugRenderer *inRenderer)
{
	JPH_PROFILE_FUNCTION();

	LockAllBodies();

	for (const Body *body : mBodies)
		if (sIsValidBodyPointer(body) && body->IsInBroadPhase())
		{
			JPH_ASSERT(mBodies[body->GetID().GetIndex()] == body);

			bool is_sensor = body->IsSensor();

			// Determine drawing mode
			Color color;
			if (is_sensor)
				color = Color::sYellow;
			else
				switch (inDrawSettings.mDrawShapeColor)
				{
				case EShapeColor::InstanceColor:
					// Each instance has own color
					color = Color::sGetDistinctColor(body->mID.GetIndex());
					break;

				case EShapeColor::ShapeTypeColor:
					color = ShapeFunctions::sGet(body->GetShape()->GetSubType()).mColor;
					break;

				case EShapeColor::MotionTypeColor:
					// Determine color based on motion type
					switch (body->mMotionType)
					{
					case EMotionType::Static:
						color = Color::sGrey;
						break;

					case EMotionType::Kinematic:
						color = Color::sGreen;
						break;

					case EMotionType::Dynamic:
						color = Color::sGetDistinctColor(body->mID.GetIndex());
						break;

					default:
						JPH_ASSERT(false);
						color = Color::sBlack;
						break;
					}
					break;

				case EShapeColor::SleepColor:
					// Determine color based on motion type
					switch (body->mMotionType)
					{
					case EMotionType::Static:
						color = Color::sGrey;
						break;

					case EMotionType::Kinematic:
						color = body->IsActive()? Color::sGreen : Color::sRed;
						break;

					case EMotionType::Dynamic:
						color = body->IsActive()? Color::sYellow : Color::sRed;
						break;

					default:
						JPH_ASSERT(false);
						color = Color::sBlack;
						break;
					}
					break;

				case EShapeColor::IslandColor:
					// Determine color based on motion type
					switch (body->mMotionType)
					{
					case EMotionType::Static:
						color = Color::sGrey;
						break;

					case EMotionType::Kinematic:
					case EMotionType::Dynamic:
						{
							uint32 idx = body->GetMotionProperties()->GetIslandIndexInternal();
							color = idx != Body::cInactiveIndex? Color::sGetDistinctColor(idx) : Color::sLightGrey;
						}
						break;

					default:
						JPH_ASSERT(false);
						color = Color::sBlack;
						break;
					}
					break;

				case EShapeColor::MaterialColor:
					color = Color::sWhite;
					break;

				default:
					JPH_ASSERT(false);
					color = Color::sBlack;
					break;
				}
			
			// Draw the results of GetSupportFunction
			if (inDrawSettings.mDrawGetSupportFunction)
				body->mShape->DrawGetSupportFunction(inRenderer, body->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f), color, inDrawSettings.mDrawSupportDirection);

			// Draw the results of GetSupportingFace
			if (inDrawSettings.mDrawGetSupportingFace)
				body->mShape->DrawGetSupportingFace(inRenderer, body->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f));

			// Draw the shape
			if (inDrawSettings.mDrawShape)
				body->mShape->Draw(inRenderer, body->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f), color, inDrawSettings.mDrawShapeColor == EShapeColor::MaterialColor, inDrawSettings.mDrawShapeWireframe || is_sensor);

			// Draw bounding box
			if (inDrawSettings.mDrawBoundingBox)
				inRenderer->DrawWireBox(body->mBounds, color);

			// Draw center of mass transform
			if (inDrawSettings.mDrawCenterOfMassTransform)
				inRenderer->DrawCoordinateSystem(body->GetCenterOfMassTransform(), 0.2f);

			// Draw world transform
			if (inDrawSettings.mDrawWorldTransform)
				inRenderer->DrawCoordinateSystem(body->GetWorldTransform(), 0.2f);

			// Draw world space linear and angular velocity
			if (inDrawSettings.mDrawVelocity)
			{
				Vec3 pos = body->GetCenterOfMassPosition();
				inRenderer->DrawArrow(pos, pos + body->GetLinearVelocity(), Color::sGreen, 0.1f);
				inRenderer->DrawArrow(pos, pos + body->GetAngularVelocity(), Color::sRed, 0.1f);
			}

			if (inDrawSettings.mDrawMassAndInertia && body->IsDynamic())
			{
				const MotionProperties *mp = body->GetMotionProperties();

				// Invert mass again
				float mass = 1.0f / mp->GetInverseMass();

				// Invert diagonal again
				Vec3 diagonal = mp->GetInverseInertiaDiagonal().Reciprocal();

				// Determine how big of a box has the equivalent inertia
				Vec3 box_size = MassProperties::sGetEquivalentSolidBoxSize(mass, diagonal);

				// Draw box with equivalent inertia
				inRenderer->DrawWireBox(body->GetCenterOfMassTransform() * Mat44::sRotation(mp->GetInertiaRotation()), AABox(-0.5f * box_size, 0.5f * box_size), Color::sOrange);

				// Draw mass
				inRenderer->DrawText3D(body->GetCenterOfMassPosition(), StringFormat("%.2f", (double)mass), Color::sOrange, 0.2f);
			}

			if (inDrawSettings.mDrawSleepStats && body->IsDynamic() && body->IsActive())
			{
				// Draw stats to know which bodies could go to sleep
				string text = StringFormat("t: %.1f", (double)body->mMotionProperties->mSleepTestTimer);
				uint8 g = uint8(Clamp(255.0f * body->mMotionProperties->mSleepTestTimer / inPhysicsSettings.mTimeBeforeSleep, 0.0f, 255.0f));
				Color sleep_color = Color(0, 255 - g, g);
				inRenderer->DrawText3D(body->GetCenterOfMassPosition(), text, sleep_color, 0.2f);
				for (int i = 0; i < 3; ++i)
					inRenderer->DrawWireSphere(body->mMotionProperties->mSleepTestSpheres[i].GetCenter(), body->mMotionProperties->mSleepTestSpheres[i].GetRadius(), sleep_color);
			}
		}

	UnlockAllBodies();
}
#endif // JPH_DEBUG_RENDERER

void BodyManager::InvalidateContactCacheForBody(Body &ioBody)
{
	// If this is the first time we flip the collision cache invalid flag, we need to add it to an internal list to ensure we reset the flag at the end of the physics update
	if (ioBody.InvalidateContactCacheInternal())
	{
		lock_guard lock(mBodiesCacheInvalidMutex);
		mBodiesCacheInvalid.push_back(ioBody.GetID());
	}
}

void BodyManager::ValidateContactCacheForAllBodies()
{
	lock_guard lock(mBodiesCacheInvalidMutex);
	
	for (const BodyID &b : mBodiesCacheInvalid)
	{
		// The body may have been removed between the call to InvalidateContactCacheForBody and this call, so check if it still exists
		Body *body = TryGetBody(b);
		if (body != nullptr)
			body->ValidateContactCacheInternal();
	}
	mBodiesCacheInvalid.clear();
}

#ifdef _DEBUG
void BodyManager::ValidateActiveBodyBounds()
{
	UniqueLock lock(mActiveBodiesMutex, EPhysicsLockTypes::ActiveBodiesList);

	for (BodyID *id = mActiveBodies, *id_end = mActiveBodies + mNumActiveBodies; id < id_end; ++id)
	{
		const Body *body = mBodies[id->GetIndex()];
		AABox cached = body->GetWorldSpaceBounds();
		AABox calculated = body->GetShape()->GetWorldSpaceBounds(body->GetCenterOfMassTransform(), Vec3::sReplicate(1.0f));
		JPH_ASSERT(cached == calculated);
	}
}
#endif // _DEBUG

JPH_NAMESPACE_END

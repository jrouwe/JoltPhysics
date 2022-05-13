// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsUpdateContext.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/IslandBuilder.h>
#include <Jolt/Core/TempAllocator.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

#ifdef JPH_DEBUG_RENDERER
bool ContactConstraintManager::sDrawContactPoint = false;
bool ContactConstraintManager::sDrawSupportingFaces = false;
bool ContactConstraintManager::sDrawContactPointReduction = false;
bool ContactConstraintManager::sDrawContactManifolds = false;
#endif // JPH_DEBUG_RENDERER

//#define JPH_MANIFOLD_CACHE_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::WorldContactPoint
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ContactConstraintManager::WorldContactPoint::CalculateNonPenetrationConstraintProperties(float inDeltaTime, const Body &inBody1, const Body &inBody2, Vec3Arg inWorldSpacePosition1, Vec3Arg inWorldSpacePosition2, Vec3Arg inWorldSpaceNormal)
{
	// Calculate collision points relative to body
	Vec3 p = 0.5f * (inWorldSpacePosition1 + inWorldSpacePosition2);
	Vec3 r1 = p - inBody1.GetCenterOfMassPosition();
	Vec3 r2 = p - inBody2.GetCenterOfMassPosition();

	mNonPenetrationConstraint.CalculateConstraintProperties(inDeltaTime, inBody1, r1, inBody2, r2, inWorldSpaceNormal);
}

template <EMotionType Type1, EMotionType Type2>
JPH_INLINE void ContactConstraintManager::WorldContactPoint::CalculateFrictionAndNonPenetrationConstraintProperties(float inDeltaTime, const Body &inBody1, const Body &inBody2, Mat44Arg inInvI1, Mat44Arg inInvI2, Vec3Arg inWorldSpacePosition1, Vec3Arg inWorldSpacePosition2, Vec3Arg inWorldSpaceNormal, Vec3Arg inWorldSpaceTangent1, Vec3Arg inWorldSpaceTangent2, float inCombinedRestitution, float inCombinedFriction, float inMinVelocityForRestitution)
{
	// Calculate collision points relative to body
	Vec3 p = 0.5f * (inWorldSpacePosition1 + inWorldSpacePosition2);
	Vec3 r1 = p - inBody1.GetCenterOfMassPosition();
	Vec3 r2 = p - inBody2.GetCenterOfMassPosition();

	const MotionProperties *mp1 = inBody1.GetMotionPropertiesUnchecked();
	const MotionProperties *mp2 = inBody2.GetMotionPropertiesUnchecked();

	// Calculate velocity of collision points
	Vec3 relative_velocity;
	if constexpr (Type1 != EMotionType::Static && Type2 != EMotionType::Static)
		relative_velocity = mp2->GetPointVelocityCOM(r2) - mp1->GetPointVelocityCOM(r1);
	else if constexpr (Type1 != EMotionType::Static)
		relative_velocity = -mp1->GetPointVelocityCOM(r1);
	else if constexpr (Type2 != EMotionType::Static)
		relative_velocity = mp2->GetPointVelocityCOM(r2);
	else
	{
		JPH_ASSERT(false); // Static vs static makes no sense
		relative_velocity = Vec3::sZero();
	}
	float normal_velocity = relative_velocity.Dot(inWorldSpaceNormal);

	// How much the shapes are penetrating (> 0 if penetrating, < 0 if separated)
	float penetration = (inWorldSpacePosition1 - inWorldSpacePosition2).Dot(inWorldSpaceNormal);

	// If there is no penetration, this is a speculative contact and we will apply a bias to the contact constraint
	// so that the constraint becomes relative_velocity . contact normal > -penetration / delta_time
	// instead of relative_velocity . contact normal > 0
	// See: GDC 2013: "Physics for Game Programmers; Continuous Collision" - Erin Catto
	float speculative_contact_velocity_bias = max(0.0f, -penetration / inDeltaTime);

	// Determine if the velocity is big enough for restitution
	float normal_velocity_bias;
	if (inCombinedRestitution > 0.0f && normal_velocity < -inMinVelocityForRestitution)
	{
		// We have a velocity that is big enough for restitution. This is where speculative contacts don't work
		// great as we have to decide now if we're going to apply the restitution or not. If the relative
		// velocity is big enough for a hit, we apply the restitution (in the end, due to other constraints,
		// the objects may actually not collide and we will have applied restitution incorrectly). Another
		// artifact that occurs because of this approximation is that the object will bounce from its current
		// position rather than from a position where it is touching the other object. This causes the object
		// to appear to move faster for 1 frame (the opposite of time stealing).
		if (normal_velocity < -speculative_contact_velocity_bias)
			normal_velocity_bias = inCombinedRestitution * normal_velocity;
		else
			normal_velocity_bias = 0.0f;
	}
	else
	{
		// No restitution. We can safely apply our contact velocity bias.
		normal_velocity_bias = speculative_contact_velocity_bias;
	}

	mNonPenetrationConstraint.TemplatedCalculateConstraintProperties<Type1, Type2>(inDeltaTime, mp1, inInvI1, r1, mp2, inInvI2, r2, inWorldSpaceNormal, normal_velocity_bias);

	// Calculate friction part
	if (inCombinedFriction > 0.0f)
	{
		// Implement friction as 2 AxisContraintParts
		mFrictionConstraint1.TemplatedCalculateConstraintProperties<Type1, Type2>(inDeltaTime, mp1, inInvI1, r1, mp2, inInvI2, r2, inWorldSpaceTangent1);
		mFrictionConstraint2.TemplatedCalculateConstraintProperties<Type1, Type2>(inDeltaTime, mp1, inInvI1, r1, mp2, inInvI2, r2, inWorldSpaceTangent2);
	}
	else
	{
		// Turn off friction constraint
		mFrictionConstraint1.Deactivate();
		mFrictionConstraint2.Deactivate();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::ContactConstraint
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef JPH_DEBUG_RENDERER
void ContactConstraintManager::ContactConstraint::Draw(DebugRenderer *inRenderer, ColorArg inManifoldColor) const
{			
	if (mContactPoints.empty())
		return;

	// Get body transforms
	Mat44 transform_body1 = mBody1->GetCenterOfMassTransform();
	Mat44 transform_body2 = mBody2->GetCenterOfMassTransform();

	Vec3 prev_point = transform_body1 * Vec3::sLoadFloat3Unsafe(mContactPoints.back().mContactPoint->mPosition1);
	for (const WorldContactPoint &wcp : mContactPoints)
	{
		// Test if any lambda from the previous frame was transferred
		float radius = wcp.mNonPenetrationConstraint.GetTotalLambda() == 0.0f 
					&& wcp.mFrictionConstraint1.GetTotalLambda() == 0.0f 
					&& wcp.mFrictionConstraint2.GetTotalLambda() == 0.0f? 0.1f :  0.2f;

		Vec3 next_point = transform_body1 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition1);
		inRenderer->DrawMarker(next_point, Color::sCyan, radius);
		inRenderer->DrawMarker(transform_body2 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition2), Color::sPurple, radius);

		// Draw edge
		inRenderer->DrawArrow(prev_point, next_point, inManifoldColor, 0.05f);
		prev_point = next_point;
	}

	// Draw normal
	Vec3 wp = transform_body1 * Vec3::sLoadFloat3Unsafe(mContactPoints[0].mContactPoint->mPosition1);
	inRenderer->DrawArrow(wp, wp + mWorldSpaceNormal, Color::sRed, 0.05f);

	// Get tangents
	Vec3 t1, t2;
	GetTangents(t1, t2);

	// Draw tangents
	inRenderer->DrawLine(wp, wp + t1, Color::sGreen);
	inRenderer->DrawLine(wp, wp + t2, Color::sBlue);
}
#endif // JPH_DEBUG_RENDERER

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::CachedContactPoint
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ContactConstraintManager::CachedContactPoint::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mPosition1);
	inStream.Write(mPosition2);
	inStream.Write(mNonPenetrationLambda);
	inStream.Write(mFrictionLambda);
}

void ContactConstraintManager::CachedContactPoint::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mPosition1);
	inStream.Read(mPosition2);
	inStream.Read(mNonPenetrationLambda);
	inStream.Read(mFrictionLambda);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::CachedManifold
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ContactConstraintManager::CachedManifold::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mContactNormal);
}

void ContactConstraintManager::CachedManifold::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mContactNormal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::CachedBodyPair
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ContactConstraintManager::CachedBodyPair::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mDeltaPosition);
	inStream.Write(mDeltaRotation);
}

void ContactConstraintManager::CachedBodyPair::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mDeltaPosition);
	inStream.Read(mDeltaRotation);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager::ManifoldCache
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ContactConstraintManager::ManifoldCache::Init(uint inMaxBodyPairs, uint inMaxContactConstraints, uint inCachedManifoldsSize)
{
	mAllocator.Init(inMaxBodyPairs * sizeof(BodyPairMap::KeyValue) + inCachedManifoldsSize);
	mCachedManifolds.Init(GetNextPowerOf2(inMaxContactConstraints));
	mCachedBodyPairs.Init(GetNextPowerOf2(inMaxBodyPairs));
}

void ContactConstraintManager::ManifoldCache::Clear()
{
	JPH_PROFILE_FUNCTION();

	mCachedManifolds.Clear();
	mCachedBodyPairs.Clear();
	mAllocator.Clear();

#ifdef JPH_ENABLE_ASSERTS
	// Mark as incomplete
	mIsFinalized = false;
#endif
}

void ContactConstraintManager::ManifoldCache::Prepare(uint inExpectedNumBodyPairs, uint inExpectedNumManifolds)
{
	// Minimum amount of buckets to use in the hash map
	constexpr uint32 cMinBuckets = 1024;

	// Use the next higher power of 2 of amount of objects in the cache from last frame to determine the amount of buckets in this frame
	mCachedManifolds.SetNumBuckets(min(max(cMinBuckets, GetNextPowerOf2(inExpectedNumManifolds)), mCachedManifolds.GetMaxBuckets()));
	mCachedBodyPairs.SetNumBuckets(min(max(cMinBuckets, GetNextPowerOf2(inExpectedNumBodyPairs)), mCachedBodyPairs.GetMaxBuckets()));
}

const ContactConstraintManager::MKeyValue *ContactConstraintManager::ManifoldCache::Find(const SubShapeIDPair &inKey, size_t inKeyHash) const
{
	JPH_ASSERT(mIsFinalized);
	return mCachedManifolds.Find(inKey, inKeyHash);
}

ContactConstraintManager::MKeyValue *ContactConstraintManager::ManifoldCache::Create(ContactAllocator &ioContactAllocator, const SubShapeIDPair &inKey, size_t inKeyHash, int inNumContactPoints)
{
	JPH_ASSERT(!mIsFinalized);
	MKeyValue *kv = mCachedManifolds.Create(ioContactAllocator, inKey, inKeyHash, CachedManifold::sGetRequiredExtraSize(inNumContactPoints));
	if (kv == nullptr)
	{
		JPH_ASSERT(false, "Out of cache space for manifold cache");
		return nullptr;
	}
	kv->GetValue().mNumContactPoints = uint16(inNumContactPoints);
	++ioContactAllocator.mNumManifolds;
	return kv;
}

ContactConstraintManager::MKVAndCreated ContactConstraintManager::ManifoldCache::FindOrCreate(ContactAllocator &ioContactAllocator, const SubShapeIDPair &inKey, size_t inKeyHash, int inNumContactPoints)
{
	MKeyValue *kv = const_cast<MKeyValue *>(mCachedManifolds.Find(inKey, inKeyHash));
	if (kv != nullptr)
		return { kv, false };

	return { Create(ioContactAllocator, inKey, inKeyHash, inNumContactPoints), true };
}

uint32 ContactConstraintManager::ManifoldCache::ToHandle(const MKeyValue *inKeyValue) const
{
	JPH_ASSERT(!mIsFinalized);	
	return mCachedManifolds.ToHandle(inKeyValue);
}

const ContactConstraintManager::MKeyValue *ContactConstraintManager::ManifoldCache::FromHandle(uint32 inHandle) const
{
	JPH_ASSERT(mIsFinalized);	
	return mCachedManifolds.FromHandle(inHandle);
}

const ContactConstraintManager::BPKeyValue *ContactConstraintManager::ManifoldCache::Find(const BodyPair &inKey, size_t inKeyHash) const
{
	JPH_ASSERT(mIsFinalized);	
	return mCachedBodyPairs.Find(inKey, inKeyHash);
}

ContactConstraintManager::BPKeyValue *ContactConstraintManager::ManifoldCache::Create(ContactAllocator &ioContactAllocator, const BodyPair &inKey, size_t inKeyHash)
{
	JPH_ASSERT(!mIsFinalized);
	BPKeyValue *kv = mCachedBodyPairs.Create(ioContactAllocator, inKey, inKeyHash, 0);
	if (kv == nullptr)
	{
		JPH_ASSERT(false, "Out of cache space for body pair cache");
		return nullptr;
	}
	++ioContactAllocator.mNumBodyPairs;
	return kv;
}

void ContactConstraintManager::ManifoldCache::GetAllBodyPairsSorted(vector<const BPKeyValue *> &outAll) const
{
	JPH_ASSERT(mIsFinalized);
	mCachedBodyPairs.GetAllKeyValues(outAll);

	// Sort by key
	sort(outAll.begin(), outAll.end(), [](const BPKeyValue *inLHS, const BPKeyValue *inRHS) {
		return inLHS->GetKey() < inRHS->GetKey();
	});
}

void ContactConstraintManager::ManifoldCache::GetAllManifoldsSorted(const CachedBodyPair &inBodyPair, vector<const MKeyValue *> &outAll) const
{
	JPH_ASSERT(mIsFinalized);

	// Iterate through the attached manifolds
	for (uint32 handle = inBodyPair.mFirstCachedManifold; handle != ManifoldMap::cInvalidHandle; handle = FromHandle(handle)->GetValue().mNextWithSameBodyPair)
	{
		const MKeyValue *kv = mCachedManifolds.FromHandle(handle);
		outAll.push_back(kv);
	}

	// Sort by key
	sort(outAll.begin(), outAll.end(), [](const MKeyValue *inLHS, const MKeyValue *inRHS) {
		return inLHS->GetKey() < inRHS->GetKey();
	});
}

void ContactConstraintManager::ManifoldCache::GetAllCCDManifoldsSorted(vector<const MKeyValue *> &outAll) const
{
	mCachedManifolds.GetAllKeyValues(outAll);

	for (int i = (int)outAll.size() - 1; i >= 0; --i)
		if ((outAll[i]->GetValue().mFlags & (uint16)CachedManifold::EFlags::CCDContact) == 0)
		{
			outAll[i] = outAll.back();
			outAll.pop_back();
		}

	// Sort by key
	sort(outAll.begin(), outAll.end(), [](const MKeyValue *inLHS, const MKeyValue *inRHS) {
		return inLHS->GetKey() < inRHS->GetKey();
	});
}

void ContactConstraintManager::ManifoldCache::ContactPointRemovedCallbacks(ContactListener *inListener)
{
	for (MKeyValue &kv : mCachedManifolds)
		if ((kv.GetValue().mFlags & uint16(CachedManifold::EFlags::ContactPersisted)) == 0)
			inListener->OnContactRemoved(kv.GetKey());
}

#ifdef JPH_ENABLE_ASSERTS

void ContactConstraintManager::ManifoldCache::Finalize()
{
	mIsFinalized = true;

#ifdef JPH_MANIFOLD_CACHE_DEBUG
	Trace("ManifoldMap:");
	mCachedManifolds.TraceStats();
	Trace("BodyPairMap:");
	mCachedBodyPairs.TraceStats();
#endif // JPH_MANIFOLD_CACHE_DEBUG
}

#endif

void ContactConstraintManager::ManifoldCache::SaveState(StateRecorder &inStream) const
{
	JPH_ASSERT(mIsFinalized);

	// Get contents of cache
	vector<const BPKeyValue *> all_bp;
	GetAllBodyPairsSorted(all_bp);

	// Write amount of body pairs
	size_t num_body_pairs = all_bp.size();
	inStream.Write(num_body_pairs);

	// Write all body pairs
	for (const BPKeyValue *bp_kv : all_bp)
	{
		// Write body pair key
		inStream.Write(bp_kv->GetKey());

		// Write body pair
		const CachedBodyPair &bp = bp_kv->GetValue();
		bp.SaveState(inStream);

		// Get attached manifolds
		vector<const MKeyValue *> all_m;
		GetAllManifoldsSorted(bp, all_m);

		// Write num manifolds
		size_t num_manifolds = all_m.size();
		inStream.Write(num_manifolds);

		// Write all manifolds
		for (const MKeyValue *m_kv : all_m)
		{
			// Write key
			inStream.Write(m_kv->GetKey());
			const CachedManifold &cm = m_kv->GetValue();
			JPH_ASSERT((cm.mFlags & (uint16)CachedManifold::EFlags::CCDContact) == 0);

			// Write amount of contacts
			inStream.Write(cm.mNumContactPoints);

			// Write manifold
			cm.SaveState(inStream);

			// Write contact points
			for (uint32 i = 0; i < cm.mNumContactPoints; ++i)
				cm.mContactPoints[i].SaveState(inStream);
		}
	}

	// Get CCD manifolds
	vector<const MKeyValue *> all_m;
	GetAllCCDManifoldsSorted(all_m);

	// Write num CCD manifolds
	size_t num_manifolds = all_m.size();
	inStream.Write(num_manifolds);

	// Write all CCD manifold keys
	for (const MKeyValue *m_kv : all_m)
		inStream.Write(m_kv->GetKey());
}

bool ContactConstraintManager::ManifoldCache::RestoreState(const ManifoldCache &inReadCache, StateRecorder &inStream)
{
	JPH_ASSERT(!mIsFinalized);

	bool success = true;

	// Create a contact allocator for restoring the contact cache
	ContactAllocator contact_allocator(GetContactAllocator());

	// When validating, get all existing body pairs
	vector<const BPKeyValue *> all_bp;
	if (inStream.IsValidating())
		inReadCache.GetAllBodyPairsSorted(all_bp);

	// Read amount of body pairs
	size_t num_body_pairs;
	if (inStream.IsValidating())
		num_body_pairs = all_bp.size();
	inStream.Read(num_body_pairs);

	// Read entire cache
	for (size_t i = 0; i < num_body_pairs; ++i)
	{
		// Read key
		BodyPair body_pair_key;
		if (inStream.IsValidating() && i < all_bp.size())
			body_pair_key = all_bp[i]->GetKey();
		inStream.Read(body_pair_key);

		// Create new entry for this body pair
		size_t body_pair_hash = BodyPairHash {} (body_pair_key);
		BPKeyValue *bp_kv = Create(contact_allocator, body_pair_key, body_pair_hash);
		if (bp_kv == nullptr)
		{
			// Out of cache space
			success = false;
			break; 
		}
		CachedBodyPair &bp = bp_kv->GetValue();

		// Read body pair
		if (inStream.IsValidating() && i < all_bp.size())
			memcpy(&bp, &all_bp[i]->GetValue(), sizeof(CachedBodyPair));
		bp.RestoreState(inStream);

		// When validating, get all existing manifolds
		vector<const MKeyValue *> all_m;
		if (inStream.IsValidating())
			inReadCache.GetAllManifoldsSorted(all_bp[i]->GetValue(), all_m);

		// Read amount of manifolds
		size_t num_manifolds;
		if (inStream.IsValidating())
			num_manifolds = all_m.size();
		inStream.Read(num_manifolds);

		uint32 handle = ManifoldMap::cInvalidHandle;
		for (size_t j = 0; j < num_manifolds; ++j)
		{
			// Read key
			SubShapeIDPair sub_shape_key;
			if (inStream.IsValidating() && j < all_m.size())
				sub_shape_key = all_m[j]->GetKey();
			inStream.Read(sub_shape_key);
			size_t sub_shape_key_hash = std::hash<SubShapeIDPair> {} (sub_shape_key);
			
			// Read amount of contact points
			uint16 num_contact_points;
			if (inStream.IsValidating() && j < all_m.size())
				num_contact_points = all_m[j]->GetValue().mNumContactPoints;
			inStream.Read(num_contact_points);

			// Read manifold
			MKeyValue *m_kv = Create(contact_allocator, sub_shape_key, sub_shape_key_hash, num_contact_points);
			if (m_kv == nullptr)
			{
				// Out of cache space
				success = false;
				break; 
			}
			CachedManifold &cm = m_kv->GetValue();
			if (inStream.IsValidating() && j < all_m.size())
			{
				memcpy(&cm, &all_m[j]->GetValue(), CachedManifold::sGetRequiredTotalSize(num_contact_points));
				cm.mNumContactPoints = uint16(num_contact_points); // Restore num contact points
			}
			cm.RestoreState(inStream);
			cm.mNextWithSameBodyPair = handle;
			handle = ToHandle(m_kv);

			// Read contact points
			for (uint32 k = 0; k < num_contact_points; ++k)
				cm.mContactPoints[k].RestoreState(inStream);
		}		
		bp.mFirstCachedManifold = handle;
	}

	// When validating, get all existing CCD manifolds
	vector<const MKeyValue *> all_m;
	if (inStream.IsValidating())
		inReadCache.GetAllCCDManifoldsSorted(all_m);

	// Read amount of CCD manifolds
	size_t num_manifolds;
	if (inStream.IsValidating())
		num_manifolds = all_m.size();
	inStream.Read(num_manifolds);

	for (size_t j = 0; j < num_manifolds; ++j)
	{
		// Read key
		SubShapeIDPair sub_shape_key;
		if (inStream.IsValidating() && j < all_m.size())
			sub_shape_key = all_m[j]->GetKey();
		inStream.Read(sub_shape_key);
		size_t sub_shape_key_hash = std::hash<SubShapeIDPair> {} (sub_shape_key);
			
		// Create CCD manifold
		MKeyValue *m_kv = Create(contact_allocator, sub_shape_key, sub_shape_key_hash, 0);
		if (m_kv == nullptr)
		{
			// Out of cache space
			success = false;
			break; 
		}
		CachedManifold &cm = m_kv->GetValue();
		cm.mFlags |= (uint16)CachedManifold::EFlags::CCDContact;
	}

#ifdef JPH_ENABLE_ASSERTS
	mIsFinalized = true;
#endif

	return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ContactConstraintManager
////////////////////////////////////////////////////////////////////////////////////////////////////////

ContactConstraintManager::ContactConstraintManager(const PhysicsSettings &inPhysicsSettings) :
	mPhysicsSettings(inPhysicsSettings)
{
#ifdef JPH_ENABLE_ASSERTS
	// For the first frame mark this empty buffer as finalized
	mCache[mCacheWriteIdx ^ 1].Finalize();
#endif
}

ContactConstraintManager::~ContactConstraintManager()
{
	JPH_ASSERT(mConstraints == nullptr);
}

void ContactConstraintManager::Init(uint inMaxBodyPairs, uint inMaxContactConstraints)
{
	mMaxConstraints = inMaxContactConstraints;

	// Calculate worst case cache usage
	uint cached_manifolds_size = inMaxContactConstraints * (sizeof(CachedManifold) + (MaxContactPoints - 1) * sizeof(CachedContactPoint));

	// Init the caches
	mCache[0].Init(inMaxBodyPairs, inMaxContactConstraints, cached_manifolds_size);
	mCache[1].Init(inMaxBodyPairs, inMaxContactConstraints, cached_manifolds_size);
}

void ContactConstraintManager::PrepareConstraintBuffer(PhysicsUpdateContext *inContext)
{	
	// Store context
	mUpdateContext = inContext;

	// Allocate temporary constraint buffer
	JPH_ASSERT(mConstraints == nullptr);
	mConstraints = (ContactConstraint *)inContext->mTempAllocator->Allocate(mMaxConstraints * sizeof(ContactConstraint));
}

template <EMotionType Type1, EMotionType Type2>
JPH_INLINE void ContactConstraintManager::TemplatedCalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint &ioConstraint, float inDeltaTime, Mat44Arg inTransformBody1, Mat44Arg inTransformBody2, const Body &inBody1, const Body &inBody2, Mat44Arg inInvI1, Mat44Arg inInvI2)
{
	// Calculate tangents
	Vec3 t1, t2;
	ioConstraint.GetTangents(t1, t2);

	// Setup velocity constraint properties
	float min_velocity_for_restitution = mPhysicsSettings.mMinVelocityForRestitution;
	for (WorldContactPoint &wcp : ioConstraint.mContactPoints)
	{
		Vec3 p1 = inTransformBody1 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition1);
		Vec3 p2 = inTransformBody2 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition2);
		wcp.CalculateFrictionAndNonPenetrationConstraintProperties<Type1, Type2>(inDeltaTime, inBody1, inBody2, inInvI1, inInvI2, p1, p2, ioConstraint.mWorldSpaceNormal, t1, t2, ioConstraint.mCombinedRestitution, ioConstraint.mCombinedFriction, min_velocity_for_restitution);
	}
}

inline void ContactConstraintManager::CalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint &ioConstraint, float inDeltaTime, Mat44Arg inTransformBody1, Mat44Arg inTransformBody2, const Body &inBody1, const Body &inBody2)
{
	// Dispatch to the correct templated form
	switch (inBody1.GetMotionType())
	{
	case EMotionType::Dynamic:
		{
			Mat44 invi1 = inBody1.GetInverseInertia();
			switch (inBody2.GetMotionType())
			{
			case EMotionType::Dynamic:
				TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EMotionType::Dynamic, EMotionType::Dynamic>(ioConstraint, inDeltaTime, inTransformBody1, inTransformBody2, inBody1, inBody2, invi1, inBody2.GetInverseInertia());
				break;

			case EMotionType::Kinematic:
				TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EMotionType::Dynamic, EMotionType::Kinematic>(ioConstraint, inDeltaTime, inTransformBody1, inTransformBody2, inBody1, inBody2, invi1, Mat44() /* Will not be used */);
				break;

			case EMotionType::Static:
				TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EMotionType::Dynamic, EMotionType::Static>(ioConstraint, inDeltaTime, inTransformBody1, inTransformBody2, inBody1, inBody2, invi1, Mat44() /* Will not be used */);
				break;

			default:
				JPH_ASSERT(false);
				break;
			}
			break;
		}

	case EMotionType::Kinematic:
		JPH_ASSERT(inBody2.IsDynamic());
		TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EMotionType::Kinematic, EMotionType::Dynamic>(ioConstraint, inDeltaTime, inTransformBody1, inTransformBody2, inBody1, inBody2, Mat44() /* Will not be used */, inBody2.GetInverseInertia());
		break;

	case EMotionType::Static:
		JPH_ASSERT(inBody2.IsDynamic());
		TemplatedCalculateFrictionAndNonPenetrationConstraintProperties<EMotionType::Static, EMotionType::Dynamic>(ioConstraint, inDeltaTime, inTransformBody1, inTransformBody2, inBody1, inBody2, Mat44() /* Will not be used */, inBody2.GetInverseInertia());
		break;

	default:
		JPH_ASSERT(false);
		break;
	}
}

void ContactConstraintManager::GetContactsFromCache(ContactAllocator &ioContactAllocator, Body &inBody1, Body &inBody2, bool &outPairHandled, bool &outConstraintCreated)
{
	JPH_PROFILE_FUNCTION();

	// Start with nothing found and not handled
	outConstraintCreated = false;
	outPairHandled = false;

	// Swap bodies so that body 1 id < body 2 id
	Body *body1, *body2;
	if (inBody1.GetID() < inBody2.GetID())
	{
		body1 = &inBody1; 
		body2 = &inBody2;
	}
	else
	{
		body1 = &inBody2;
		body2 = &inBody1;
	}

	// Find the cached body pair
	BodyPair body_pair_key(body1->GetID(), body2->GetID());
	size_t body_pair_hash = BodyPairHash {} (body_pair_key);
	const ManifoldCache &read_cache = mCache[mCacheWriteIdx ^ 1];
	const BPKeyValue *kv = read_cache.Find(body_pair_key, body_pair_hash);
	if (kv == nullptr)
		return;
	const CachedBodyPair &input_cbp = kv->GetValue();

	// Get relative translation
	Quat inv_r1 = body1->GetRotation().Conjugated();
	Vec3 delta_position = inv_r1 * (body2->GetCenterOfMassPosition() - body1->GetCenterOfMassPosition());

	// Get old position delta
	Vec3 old_delta_position = Vec3::sLoadFloat3Unsafe(input_cbp.mDeltaPosition);

	// Check if bodies are still roughly in the same relative position
	if ((delta_position - old_delta_position).LengthSq() > mPhysicsSettings.mBodyPairCacheMaxDeltaPositionSq)
		return;

	// Determine relative orientation
	Quat delta_rotation = inv_r1 * body2->GetRotation();

	// Reconstruct old quaternion delta
	Quat old_delta_rotation = Quat::sLoadFloat3Unsafe(input_cbp.mDeltaRotation);

	// Check if bodies are still roughly in the same relative orientation
	// The delta between 2 quaternions p and q is: p q^* = [rotation_axis * sin(angle / 2), cos(angle / 2)]
	// From the W component we can extract the angle: cos(angle / 2) = px * qx + py * qy + pz * qz + pw * qw = p . q
	// Since we want to abort if the rotation is smaller than -angle or bigger than angle, we can write the comparison as |p . q| < cos(angle / 2)
	if (abs(delta_rotation.Dot(old_delta_rotation)) < mPhysicsSettings.mBodyPairCacheCosMaxDeltaRotationDiv2)
		return;

	// The cache is valid, return that we've handled this body pair
	outPairHandled = true;

	// Copy the cached body pair to this frame
	ManifoldCache &write_cache = mCache[mCacheWriteIdx];
	BPKeyValue *output_bp_kv = write_cache.Create(ioContactAllocator, body_pair_key, body_pair_hash);
	if (output_bp_kv == nullptr)
		return; // Out of cache space
	CachedBodyPair *output_cbp = &output_bp_kv->GetValue();
	memcpy(output_cbp, &input_cbp, sizeof(CachedBodyPair));

	// If there were no contacts, we have handled the contact
	if (input_cbp.mFirstCachedManifold == ManifoldMap::cInvalidHandle)
		return;

	// Get body transforms
	Mat44 transform_body1 = body1->GetCenterOfMassTransform();
	Mat44 transform_body2 = body2->GetCenterOfMassTransform();

	// Get time step
	float delta_time = mUpdateContext->mSubStepDeltaTime;

	// Copy manifolds
	uint32 output_handle = ManifoldMap::cInvalidHandle;
	uint32 input_handle = input_cbp.mFirstCachedManifold;
	do
	{
		JPH_PROFILE("Add Constraint From Cached Manifold");

		// Find the existing manifold
		const MKeyValue *input_kv = read_cache.FromHandle(input_handle);
		const SubShapeIDPair &input_key = input_kv->GetKey();
		const CachedManifold &input_cm = input_kv->GetValue();
		JPH_ASSERT(input_cm.mNumContactPoints > 0); // There should be contact points in this manifold!

		// Create room for manifold in write buffer and copy data
		size_t input_hash = std::hash<SubShapeIDPair> {} (input_key);
		MKeyValue *output_kv = write_cache.Create(ioContactAllocator, input_key, input_hash, input_cm.mNumContactPoints);
		if (output_kv == nullptr)
			break; // Out of cache space
		CachedManifold *output_cm = &output_kv->GetValue();
		memcpy(output_cm, &input_cm, CachedManifold::sGetRequiredTotalSize(input_cm.mNumContactPoints));

		// Link the object under the body pairs
		output_cm->mNextWithSameBodyPair = output_handle;
		output_handle = write_cache.ToHandle(output_kv);

		// Calculate default contact settings
		ContactSettings settings;
		settings.mCombinedFriction = mCombineFriction(*body1, input_key.GetSubShapeID1(), *body2, input_key.GetSubShapeID2());
		settings.mCombinedRestitution = mCombineRestitution(*body1, input_key.GetSubShapeID1(), *body2, input_key.GetSubShapeID2());
		settings.mIsSensor = body1->IsSensor() || body2->IsSensor();

		// Calculate world space contact normal
		Vec3 world_space_normal = transform_body2.Multiply3x3(Vec3::sLoadFloat3Unsafe(output_cm->mContactNormal)).Normalized();

		// Call contact listener to update settings
		if (mContactListener != nullptr)
		{
			// Convert constraint to manifold structure for callback
			ContactManifold manifold;
			manifold.mWorldSpaceNormal = world_space_normal;
			manifold.mSubShapeID1 = input_key.GetSubShapeID1();
			manifold.mSubShapeID2 = input_key.GetSubShapeID2();
			manifold.mWorldSpaceContactPointsOn1.resize(output_cm->mNumContactPoints);
			manifold.mWorldSpaceContactPointsOn2.resize(output_cm->mNumContactPoints);
			float penetration_depth = -FLT_MAX;
			for (uint32 i = 0; i < output_cm->mNumContactPoints; ++i)
			{
				const CachedContactPoint &ccp = output_cm->mContactPoints[i];
				manifold.mWorldSpaceContactPointsOn1[i] = transform_body1 * Vec3::sLoadFloat3Unsafe(ccp.mPosition1);
				manifold.mWorldSpaceContactPointsOn2[i] = transform_body2 * Vec3::sLoadFloat3Unsafe(ccp.mPosition2);
				penetration_depth = max(penetration_depth, (manifold.mWorldSpaceContactPointsOn1[0] - manifold.mWorldSpaceContactPointsOn2[0]).Dot(world_space_normal));
			}
			manifold.mPenetrationDepth = penetration_depth; // We don't have the penetration depth anymore, estimate it

			// Notify callback
			mContactListener->OnContactPersisted(*body1, *body2, manifold, settings);
		}

		// If one of the bodies is a sensor, don't actually create the constraint
		JPH_ASSERT(settings.mIsSensor || !(body1->IsSensor() || body2->IsSensor()), "Sensors cannot be converted into regular bodies by a contact callback!");
		if (!settings.mIsSensor)
		{
			// Add contact constraint in world space for the solver
			uint32 constraint_idx = mNumConstraints++;
			if (constraint_idx >= mMaxConstraints)
			{
				JPH_ASSERT(false, "Out of contact constraints!");
				break;
			}
			
			// A constraint will be created
			outConstraintCreated = true;

			ContactConstraint &constraint = mConstraints[constraint_idx];
			new (&constraint) ContactConstraint();
			constraint.mBody1 = body1;
			constraint.mBody2 = body2;
			constraint.mSortKey = input_hash;
			constraint.mWorldSpaceNormal = world_space_normal;
			constraint.mCombinedFriction = settings.mCombinedFriction;
			constraint.mCombinedRestitution = settings.mCombinedRestitution;
			constraint.mContactPoints.resize(output_cm->mNumContactPoints);
			for (uint32 i = 0; i < output_cm->mNumContactPoints; ++i)
			{
				CachedContactPoint &ccp = output_cm->mContactPoints[i];
				WorldContactPoint &wcp = constraint.mContactPoints[i];
				wcp.mNonPenetrationConstraint.SetTotalLambda(ccp.mNonPenetrationLambda);
				wcp.mFrictionConstraint1.SetTotalLambda(ccp.mFrictionLambda[0]);
				wcp.mFrictionConstraint2.SetTotalLambda(ccp.mFrictionLambda[1]);
				wcp.mContactPoint = &ccp;
			}

			// Calculate friction and non-penetration constraint properties for all contact points
			CalculateFrictionAndNonPenetrationConstraintProperties(constraint, delta_time, transform_body1, transform_body2, *body1, *body2);

			// Notify island builder
			mUpdateContext->mIslandBuilder->LinkContact(constraint_idx, body1->GetIndexInActiveBodiesInternal(), body2->GetIndexInActiveBodiesInternal());

		#ifdef JPH_DEBUG_RENDERER
			// Draw the manifold
			if (sDrawContactManifolds)
				constraint.Draw(DebugRenderer::sInstance, Color::sYellow);
		#endif // JPH_DEBUG_RENDERER
		}

		// Mark contact as persisted so that we won't fire OnContactRemoved callbacks
		input_cm.mFlags |= (uint16)CachedManifold::EFlags::ContactPersisted;

		// Fetch the next manifold
		input_handle = input_cm.mNextWithSameBodyPair;
	}
	while (input_handle != ManifoldMap::cInvalidHandle);
	output_cbp->mFirstCachedManifold = output_handle;
}

ContactConstraintManager::BodyPairHandle ContactConstraintManager::AddBodyPair(ContactAllocator &ioContactAllocator, const Body &inBody1, const Body &inBody2)
{
	JPH_PROFILE_FUNCTION();

	// Swap bodies so that body 1 id < body 2 id
	const Body *body1, *body2;
	if (inBody1.GetID() < inBody2.GetID())
	{
		body1 = &inBody1;
		body2 = &inBody2;
	}
	else
	{
		body1 = &inBody2;
		body2 = &inBody1;
	}

	// Add an entry
	BodyPair body_pair_key(body1->GetID(), body2->GetID());
	size_t body_pair_hash = BodyPairHash {} (body_pair_key);
	BPKeyValue *body_pair_kv = mCache[mCacheWriteIdx].Create(ioContactAllocator, body_pair_key, body_pair_hash);
	if (body_pair_kv == nullptr)
		return nullptr; // Out of cache space
	CachedBodyPair *cbp = &body_pair_kv->GetValue();
	cbp->mFirstCachedManifold = ManifoldMap::cInvalidHandle;

	// Get relative translation
	Quat inv_r1 = body1->GetRotation().Conjugated();
	Vec3 delta_position = inv_r1 * (body2->GetCenterOfMassPosition() - body1->GetCenterOfMassPosition());

	// Store it
	delta_position.StoreFloat3(&cbp->mDeltaPosition);

	// Determine relative orientation
	Quat delta_rotation = inv_r1 * body2->GetRotation();

	// Store it
	delta_rotation.StoreFloat3(&cbp->mDeltaRotation);

	return cbp;
}

template <EMotionType Type1, EMotionType Type2>
bool ContactConstraintManager::TemplatedAddContactConstraint(ContactAllocator &ioContactAllocator, BodyPairHandle inBodyPairHandle, Body &inBody1, Body &inBody2, const ContactManifold &inManifold, Mat44Arg inInvI1, Mat44Arg inInvI2)
{
	// Calculate hash
	SubShapeIDPair key { inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2 };
	size_t key_hash = std::hash<SubShapeIDPair> {} (key);

	// Determine number of contact points
	int num_contact_points = (int)inManifold.mWorldSpaceContactPointsOn1.size();
	JPH_ASSERT(num_contact_points <= MaxContactPoints);
	JPH_ASSERT(num_contact_points == (int)inManifold.mWorldSpaceContactPointsOn2.size());

	// Reserve space for new contact cache entry
	// Note that for dynamic vs dynamic we always require the first body to have a lower body id to get a consistent key
	// under which to look up the contact
	ManifoldCache &write_cache = mCache[mCacheWriteIdx];
	MKeyValue *new_manifold_kv = write_cache.Create(ioContactAllocator, key, key_hash, num_contact_points);
	if (new_manifold_kv == nullptr)
		return false; // Out of cache space
	CachedManifold *new_manifold = &new_manifold_kv->GetValue();

	// Transform the world space normal to the space of body 2 (this is usually the static body)
	Mat44 inverse_transform_body2 = inBody2.GetInverseCenterOfMassTransform();
	inverse_transform_body2.Multiply3x3(inManifold.mWorldSpaceNormal).Normalized().StoreFloat3(&new_manifold->mContactNormal);

	// Settings object that gets passed to the callback
	ContactSettings settings;
	settings.mCombinedFriction = mCombineFriction(inBody1, inManifold.mSubShapeID1, inBody2, inManifold.mSubShapeID2);
	settings.mCombinedRestitution = mCombineRestitution(inBody1, inManifold.mSubShapeID1, inBody2, inManifold.mSubShapeID2);
	settings.mIsSensor = inBody1.IsSensor() || inBody2.IsSensor();

	// Get the contact points for the old cache entry
	const ManifoldCache &read_cache = mCache[mCacheWriteIdx ^ 1];
	const MKeyValue *old_manifold_kv = read_cache.Find(key, key_hash);
	const CachedContactPoint *ccp_start;
	const CachedContactPoint *ccp_end;
	if (old_manifold_kv != nullptr)
	{
		// Call point persisted listener
		if (mContactListener != nullptr)
			mContactListener->OnContactPersisted(inBody1, inBody2, inManifold, settings);

		// Fetch the contact points from the old manifold
		const CachedManifold *old_manifold = &old_manifold_kv->GetValue();
		ccp_start = old_manifold->mContactPoints;
		ccp_end = ccp_start + old_manifold->mNumContactPoints;

		// Mark contact as persisted so that we won't fire OnContactRemoved callbacks
		old_manifold->mFlags |= (uint16)CachedManifold::EFlags::ContactPersisted;
	}
	else
	{
		// Call point added listener
		if (mContactListener != nullptr)
			mContactListener->OnContactAdded(inBody1, inBody2, inManifold, settings);

		// No contact points available from old manifold
		ccp_start = nullptr;
		ccp_end = nullptr;
	}

	// Get inverse transform for body 1
	Mat44 inverse_transform_body1 = inBody1.GetInverseCenterOfMassTransform();

	bool contact_constraint_created;

	// If one of the bodies is a sensor, don't actually create the constraint
	JPH_ASSERT(settings.mIsSensor || !(inBody1.IsSensor() || inBody2.IsSensor()), "Sensors cannot be converted into regular bodies by a contact callback!");
	if (settings.mIsSensor)
	{
		// Store the contact manifold in the cache
		for (int i = 0; i < num_contact_points; ++i)
		{
			// Convert to local space to the body
			Vec3 p1 = inverse_transform_body1 * inManifold.mWorldSpaceContactPointsOn1[i];
			Vec3 p2 = inverse_transform_body2 * inManifold.mWorldSpaceContactPointsOn2[i];

			// Create new contact point
			CachedContactPoint &cp = new_manifold->mContactPoints[i];
			p1.StoreFloat3(&cp.mPosition1);
			p2.StoreFloat3(&cp.mPosition2);

			// We don't use this, but reset them anyway for determinism check
			cp.mNonPenetrationLambda = 0.0f;
			cp.mFrictionLambda[0] = 0.0f;
			cp.mFrictionLambda[1] = 0.0f;
		}

		// No constraint created
		contact_constraint_created = false;
	}
	else
	{
		// Add contact constraint
		uint32 constraint_idx = mNumConstraints++;
		if (constraint_idx >= mMaxConstraints)
		{
			JPH_ASSERT(false, "Out of contact constraints!");

			// Manifold has been created already, we're not filling it in, so we need to reset the contact number of points.
			// Note that we don't hook it up to the body pair cache so that it won't be used as a cache during the next simulation.
			new_manifold->mNumContactPoints = 0; 
			return false;
		}

		// We will create a contact constraint
		contact_constraint_created = true;
		
		ContactConstraint &constraint = mConstraints[constraint_idx];
		new (&constraint) ContactConstraint();
		constraint.mWorldSpaceNormal = inManifold.mWorldSpaceNormal;
		constraint.mBody1 = &inBody1;
		constraint.mBody2 = &inBody2;
		constraint.mSortKey = key_hash;
		constraint.mCombinedFriction = settings.mCombinedFriction;
		constraint.mCombinedRestitution = settings.mCombinedRestitution;

		// Notify island builder
		mUpdateContext->mIslandBuilder->LinkContact(constraint_idx, inBody1.GetIndexInActiveBodiesInternal(), inBody2.GetIndexInActiveBodiesInternal());

		// Get time step
		float delta_time = mUpdateContext->mSubStepDeltaTime;

		// Calculate tangents
		Vec3 t1, t2;
		constraint.GetTangents(t1, t2);

		constraint.mContactPoints.resize(num_contact_points);
		for (int i = 0; i < num_contact_points; ++i)
		{
			// Convert to world space and set positions
			WorldContactPoint &wcp = constraint.mContactPoints[i];
			Vec3 p1_ws = inManifold.mWorldSpaceContactPointsOn1[i];
			Vec3 p2_ws = inManifold.mWorldSpaceContactPointsOn2[i];

			// Convert to local space to the body
			Vec3 p1_ls = inverse_transform_body1 * p1_ws;
			Vec3 p2_ls = inverse_transform_body2 * p2_ws;
						
			// Check if we have a close contact point from last update
			bool lambda_set = false;
			for (const CachedContactPoint *ccp = ccp_start; ccp < ccp_end; ccp++)
				if (Vec3::sLoadFloat3Unsafe(ccp->mPosition1).IsClose(p1_ls, mPhysicsSettings.mContactPointPreserveLambdaMaxDistSq) 
					&& Vec3::sLoadFloat3Unsafe(ccp->mPosition2).IsClose(p2_ls, mPhysicsSettings.mContactPointPreserveLambdaMaxDistSq))
				{
					// Get lambdas from previous frame
					wcp.mNonPenetrationConstraint.SetTotalLambda(ccp->mNonPenetrationLambda);
					wcp.mFrictionConstraint1.SetTotalLambda(ccp->mFrictionLambda[0]);
					wcp.mFrictionConstraint2.SetTotalLambda(ccp->mFrictionLambda[1]);
					lambda_set = true;
					break;
				}
			if (!lambda_set)
			{
				wcp.mNonPenetrationConstraint.SetTotalLambda(0.0f);
				wcp.mFrictionConstraint1.SetTotalLambda(0.0f);
				wcp.mFrictionConstraint2.SetTotalLambda(0.0f);
			}

			// Create new contact point
			CachedContactPoint &cp = new_manifold->mContactPoints[i];
			p1_ls.StoreFloat3(&cp.mPosition1);
			p2_ls.StoreFloat3(&cp.mPosition2);
			wcp.mContactPoint = &cp;

			// Setup velocity constraint
			wcp.CalculateFrictionAndNonPenetrationConstraintProperties<Type1, Type2>(delta_time, inBody1, inBody2, inInvI1, inInvI2, p1_ws, p2_ws, inManifold.mWorldSpaceNormal, t1, t2, settings.mCombinedRestitution, settings.mCombinedFriction, mPhysicsSettings.mMinVelocityForRestitution);
		}

	#ifdef JPH_DEBUG_RENDERER
		// Draw the manifold
		if (sDrawContactManifolds)
			constraint.Draw(DebugRenderer::sInstance, Color::sOrange);
	#endif // JPH_DEBUG_RENDERER
	}

	// Store cached contact point in body pair cache
	CachedBodyPair *cbp = reinterpret_cast<CachedBodyPair *>(inBodyPairHandle);
	new_manifold->mNextWithSameBodyPair = cbp->mFirstCachedManifold;
	cbp->mFirstCachedManifold = write_cache.ToHandle(new_manifold_kv);

	// A contact constraint was added
	return contact_constraint_created;
}

bool ContactConstraintManager::AddContactConstraint(ContactAllocator &ioContactAllocator, BodyPairHandle inBodyPairHandle, Body &inBody1, Body &inBody2, const ContactManifold &inManifold)
{
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inManifold.mWorldSpaceNormal.IsNormalized());

	// Swap bodies so that body 1 id < body 2 id
	const ContactManifold *manifold;
	Body *body1, *body2;
	ContactManifold temp;
	if (inBody2.GetID() < inBody1.GetID())
	{
		body1 = &inBody2;
		body2 = &inBody1;
		temp = inManifold.SwapShapes();
		manifold = &temp;
	}
	else
	{
		body1 = &inBody1;
		body2 = &inBody2;
		manifold = &inManifold;
	}

	// Dispatch to the correct templated form
	// Note: Non-dynamic vs non-dynamic can happen in this case due to one body being a sensor, so we need to have an extended switch case here
	switch (body1->GetMotionType())
	{
	case EMotionType::Dynamic:
		{
			Mat44 invi1 = body1->GetInverseInertia();
			switch (body2->GetMotionType())
			{
			case EMotionType::Dynamic:
				return TemplatedAddContactConstraint<EMotionType::Dynamic, EMotionType::Dynamic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, invi1, body2->GetInverseInertia());

			case EMotionType::Kinematic:
				return TemplatedAddContactConstraint<EMotionType::Dynamic, EMotionType::Kinematic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, invi1, Mat44() /* Will not be used */);

			case EMotionType::Static:
				return TemplatedAddContactConstraint<EMotionType::Dynamic, EMotionType::Static>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, invi1, Mat44() /* Will not be used */);

			default:
				JPH_ASSERT(false);
				break;
			}
			break;
		}

	case EMotionType::Kinematic:
		switch (body2->GetMotionType())
		{
		case EMotionType::Dynamic:
			return TemplatedAddContactConstraint<EMotionType::Kinematic, EMotionType::Dynamic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, Mat44() /* Will not be used */, body2->GetInverseInertia());

		case EMotionType::Kinematic:
			return TemplatedAddContactConstraint<EMotionType::Kinematic, EMotionType::Kinematic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, Mat44() /* Will not be used */, Mat44() /* Will not be used */);

		case EMotionType::Static:
			return TemplatedAddContactConstraint<EMotionType::Kinematic, EMotionType::Static>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, Mat44() /* Will not be used */, Mat44() /* Will not be used */);

		default:
			JPH_ASSERT(false);
			break;
		}
		break;

	case EMotionType::Static:
		switch (body2->GetMotionType())
		{
		case EMotionType::Dynamic:
			return TemplatedAddContactConstraint<EMotionType::Static, EMotionType::Dynamic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, Mat44() /* Will not be used */, body2->GetInverseInertia());

		case EMotionType::Kinematic:
			return TemplatedAddContactConstraint<EMotionType::Static, EMotionType::Kinematic>(ioContactAllocator, inBodyPairHandle, *body1, *body2, *manifold, Mat44() /* Will not be used */, Mat44() /* Will not be used */);

		case EMotionType::Static: // Static vs static not possible
		default: 
			JPH_ASSERT(false);
			break;
		}
		break;

	default:
		JPH_ASSERT(false);
		break;
	}

	return false;
}

void ContactConstraintManager::OnCCDContactAdded(ContactAllocator &ioContactAllocator, const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &outSettings)
{
	JPH_ASSERT(inManifold.mWorldSpaceNormal.IsNormalized());

	// Calculate contact settings
	outSettings.mCombinedFriction = mCombineFriction(inBody1, inManifold.mSubShapeID1, inBody2, inManifold.mSubShapeID2);
	outSettings.mCombinedRestitution = mCombineRestitution(inBody1, inManifold.mSubShapeID1, inBody2, inManifold.mSubShapeID2);
	outSettings.mIsSensor = false; // For now, no sensors are supported during CCD

	// The remainder of this function only deals with calling contact callbacks, if there's no contact callback we also don't need to do this work
	if (mContactListener != nullptr)
	{
		// Swap bodies so that body 1 id < body 2 id
		const ContactManifold *manifold;
		const Body *body1, *body2;
		ContactManifold temp;
		if (inBody2.GetID() < inBody1.GetID())
		{
			body1 = &inBody2;
			body2 = &inBody1;
			temp = inManifold.SwapShapes();
			manifold = &temp;
		}
		else
		{
			body1 = &inBody1;
			body2 = &inBody2;
			manifold = &inManifold;
		}

		// Calculate hash
		SubShapeIDPair key { body1->GetID(), manifold->mSubShapeID1, body2->GetID(), manifold->mSubShapeID2 };
		size_t key_hash = std::hash<SubShapeIDPair> {} (key);

		// Check if we already created this contact this physics update
		ManifoldCache &write_cache = mCache[mCacheWriteIdx];
		MKVAndCreated new_manifold_kv = write_cache.FindOrCreate(ioContactAllocator, key, key_hash, 0);
		if (new_manifold_kv.second)
		{
			// This contact is new for this physics update, check if previous update we already had this contact.
			const ManifoldCache &read_cache = mCache[mCacheWriteIdx ^ 1];
			const MKeyValue *old_manifold_kv = read_cache.Find(key, key_hash);
			if (old_manifold_kv == nullptr)
			{
				// New contact
				mContactListener->OnContactAdded(*body1, *body2, *manifold, outSettings);
			}
			else
			{
				// Existing contact
				mContactListener->OnContactPersisted(*body1, *body2, *manifold, outSettings);

				// Mark contact as persisted so that we won't fire OnContactRemoved callbacks
				old_manifold_kv->GetValue().mFlags |= (uint16)CachedManifold::EFlags::ContactPersisted;
			}

			// Check if the cache is full
			if (new_manifold_kv.first != nullptr)
			{
				// We don't store any contact points in this manifold as it is not for caching impulses, we only need to know that the contact was created
				CachedManifold &new_manifold = new_manifold_kv.first->GetValue();
				new_manifold.mContactNormal = { 0, 0, 0 };
				new_manifold.mFlags |= (uint16)CachedManifold::EFlags::CCDContact;
			}
		}
		else
		{
			// Already found this contact this physics update. 
			// Note that we can trigger OnContactPersisted multiple times per physics update, but otherwise we have no way of obtaining the settings
			mContactListener->OnContactPersisted(*body1, *body2, *manifold, outSettings);
		}
	}

	JPH_ASSERT(!outSettings.mIsSensor, "CCD bodies cannot currently act as sensors");
}

void ContactConstraintManager::SortContacts(uint32 *inConstraintIdxBegin, uint32 *inConstraintIdxEnd) const
{
	JPH_PROFILE_FUNCTION();

	sort(inConstraintIdxBegin, inConstraintIdxEnd, [this](uint32 inLHS, uint32 inRHS) {
		const ContactConstraint &lhs = mConstraints[inLHS];
		const ContactConstraint &rhs = mConstraints[inRHS];
		JPH_ASSERT(lhs.mSortKey != rhs.mSortKey, "Hash collision, ordering will be inconsistent");
		return lhs.mSortKey < rhs.mSortKey;
	});
}

void ContactConstraintManager::FinalizeContactCache(uint inExpectedNumBodyPairs, uint inExpectedNumManifolds)
{
	JPH_PROFILE_FUNCTION();

#ifdef JPH_ENABLE_ASSERTS
	// Mark cache as finalized
	ManifoldCache &old_write_cache = mCache[mCacheWriteIdx];
	old_write_cache.Finalize();

	// Check that the count of body pairs and manifolds that we tracked outside of the cache (to avoid contention on an atomic) is correct
	JPH_ASSERT(old_write_cache.GetNumBodyPairs() == inExpectedNumBodyPairs);
	JPH_ASSERT(old_write_cache.GetNumManifolds() == inExpectedNumManifolds);
#endif

	// Buffers are now complete, make write buffer the read buffer
	mCacheWriteIdx ^= 1;

	// Use the amount of contacts from the last iteration to determine the amount of buckets to use in the hash map for the next iteration
	mCache[mCacheWriteIdx].Prepare(inExpectedNumBodyPairs, inExpectedNumManifolds);
}

void ContactConstraintManager::ContactPointRemovedCallbacks()
{
	JPH_PROFILE_FUNCTION();

	// Get the read cache
	ManifoldCache &read_cache = mCache[mCacheWriteIdx ^ 1];

	// Call the actual callbacks
	if (mContactListener != nullptr)
		read_cache.ContactPointRemovedCallbacks(mContactListener);

	// We're done with the cache now
	read_cache.Clear();
}

void ContactConstraintManager::SetupVelocityConstraints(const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inDeltaTime)
{
	JPH_PROFILE_FUNCTION();

	for (const uint32 *constraint_idx = inConstraintIdxBegin; constraint_idx < inConstraintIdxEnd; ++constraint_idx)
	{
		ContactConstraint &constraint = mConstraints[*constraint_idx];

		// Fetch bodies
		const Body &body1 = *constraint.mBody1;
		const Body &body2 = *constraint.mBody2;
		
		// Get body transforms
		Mat44 transform_body1 = body1.GetCenterOfMassTransform();
		Mat44 transform_body2 = body2.GetCenterOfMassTransform();

		// Calculate friction and non-penetration constraint properties for all contact points
		CalculateFrictionAndNonPenetrationConstraintProperties(constraint, inDeltaTime, transform_body1, transform_body2, body1, body2);
	}
}

template <EMotionType Type1, EMotionType Type2>
JPH_INLINE void ContactConstraintManager::sWarmStartConstraint(ContactConstraint &ioConstraint, MotionProperties *ioMotionProperties1, MotionProperties *ioMotionProperties2, float inWarmStartImpulseRatio)
{
	// Calculate tangents
	Vec3 t1, t2;
	ioConstraint.GetTangents(t1, t2);
		
	for (WorldContactPoint &wcp : ioConstraint.mContactPoints)
	{
		// Warm starting: Apply impulse from last frame
		if (wcp.mFrictionConstraint1.IsActive())
		{
			JPH_ASSERT(wcp.mFrictionConstraint2.IsActive());
			wcp.mFrictionConstraint1.TemplatedWarmStart<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, t1, inWarmStartImpulseRatio);
			wcp.mFrictionConstraint2.TemplatedWarmStart<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, t2, inWarmStartImpulseRatio);
		}
		wcp.mNonPenetrationConstraint.TemplatedWarmStart<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, ioConstraint.mWorldSpaceNormal, inWarmStartImpulseRatio);
	}
}

void ContactConstraintManager::WarmStartVelocityConstraints(const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd, float inWarmStartImpulseRatio)
{
	JPH_PROFILE_FUNCTION();

	for (const uint32 *constraint_idx = inConstraintIdxBegin; constraint_idx < inConstraintIdxEnd; ++constraint_idx)
	{
		ContactConstraint &constraint = mConstraints[*constraint_idx];

		// Fetch bodies
		Body &body1 = *constraint.mBody1;
		EMotionType motion_type1 = body1.GetMotionType();
		MotionProperties *motion_properties1 = body1.GetMotionPropertiesUnchecked();

		Body &body2 = *constraint.mBody2;
		EMotionType motion_type2 = body2.GetMotionType();
		MotionProperties *motion_properties2 = body2.GetMotionPropertiesUnchecked();
				
		// Dispatch to the correct templated form
		// Note: Warm starting doesn't differentiate between kinematic/static bodies so we handle both as static bodies
		if (motion_type1 == EMotionType::Dynamic)
		{
			if (motion_type2 == EMotionType::Dynamic)
				sWarmStartConstraint<EMotionType::Dynamic, EMotionType::Dynamic>(constraint, motion_properties1, motion_properties2, inWarmStartImpulseRatio);
			else
				sWarmStartConstraint<EMotionType::Dynamic, EMotionType::Static>(constraint, motion_properties1, motion_properties2, inWarmStartImpulseRatio);
		}
		else
		{
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			sWarmStartConstraint<EMotionType::Static, EMotionType::Dynamic>(constraint, motion_properties1, motion_properties2, inWarmStartImpulseRatio);
		}
	}
}

template <EMotionType Type1, EMotionType Type2>
JPH_INLINE bool ContactConstraintManager::sSolveVelocityConstraint(ContactConstraint &ioConstraint, MotionProperties *ioMotionProperties1, MotionProperties *ioMotionProperties2)
{
	bool any_impulse_applied = false;

	// Calculate tangents
	Vec3 t1, t2;
	ioConstraint.GetTangents(t1, t2);

	// First apply all friction constraints (non-penetration is more important than friction)
	for (WorldContactPoint &wcp : ioConstraint.mContactPoints)
	{
		// Check if friction is enabled
		if (wcp.mFrictionConstraint1.IsActive())
		{
			JPH_ASSERT(wcp.mFrictionConstraint2.IsActive());

			// Calculate max impulse that can be applied. Note that we're using the non-penetration impulse from the previous iteration here.
			// We do this because non-penetration is more important so is solved last (the last things that are solved in an iterative solver
			// contribute the most).
			float max_lambda_f = ioConstraint.mCombinedFriction * wcp.mNonPenetrationConstraint.GetTotalLambda();

			// Solve friction velocities
			// Note that what we're doing is not fully correct since the max force we can apply is 2 * max_lambda_f instead of max_lambda_f since we're solving axis independently
			if (wcp.mFrictionConstraint1.TemplatedSolveVelocityConstraint<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, t1, -max_lambda_f, max_lambda_f))
				any_impulse_applied = true;
			if (wcp.mFrictionConstraint2.TemplatedSolveVelocityConstraint<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, t2, -max_lambda_f, max_lambda_f))
				any_impulse_applied = true;
		}
	}

	// Then apply all non-penetration constraints
	for (WorldContactPoint &wcp : ioConstraint.mContactPoints)
	{
		// Solve non penetration velocities
		if (wcp.mNonPenetrationConstraint.TemplatedSolveVelocityConstraint<Type1, Type2>(ioMotionProperties1, ioMotionProperties2, ioConstraint.mWorldSpaceNormal, 0.0f, FLT_MAX))
			any_impulse_applied = true;
	}

	return any_impulse_applied;
}

bool ContactConstraintManager::SolveVelocityConstraints(const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd)
{
	JPH_PROFILE_FUNCTION();

	bool any_impulse_applied = false;

	for (const uint32 *constraint_idx = inConstraintIdxBegin; constraint_idx < inConstraintIdxEnd; ++constraint_idx)
	{
		ContactConstraint &constraint = mConstraints[*constraint_idx];

		// Fetch bodies
		Body &body1 = *constraint.mBody1;
		EMotionType motion_type1 = body1.GetMotionType();
		MotionProperties *motion_properties1 = body1.GetMotionPropertiesUnchecked();

		Body &body2 = *constraint.mBody2;
		EMotionType motion_type2 = body2.GetMotionType();
		MotionProperties *motion_properties2 = body2.GetMotionPropertiesUnchecked();

		// Dispatch to the correct templated form
		switch (motion_type1)
		{
		case EMotionType::Dynamic:
			switch (motion_type2)
			{
			case EMotionType::Dynamic:
				any_impulse_applied |= sSolveVelocityConstraint<EMotionType::Dynamic, EMotionType::Dynamic>(constraint, motion_properties1, motion_properties2);
				break;

			case EMotionType::Kinematic:
				any_impulse_applied |= sSolveVelocityConstraint<EMotionType::Dynamic, EMotionType::Kinematic>(constraint, motion_properties1, motion_properties2);
				break;

			case EMotionType::Static:
				any_impulse_applied |= sSolveVelocityConstraint<EMotionType::Dynamic, EMotionType::Static>(constraint, motion_properties1, motion_properties2);
				break;

			default:
				JPH_ASSERT(false);
				break;
			}
			break;

		case EMotionType::Kinematic:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			any_impulse_applied |= sSolveVelocityConstraint<EMotionType::Kinematic, EMotionType::Dynamic>(constraint, motion_properties1, motion_properties2);
			break;

		case EMotionType::Static:
			JPH_ASSERT(motion_type2 == EMotionType::Dynamic);
			any_impulse_applied |= sSolveVelocityConstraint<EMotionType::Static, EMotionType::Dynamic>(constraint, motion_properties1, motion_properties2);
			break;

		default:
			JPH_ASSERT(false);
			break;
		}
	}

	return any_impulse_applied;
}

void ContactConstraintManager::StoreAppliedImpulses(const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd) const
{
	// Copy back total applied impulse to cache for the next frame
	for (const uint32 *constraint_idx = inConstraintIdxBegin; constraint_idx < inConstraintIdxEnd; ++constraint_idx)
	{
		const ContactConstraint &constraint = mConstraints[*constraint_idx];

		for (const WorldContactPoint &wcp : constraint.mContactPoints)
		{
			wcp.mContactPoint->mNonPenetrationLambda = wcp.mNonPenetrationConstraint.GetTotalLambda();
			wcp.mContactPoint->mFrictionLambda[0] = wcp.mFrictionConstraint1.GetTotalLambda();
			wcp.mContactPoint->mFrictionLambda[1] = wcp.mFrictionConstraint2.GetTotalLambda();
		}
	}
}

bool ContactConstraintManager::SolvePositionConstraints(const uint32 *inConstraintIdxBegin, const uint32 *inConstraintIdxEnd)
{
	JPH_PROFILE_FUNCTION();

	bool any_impulse_applied = false;

	float delta_time = mUpdateContext->mSubStepDeltaTime;

	for (const uint32 *constraint_idx = inConstraintIdxBegin; constraint_idx < inConstraintIdxEnd; ++constraint_idx)
	{
		ContactConstraint &constraint = mConstraints[*constraint_idx];

		// Fetch bodies
		Body &body1 = *constraint.mBody1;
		Body &body2 = *constraint.mBody2;

		// Get transforms
		Mat44 transform1 = body1.GetCenterOfMassTransform();
		Mat44 transform2 = body2.GetCenterOfMassTransform();

		for (WorldContactPoint &wcp : constraint.mContactPoints)
		{
			// Calculate new contact point positions in world space (the bodies may have moved)
			Vec3 p1 = transform1 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition1);
			Vec3 p2 = transform2 * Vec3::sLoadFloat3Unsafe(wcp.mContactPoint->mPosition2);

			// Calculate separation along the normal (negative if interpenetrating)
			// Allow a little penetration by default (PhysicsSettings::mPenetrationSlop) to avoid jittering between contact/no-contact which wipes out the contact cache and warm start impulses
			// Clamp penetration to a max PhysicsSettings::mMaxPenetrationDistance so that we don't apply a huge impulse if we're penetrating a lot
			float separation = max((p2 - p1).Dot(constraint.mWorldSpaceNormal) + mPhysicsSettings.mPenetrationSlop, -mPhysicsSettings.mMaxPenetrationDistance);

			// Only enforce constraint when separation < 0 (otherwise we're apart)
			if (separation < 0.0f)
			{
				// Update constraint properties (bodies may have moved)
				wcp.CalculateNonPenetrationConstraintProperties(delta_time, body1, body2, p1, p2, constraint.mWorldSpaceNormal);

				// Solve position errors
				if (wcp.mNonPenetrationConstraint.SolvePositionConstraint(body1, body2, constraint.mWorldSpaceNormal, separation, mPhysicsSettings.mBaumgarte))
					any_impulse_applied = true;
			}
		}
	}

	return any_impulse_applied;
}

void ContactConstraintManager::RecycleConstraintBuffer()
{
	// Reset constraint array
	mNumConstraints = 0;
}

void ContactConstraintManager::FinishConstraintBuffer()
{
	// Free constraints buffer
	mUpdateContext->mTempAllocator->Free(mConstraints, mMaxConstraints * sizeof(ContactConstraint));
	mConstraints = nullptr;
	mNumConstraints = 0;

	// Reset update context
	mUpdateContext = nullptr;
}

void ContactConstraintManager::SaveState(StateRecorder &inStream) const
{
	mCache[mCacheWriteIdx ^ 1].SaveState(inStream);
}

bool ContactConstraintManager::RestoreState(StateRecorder &inStream)
{
	bool success = mCache[mCacheWriteIdx].RestoreState(mCache[mCacheWriteIdx ^ 1], inStream);
	mCacheWriteIdx ^= 1;
	mCache[mCacheWriteIdx].Clear();
	return success;
}

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Utils/ContactListenerImpl.h>
#include <Renderer/DebugRendererImp.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Core/QuickSort.h>

ValidateResult ContactListenerImpl::OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult)
{
	// Check ordering contract between body 1 and body 2
	bool contract = inBody1.GetMotionType() >= inBody2.GetMotionType()
		|| (inBody1.GetMotionType() == inBody2.GetMotionType() && inBody1.GetID() < inBody2.GetID());
	if (!contract)
		JPH_BREAKPOINT;

	ValidateResult result;
	if (mNext != nullptr)
		result = mNext->OnContactValidate(inBody1, inBody2, inBaseOffset, inCollisionResult);
	else
		result = ContactListener::OnContactValidate(inBody1, inBody2, inBaseOffset, inCollisionResult);

	RVec3 contact_point = inBaseOffset + inCollisionResult.mContactPointOn1;
	DebugRenderer::sInstance->DrawArrow(contact_point, contact_point - inCollisionResult.mPenetrationAxis.NormalizedOr(Vec3::sZero()), Color::sBlue, 0.05f);

	Trace("Validate %u and %u result %d", inBody1.GetID().GetIndex(), inBody2.GetID().GetIndex(), (int)result);

	return result;
}

void ContactListenerImpl::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Expect bodies to be sorted
	if (!(inBody1.GetID() < inBody2.GetID()))
		JPH_BREAKPOINT;

	Trace("Contact added %u (%08x) and %u (%08x)", inBody1.GetID().GetIndex(), inManifold.mSubShapeID1.GetValue(), inBody2.GetID().GetIndex(), inManifold.mSubShapeID2.GetValue());

	DebugRenderer::sInstance->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn1, Color::sGreen, 0.05f);
	DebugRenderer::sInstance->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn2, Color::sGreen, 0.05f);
	DebugRenderer::sInstance->DrawArrow(inManifold.GetWorldSpaceContactPointOn1(0), inManifold.GetWorldSpaceContactPointOn1(0) + inManifold.mWorldSpaceNormal, Color::sGreen, 0.05f);

	// Insert new manifold into state map
	{
		lock_guard lock(mStateMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		if (mState.find(key) != mState.end())
			JPH_BREAKPOINT; // Added contact that already existed
		mState[key] = StatePair(inManifold.mBaseOffset, inManifold.mRelativeContactPointsOn1);
	}

	if (mNext != nullptr)
		mNext->OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
}

void ContactListenerImpl::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Expect bodies to be sorted
	if (!(inBody1.GetID() < inBody2.GetID()))
		JPH_BREAKPOINT;

	Trace("Contact persisted %u (%08x) and %u (%08x)", inBody1.GetID().GetIndex(), inManifold.mSubShapeID1.GetValue(), inBody2.GetID().GetIndex(), inManifold.mSubShapeID2.GetValue());

	DebugRenderer::sInstance->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn1, Color::sYellow, 0.05f);
	DebugRenderer::sInstance->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn2, Color::sYellow, 0.05f);
	DebugRenderer::sInstance->DrawArrow(inManifold.GetWorldSpaceContactPointOn1(0), inManifold.GetWorldSpaceContactPointOn1(0) + inManifold.mWorldSpaceNormal, Color::sYellow, 0.05f);

	// Update existing manifold in state map
	{
		lock_guard lock(mStateMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		StateMap::iterator i = mState.find(key);
		if (i != mState.end())
			i->second = StatePair(inManifold.mBaseOffset, inManifold.mRelativeContactPointsOn1);
		else
			JPH_BREAKPOINT; // Persisted contact that didn't exist
	}

	if (mNext != nullptr)
		mNext->OnContactPersisted(inBody1, inBody2, inManifold, ioSettings);
}

void ContactListenerImpl::OnContactRemoved(const SubShapeIDPair &inSubShapePair)
{
	// Expect bodies to be sorted
	if (!(inSubShapePair.GetBody1ID() < inSubShapePair.GetBody2ID()))
		JPH_BREAKPOINT;

	Trace("Contact removed %u (%08x) and %u (%08x)", inSubShapePair.GetBody1ID().GetIndex(), inSubShapePair.GetSubShapeID1().GetValue(), inSubShapePair.GetBody2ID().GetIndex(), inSubShapePair.GetSubShapeID2().GetValue());

	// Update existing manifold in state map
	{
		lock_guard lock(mStateMutex);
		StateMap::iterator i = mState.find(inSubShapePair);
		if (i != mState.end())
			mState.erase(i);
		else
			JPH_BREAKPOINT; // Removed contact that didn't exist
	}

	if (mNext != nullptr)
		mNext->OnContactRemoved(inSubShapePair);
}

void ContactListenerImpl::SaveState(StateRecorder &inStream) const
{
	// Write length
	uint32 length = uint32(mState.size());
	inStream.Write(length);

	// Get and sort keys
	Array<SubShapeIDPair> keys;
	for (const StateMap::value_type &kv : mState)
		keys.push_back(kv.first);
	QuickSort(keys.begin(), keys.end());

	// Write key value pairs
	for (const SubShapeIDPair &k : keys)
	{
		// Write key
		inStream.Write(k);

		// Write value
		const StatePair &sp = mState.find(k)->second;
		inStream.Write(sp.first);
		inStream.Write(uint32(sp.second.size()));
		inStream.WriteBytes(sp.second.data(), sp.second.size() * sizeof(Vec3));
	}
}

void ContactListenerImpl::RestoreState(StateRecorder &inStream)
{
	Trace("Restore Contact State");

	// Read length
	uint32 length;
	if (inStream.IsValidating())
		length = uint32(mState.size());
	inStream.Read(length);

	Array<SubShapeIDPair> keys;

	// Clear the state and remember the old state for validation
	StateMap old_state;
	old_state.swap(mState);

	// Prepopulate keys and values with current values if we're validating
	if (inStream.IsValidating())
	{
		// Get and sort keys
		for (const StateMap::value_type &kv : old_state)
			keys.push_back(kv.first);
		QuickSort(keys.begin(), keys.end());
	}

	// Ensure we have the correct size
	keys.resize(length);

	for (size_t i = 0; i < length; ++i)
	{
		// Read key
		SubShapeIDPair key;
		if (inStream.IsValidating())
			key = keys[i];
		inStream.Read(key);

		StatePair sp;
		if (inStream.IsValidating())
			sp = old_state[key];

		// Read offset
		inStream.Read(sp.first);

		// Read num contact points
		uint32 num_contacts;
		if (inStream.IsValidating())
			num_contacts = uint32(old_state[key].second.size());
		inStream.Read(num_contacts);

		// Read contact points
		sp.second.resize(num_contacts);
		inStream.ReadBytes(sp.second.data(), num_contacts * sizeof(Vec3));

		// Store the new value
		mState[key] = sp;
	}
}

void ContactListenerImpl::DrawState()
{
	Trace("Draw Contact State");

	lock_guard lock(mStateMutex);
	for (const StateMap::value_type &kv : mState)
		for (Vec3 v : kv.second.second)
			DebugRenderer::sInstance->DrawWireSphere(kv.second.first + v, 0.05f, Color::sRed, 1);
}

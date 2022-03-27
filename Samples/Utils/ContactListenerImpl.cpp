// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Utils/ContactListenerImpl.h>
#include <Renderer/DebugRendererImp.h>
#include <Jolt/Physics/Body/Body.h>

ValidateResult ContactListenerImpl::OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult)
{
	// Expect body 1 to be dynamic (or one of the bodies must be a sensor)
	if (!inBody1.IsDynamic() && !inBody1.IsSensor() && !inBody2.IsSensor())
		JPH_BREAKPOINT;

	ValidateResult result;
	if (mNext != nullptr)
		result = mNext->OnContactValidate(inBody1, inBody2, inCollisionResult);
	else
		result = ContactListener::OnContactValidate(inBody1, inBody2, inCollisionResult);

	Trace("Validate %d and %d result %d", inBody1.GetID().GetIndex(), inBody2.GetID().GetIndex(), (int)result);

	return result;
}

void ContactListenerImpl::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Expect bodies to be sorted
	if (!(inBody1.GetID() < inBody2.GetID()))
		JPH_BREAKPOINT;

	Trace("Contact added %d (%08x) and %d (%08x)", inBody1.GetID().GetIndex(), inManifold.mSubShapeID1.GetValue(), inBody2.GetID().GetIndex(), inManifold.mSubShapeID2.GetValue());

	DebugRenderer::sInstance->DrawWirePolygon(inManifold.mWorldSpaceContactPointsOn1, Color::sGreen, 0.05f);
	DebugRenderer::sInstance->DrawWirePolygon(inManifold.mWorldSpaceContactPointsOn2, Color::sGreen, 0.05f);
	DebugRenderer::sInstance->DrawArrow(inManifold.mWorldSpaceContactPointsOn1[0], inManifold.mWorldSpaceContactPointsOn1[0] + inManifold.mWorldSpaceNormal, Color::sGreen, 0.05f);

	// Insert new manifold into state map
	{
		lock_guard lock(mStateMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		if (mState.find(key) != mState.end())
			JPH_BREAKPOINT; // Added contact that already existed
		mState[key] = inManifold.mWorldSpaceContactPointsOn1;
	}

	if (mNext != nullptr)
		mNext->OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
}

void ContactListenerImpl::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Expect bodies to be sorted
	if (!(inBody1.GetID() < inBody2.GetID()))
		JPH_BREAKPOINT;

	Trace("Contact persisted %d (%08x) and %d (%08x)", inBody1.GetID().GetIndex(), inManifold.mSubShapeID1.GetValue(), inBody2.GetID().GetIndex(), inManifold.mSubShapeID2.GetValue());

	DebugRenderer::sInstance->DrawWirePolygon(inManifold.mWorldSpaceContactPointsOn1, Color::sYellow, 0.05f);
	DebugRenderer::sInstance->DrawWirePolygon(inManifold.mWorldSpaceContactPointsOn2, Color::sYellow, 0.05f);
	DebugRenderer::sInstance->DrawArrow(inManifold.mWorldSpaceContactPointsOn1[0], inManifold.mWorldSpaceContactPointsOn1[0] + inManifold.mWorldSpaceNormal, Color::sYellow, 0.05f);

	// Update existing manifold in state map
	{
		lock_guard lock(mStateMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		StateMap::iterator i = mState.find(key);
		if (i != mState.end())
			i->second = inManifold.mWorldSpaceContactPointsOn1;
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

	Trace("Contact removed %d (%08x) and %d (%08x)", inSubShapePair.GetBody1ID().GetIndex(), inSubShapePair.GetSubShapeID1().GetValue(), inSubShapePair.GetBody2ID().GetIndex(), inSubShapePair.GetSubShapeID2().GetValue());

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
	inStream.Write(mState.size());

	// Get and sort keys
	vector<SubShapeIDPair> keys;
	for (const StateMap::value_type &kv : mState)
		keys.push_back(kv.first);
	sort(keys.begin(), keys.end());

	// Write key value pairs
	for (const SubShapeIDPair &k : keys)
	{
		// Write key
		inStream.Write(k);

		// Write value
		const ContactPoints &cp = mState.find(k)->second;
		inStream.Write(cp.size());
		inStream.WriteBytes(cp.data(), cp.size() * sizeof(Vec3));
	}
}

void ContactListenerImpl::RestoreState(StateRecorder &inStream)
{
	Trace("Restore Contact State");

	// Read length
	StateMap::size_type length;
	if (inStream.IsValidating())
		length = mState.size();
	inStream.Read(length);

	vector<SubShapeIDPair> keys;

	// Clear the state and remember the old state for validation
	StateMap old_state;
	old_state.swap(mState);

	// Prepopulate keys and values with current values if we're validating
	if (inStream.IsValidating())
	{
		// Get and sort keys
		for (const StateMap::value_type &kv : old_state)
			keys.push_back(kv.first);
		sort(keys.begin(), keys.end());
	}

	// Ensure we have the corect size
	keys.resize(length);

	for (size_t i = 0; i < length; ++i)	
	{
		// Read key
		SubShapeIDPair key;
		if (inStream.IsValidating())
			key = keys[i];
		inStream.Read(key);

		// Read value length
		ContactPoints::size_type num_contacts;
		if (inStream.IsValidating())
			num_contacts = old_state[key].size();
		inStream.Read(num_contacts);

		// Read values
		ContactPoints contacts;
		if (inStream.IsValidating())
			contacts = old_state[key];
		contacts.resize(num_contacts);
		inStream.ReadBytes(contacts.data(), num_contacts * sizeof(Vec3));

		// Store the new value
		mState[key] = contacts;
	}
}

void ContactListenerImpl::DrawState()
{
	Trace("Draw Contact State");

	lock_guard lock(mStateMutex);
	for (const StateMap::value_type &kv : mState)
		for (Vec3 v : kv.second)
			DebugRenderer::sInstance->DrawWireSphere(v, 0.05f, Color::sRed, 1);
}

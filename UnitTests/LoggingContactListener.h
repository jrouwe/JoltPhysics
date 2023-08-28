// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ContactListener.h>

// Contact listener that just logs the calls made to it for later validation
class LoggingContactListener : public ContactListener
{
public:
	// Contact callback type
	enum class EType
	{
		Validate,
		Add,
		Persist,
		Remove
	};

	// Entry written when a contact callback happens
	struct LogEntry
	{
		EType						mType;
		BodyID						mBody1;
		BodyID						mBody2;
		ContactManifold				mManifold;
	};

	virtual ValidateResult			OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
	{
		// Check contract that body 1 is dynamic or that body2 is not dynamic
		bool contract = inBody1.IsDynamic() || !inBody2.IsDynamic();
		CHECK(contract);

		lock_guard lock(mLogMutex);
		mLog.push_back({ EType::Validate, inBody1.GetID(), inBody2.GetID(), ContactManifold() });
		return ValidateResult::AcceptContact;
	}

	virtual void					OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		// Check contract that body 1 < body 2
		CHECK(inBody1.GetID() < inBody2.GetID());

		lock_guard lock(mLogMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		CHECK(mExistingContacts.insert(key).second); // Validate that contact does not exist yet
		mLog.push_back({ EType::Add, inBody1.GetID(), inBody2.GetID(), inManifold });
	}

	virtual void					OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		// Check contract that body 1 < body 2
		CHECK(inBody1.GetID() < inBody2.GetID());

		lock_guard lock(mLogMutex);
		SubShapeIDPair key(inBody1.GetID(), inManifold.mSubShapeID1, inBody2.GetID(), inManifold.mSubShapeID2);
		CHECK(mExistingContacts.find(key) != mExistingContacts.end()); // Validate that OnContactAdded was called
		mLog.push_back({ EType::Persist, inBody1.GetID(), inBody2.GetID(), inManifold });
	}

	virtual void					OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
	{
		// Check contract that body 1 < body 2
		CHECK(inSubShapePair.GetBody1ID() < inSubShapePair.GetBody2ID());

		lock_guard lock(mLogMutex);
		CHECK(mExistingContacts.erase(inSubShapePair) == 1); // Validate that OnContactAdded was called
		ContactManifold manifold;
		manifold.mSubShapeID1 = inSubShapePair.GetSubShapeID1();
		manifold.mSubShapeID2 = inSubShapePair.GetSubShapeID2();
		mLog.push_back({ EType::Remove, inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID(), manifold });
	}

	void							Clear()
	{
		mLog.clear();
	}

	size_t							GetEntryCount() const
	{
		return mLog.size();
	}

	const LogEntry &				GetEntry(size_t inIdx) const
	{
		return mLog[inIdx];
	}

	// Find first event with a particular type and involving two particular bodies
	int								Find(EType inType, const BodyID &inBody1, const BodyID &inBody2) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && ((e.mBody1 == inBody1 && e.mBody2 == inBody2) || (e.mBody1 == inBody2 && e.mBody2 == inBody1)))
				return int(i);
		}

		return -1;
	}

	// Check if event with a particular type and involving two particular bodies exists
	bool							Contains(EType inType, const BodyID &inBody1, const BodyID &inBody2) const
	{
		return Find(inType, inBody1, inBody2) >= 0;
	}

private:
	Mutex							mLogMutex; // Callbacks are made from a thread, make sure we don't corrupt the log
	Array<LogEntry>					mLog;
	UnorderedSet<SubShapeIDPair>	mExistingContacts; // For validation purposes: the contacts that are currently active
};

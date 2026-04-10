// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/SoftBody/SoftBodyContactListener.h>
#include <Jolt/Physics/SoftBody/SoftBodyManifold.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/UnorderedSet.h>

// Soft body contact listener that just logs the calls made to it for later validation
class LoggingSoftBodyContactListener : public SoftBodyContactListener
{
public:
	// Contact callback type
	enum class EType
	{
		Validate,
		Add,
	};

	// Entry written when a contact callback happens
	struct LogEntry
	{
		EType						mType;
		BodyID						mSoftBodyID;			///< The soft body involved in the contact
		BodyID						mOtherBodyID;			///< The rigid body involved in the contact
	};

	// Value to return from the OnSoftBodyContactValidate callback
	void							SetValidateValueToReturn(SoftBodyValidateResult inValue)
	{
		mValidateValueToReturn = inValue;
	}

	virtual SoftBodyValidateResult	OnSoftBodyContactValidate(const Body &inSoftBody, const Body &inOtherBody, SoftBodyContactSettings &ioSettings) override
	{
		lock_guard lock(mLogMutex);
		mLog.push_back({ EType::Validate, inSoftBody.GetID(), inOtherBody.GetID() });
		return mValidateValueToReturn;
	}

	virtual void					OnSoftBodyContactAdded(const Body &inSoftBody, const SoftBodyManifold &inManifold) override
	{
		// Count the number of vertices in contact per body
		UnorderedSet<BodyID> bodies_in_contact;
		for (const SoftBodyVertex &v : inManifold.GetVertices())
			if (inManifold.HasContact(v))
				bodies_in_contact.insert(inManifold.GetContactBodyID(v));

		lock_guard lock(mLogMutex);
		for (const BodyID &entry : bodies_in_contact)
			mLog.push_back({ EType::Add, inSoftBody.GetID(), entry });
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

	// Find the first event of a given type involving the specified soft body and other body
	int								Find(EType inType, const BodyID &inSoftBodyID, const BodyID &inOtherBodyID) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && e.mSoftBodyID == inSoftBodyID && e.mOtherBodyID == inOtherBodyID)
				return int(i);
		}
		return -1;
	}

	// Check if an event of a given type involving the specified soft body and other body exists
	bool							Contains(EType inType, const BodyID &inSoftBodyID, const BodyID &inOtherBodyID) const
	{
		return Find(inType, inSoftBodyID, inOtherBodyID) >= 0;
	}

private:
	Mutex							mLogMutex; // Callbacks may be made from multiple threads, protect the log
	Array<LogEntry>					mLog;
	SoftBodyValidateResult			mValidateValueToReturn = SoftBodyValidateResult::AcceptContact;
};

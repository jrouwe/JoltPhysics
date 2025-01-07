// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Character/CharacterVirtual.h>

// Character contact listener that just logs the calls made to it for later validation
class LoggingCharacterContactListener : public CharacterContactListener
{
public:
	// Contact callback type
	enum class EType
	{
		ValidateBody,
		ValidateCharacter,
		AddBody,
		PersistBody,
		RemoveBody,
		AddCharacter,
		PersistCharacter,
		RemoveCharacter
	};

	// Entry written when a contact callback happens
	struct LogEntry
	{
		EType						mType;
		const CharacterVirtual *	mCharacter;
		BodyID						mBody2;
		CharacterID					mCharacterID2;
		SubShapeID					mSubShapeID2;
	};

	virtual bool					OnContactValidate(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2) override
	{
		mLog.push_back({ EType::ValidateBody, inCharacter, inBodyID2, CharacterID(), inSubShapeID2 });
		return true;
	}

	virtual bool					OnCharacterContactValidate(const CharacterVirtual *inCharacter, const CharacterVirtual *inOtherCharacter, const SubShapeID &inSubShapeID2) override
	{
		mLog.push_back({ EType::ValidateCharacter, inCharacter, BodyID(), inOtherCharacter->GetID(), inSubShapeID2 });
		return true;
	}

	virtual void					OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		mLog.push_back({ EType::AddBody, inCharacter, inBodyID2, CharacterID(), inSubShapeID2 });
	}

	virtual void					OnContactPersisted(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		mLog.push_back({ EType::PersistBody, inCharacter, inBodyID2, CharacterID(), inSubShapeID2 });
	}

	virtual void					OnContactRemoved(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2) override
	{
		mLog.push_back({ EType::RemoveBody, inCharacter, inBodyID2, CharacterID(), inSubShapeID2 });
	}

	virtual void					OnCharacterContactAdded(const CharacterVirtual *inCharacter, const CharacterVirtual *inOtherCharacter, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		mLog.push_back({ EType::AddCharacter, inCharacter, BodyID(), inOtherCharacter->GetID(), inSubShapeID2 });
	}

	virtual void					OnCharacterContactPersisted(const CharacterVirtual *inCharacter, const CharacterVirtual *inOtherCharacter, const SubShapeID &inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) override
	{
		mLog.push_back({ EType::PersistCharacter, inCharacter, BodyID(), inOtherCharacter->GetID(), inSubShapeID2 });
	}

	virtual void					OnCharacterContactRemoved(const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID, const SubShapeID &inSubShapeID2) override
	{
		mLog.push_back({ EType::RemoveCharacter, inCharacter, BodyID(), inOtherCharacterID, inSubShapeID2 });
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

	// Find first event with a particular type and involving a particular character vs body
	int								Find(EType inType, const CharacterVirtual *inCharacter, const BodyID &inBody2) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && e.mCharacter == inCharacter && e.mBody2 == inBody2 && e.mCharacterID2.IsInvalid())
				return int(i);
		}

		return -1;
	}

	// Check if event with a particular type and involving a particular character vs body
	bool							Contains(EType inType, const CharacterVirtual *inCharacter, const BodyID &inBody2) const
	{
		return Find(inType, inCharacter, inBody2) >= 0;
	}

	// Find first event with a particular type and involving a particular character vs character
	int								Find(EType inType, const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && e.mCharacter == inCharacter && e.mBody2.IsInvalid() && e.mCharacterID2 == inOtherCharacterID)
				return int(i);
		}

		return -1;
	}

	// Check if event with a particular type and involving a particular character vs character
	bool							Contains(EType inType, const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID) const
	{
		return Find(inType, inCharacter, inOtherCharacterID) >= 0;
	}

	// Find first event with a particular type and involving a particular character vs body and sub shape ID
	int								Find(EType inType, const CharacterVirtual *inCharacter, const BodyID &inBody2, const SubShapeID &inSubShapeID2) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && e.mCharacter == inCharacter && e.mBody2 == inBody2 && e.mCharacterID2.IsInvalid() && e.mSubShapeID2 == inSubShapeID2)
				return int(i);
		}

		return -1;
	}

	// Check if a particular type and involving a particular character vs body and sub shape ID exists
	bool							Contains(EType inType, const CharacterVirtual *inCharacter, const BodyID &inBody2, const SubShapeID &inSubShapeID2) const
	{
		return Find(inType, inCharacter, inBody2, inSubShapeID2) >= 0;
	}

	// Find first event with a particular type and involving a particular character vs character and sub shape ID
	int								Find(EType inType, const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID, const SubShapeID &inSubShapeID2) const
	{
		for (size_t i = 0; i < mLog.size(); ++i)
		{
			const LogEntry &e = mLog[i];
			if (e.mType == inType && e.mCharacter == inCharacter && e.mBody2.IsInvalid() && e.mCharacterID2 == inOtherCharacterID && e.mSubShapeID2 == inSubShapeID2)
				return int(i);
		}

		return -1;
	}

	// Check if a particular type and involving a particular character vs character and sub shape ID exists
	bool							Contains(EType inType, const CharacterVirtual *inCharacter, const CharacterID &inOtherCharacterID, const SubShapeID &inSubShapeID2) const
	{
		return Find(inType, inCharacter, inOtherCharacterID, inSubShapeID2) >= 0;
	}

private:
	Array<LogEntry>					mLog;
};

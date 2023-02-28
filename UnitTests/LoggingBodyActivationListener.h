// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT


#pragma once

#include <Jolt/Physics/Body/BodyActivationListener.h>

/// Activation listener that just logs the activations/deactivations
class LoggingBodyActivationListener : public BodyActivationListener
{
public:
	// Activation callback type
	enum class EType
	{
		Activated,
		Deactivated,
	};

	// Entry written when an activation callback happens
	struct LogEntry
	{
		EType				mType;
		BodyID				mBodyID;
	};

	virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		lock_guard lock(mLogMutex);
		mLog.push_back({ EType::Activated, inBodyID });
	}

	virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		lock_guard lock(mLogMutex);
		mLog.push_back({ EType::Deactivated, inBodyID });
	}

	void				Clear()
	{
		mLog.clear();
	}

	size_t				GetEntryCount() const
	{
		return mLog.size();
	}

	// Check if we have logged an event with a particular type and involving a particular body
	bool				Contains(EType inType, const BodyID &inBodyID) const
	{
		for (const LogEntry &e : mLog)
			if (e.mType == inType && e.mBodyID == inBodyID)
				return true;

		return false;
	}

private:
	Mutex				mLogMutex; // Callbacks are made from a thread, make sure we don't corrupt the log
	Array<LogEntry>		mLog;
};

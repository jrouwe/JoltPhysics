// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/NonCopyable.h>
#include <Physics/Body/BodyID.h>

namespace JPH {

/// Class function to filter out bodies, returns true if test should collide with body
class BodyFilter : public NonCopyable
{
public:
	/// Destructor
	virtual					~BodyFilter() { }

	/// Filter function. Returns true if we should collide with inBodyID
	virtual bool			ShouldCollide(const BodyID &inBodyID) const
	{
		return true;
	}
};

/// A simple body filter implementation that ignores a single, specified body
class IgnoreSingleBodyFilter : public BodyFilter
{
public:
	/// Constructor, pass the body you want to ignore
							IgnoreSingleBodyFilter(const BodyID &inBodyID) : 
		mBodyID(inBodyID)
	{
	}

	/// Filter function. Returns true if we should collide with inBodyID
	virtual bool			ShouldCollide(const BodyID &inBodyID) const override
	{
		return mBodyID != inBodyID;
	}
		
private:
	BodyID					mBodyID;
};

/// A simple body filter implementation that ignores multiple, specified bodies
class IgnoreMultipleBodiesFilter : public BodyFilter
{
public:
	/// Remove all bodies from the filter
	void					Clear()
	{
		mBodyIDs.clear();
	}

	/// Reserve space for inSize body ID's
	void					Reserve(uint inSize)
	{
		mBodyIDs.reserve(inSize);
	}

	/// Add a body to be ignored
	void					IgnoreBody(const BodyID &inBodyID)
	{
		mBodyIDs.push_back(inBodyID);
	}
		
	/// Filter function. Returns true if we should collide with inBodyID
	virtual bool			ShouldCollide(const BodyID &inBodyID) const override
	{
		return find(mBodyIDs.begin(), mBodyIDs.end(), inBodyID) == mBodyIDs.end();
	}

private:
	vector<BodyID>			mBodyIDs;
};

} // JPH
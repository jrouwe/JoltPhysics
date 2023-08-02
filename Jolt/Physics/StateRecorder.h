// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

class Body;
class Constraint;
class BodyID;

/// User callbacks that allow determining which parts of the simulation should be saved by a StateRecorder
class JPH_EXPORT StateRecorderFilter
{
public:
	/// Destructor
	virtual						~StateRecorderFilter() = default;

	/// If the state of previous delta time should be saved
	virtual bool				ShouldSavePreviousDeltaTime() const							{ return true; }

	/// If the state of gravity should be saved
	virtual bool				ShouldSaveGravity() const									{ return true; }

	/// If the state of any body should be saved
	virtual bool				ShouldSaveBodies() const									{ return true; }

	/// If the state of a specific body should be saved
	virtual bool				ShouldSaveBody(const Body &inBody) const					{ return true; }

	/// If any constraints should be saved
	virtual bool				ShouldSaveConstraints() const								{ return true; }

	/// If the state of a specific constraint should be saved
	virtual bool				ShouldSaveConstraint(const Constraint &inConstraint) const	{ return true; }

	/// If any contacts should be saved
	virtual bool				ShouldSaveContacts() const									{ return true; }

	/// If the state of a specific contact should be saved
	virtual bool				ShouldSaveContact(const BodyID &inBody1, const BodyID &inBody2) const { return true; }
};

/// Class that records the state of a physics system. Can be used to check if the simulation is deterministic by putting the recorder in validation mode.
/// Can be used to restore the state to an earlier point in time.
class JPH_EXPORT StateRecorder : public StreamIn, public StreamOut
{
public:
	/// Constructor
								StateRecorder() = default;
								StateRecorder(const StateRecorder &inRHS)					: mIsValidating(inRHS.mIsValidating) { }

	/// Sets the stream in validation mode. In this case the physics system ensures that before it calls ReadBytes that it will
	/// ensure that those bytes contain the current state. This makes it possible to step and save the state, restore to the previous
	/// step and step again and when the recorded state is not the same it can restore the expected state and any byte that changes
	/// due to a ReadBytes function can be caught to find out which part of the simulation is not deterministic
	void						SetValidating(bool inValidating)							{ JPH_ASSERT(mFilter == nullptr || !inValidating); mIsValidating = inValidating; }
	bool						IsValidating() const										{ return mIsValidating; }

	/// Sets the filter that determines which parts of the simulation should be saved. The filter is ignored while restoring the state and it can also not be used in validation mode.
	void						SetFilter(const StateRecorderFilter *inFilter)				{ JPH_ASSERT(inFilter == nullptr || !mIsValidating); mFilter = inFilter; }
	const StateRecorderFilter *	GetFilter() const											{ return mFilter; }

private:
	bool						mIsValidating = false;
	const StateRecorderFilter *	mFilter = nullptr;
};

JPH_NAMESPACE_END

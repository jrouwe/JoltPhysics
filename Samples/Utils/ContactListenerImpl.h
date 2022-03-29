// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/Core/Mutex.h>
#include <unordered_map>

// Tests the contact listener callbacks
class ContactListenerImpl : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult) override;
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override;

	// Saving / restoring state for replay
	void					SaveState(StateRecorder &inStream) const;
	void					RestoreState(StateRecorder &inStream);

	// Draw the current contact state
	void					DrawState();

	// Ability to defer to the next contact listener after this one handles the callback
	void					SetNextListener(ContactListener *inListener)				{ mNext = inListener; }

private:
	// Map that keeps track of the current state of contacts based on the contact listener callbacks
	using StateMap = unordered_map<SubShapeIDPair, ContactPoints>;
	Mutex					mStateMutex;
	StateMap				mState;

	ContactListener *		mNext = nullptr;
};

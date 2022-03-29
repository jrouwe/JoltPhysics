// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;

/// Contains the creation settings of a set of bodies
class PhysicsScene : public RefTarget<PhysicsScene>
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(PhysicsScene)

	/// Add a body to the scene
	void									AddBody(const BodyCreationSettings &inBody);

	/// Get amount of bodies in this scene
	size_t									GetNumBodies() const							{ return mBodies.size(); }

	/// Access to the body settings for this scene
	const vector<BodyCreationSettings> &	GetBodies() const								{ return mBodies; }
	vector<BodyCreationSettings> &			GetBodies()										{ return mBodies; }

	/// Instantiate all bodies, returns false if not all bodies could be created
	bool									CreateBodies(PhysicsSystem *inSystem) const;

	/// Go through all body creation settings and fix shapes that are scaled incorrectly (note this will change the scene a bit).
	/// @return False when not all scales could be fixed.
	bool									FixInvalidScales();

	/// Saves the state of this object in binary form to inStream.
	/// @param inStream The stream to save the state to
	/// @param inSaveShapes If the shapes should be saved as well (these could be shared between physics scenes, in which case the calling application may want to write custom code to restore them)
	/// @param inSaveGroupFilter If the group filter should be saved as well (these could be shared)
	void									SaveBinaryState(StreamOut &inStream, bool inSaveShapes, bool inSaveGroupFilter) const;

	using PhysicsSceneResult = Result<Ref<PhysicsScene>>;

	/// Restore a saved scene from inStream
	static PhysicsSceneResult				sRestoreFromBinaryState(StreamIn &inStream);

private:
	/// The bodies that are part of this scene
	vector<BodyCreationSettings>			mBodies;
};

JPH_NAMESPACE_END

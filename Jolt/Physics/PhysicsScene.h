// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;

/// Contains the creation settings of a set of bodies
class PhysicsScene : public RefTarget<PhysicsScene>
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(PhysicsScene)

	/// Add a body to the scene
	void									AddBody(const BodyCreationSettings &inBody);

	/// Body constant to use to indicate that the constraint is attached to the fixed world
	static constexpr uint32					cFixedToWorld = 0xffffffff;

	/// Add a constraint to the scene
	/// @param inConstraint Constraint settings
	/// @param inBody1 Index in the bodies list of first body to attach constraint to
	/// @param inBody2 Index in the bodies list of the second body to attach constraint to
	void									AddConstraint(const TwoBodyConstraintSettings *inConstraint, uint32 inBody1, uint32 inBody2);

	/// Get number of bodies in this scene
	size_t									GetNumBodies() const							{ return mBodies.size(); }

	/// Access to the body settings for this scene
	const vector<BodyCreationSettings> &	GetBodies() const								{ return mBodies; }
	vector<BodyCreationSettings> &			GetBodies()										{ return mBodies; }

	/// A constraint and how it is connected to the bodies in the scene
	class ConnectedConstraint
	{
	public:
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(ConnectedConstraint)

											ConnectedConstraint() = default;
											ConnectedConstraint(const TwoBodyConstraintSettings *inSettings, uint inBody1, uint inBody2) : mSettings(inSettings), mBody1(inBody1), mBody2(inBody2) { }

		RefConst<TwoBodyConstraintSettings>	mSettings;										///< Constraint settings
		uint32								mBody1;											///< Index of first body (in mBodies)
		uint32								mBody2;											///< Index of second body (in mBodies)
	};

	/// Get number of constraints in this scene
	size_t									GetNumConstraints() const						{ return mConstraints.size(); }

	/// Access to the constraints for this scene
	const vector<ConnectedConstraint> &		GetConstraints() const							{ return mConstraints; }
	vector<ConnectedConstraint> &			GetConstraints()								{ return mConstraints; }

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

	/// For debugging purposes: Construct a scene from the current state of the physics system
	void									FromPhysicsSystem(const PhysicsSystem *inSystem);

private:
	/// The bodies that are part of this scene
	vector<BodyCreationSettings>			mBodies;

	/// Constraints that are part of this scene
	vector<ConnectedConstraint>				mConstraints;
};

JPH_NAMESPACE_END

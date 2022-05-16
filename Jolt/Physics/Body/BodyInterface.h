// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Core/Reference.h>

JPH_NAMESPACE_BEGIN

class Body;
class BodyCreationSettings;
class BodyLockInterface;
class BroadPhase;
class BodyManager;
class TransformedShape;
class PhysicsMaterial;
class SubShapeID;
class Shape;
class TwoBodyConstraintSettings;
class TwoBodyConstraint;

/// Class that provides operations on bodies using a body ID. Note that if you need to do multiple operations on a single body, it is more efficient to lock the body once and combine the operations.
/// All quantities are in world space unless otherwise specified.
class BodyInterface : public NonCopyable
{
public:
	/// Initialize the interface (should only be called by PhysicsSystem)
	void						Init(BodyLockInterface &inBodyLockInterface, BodyManager &inBodyManager, BroadPhase &inBroadPhase) { mBodyLockInterface = &inBodyLockInterface; mBodyManager = &inBodyManager; mBroadPhase = &inBroadPhase; }
	
	/// Create a body
	/// @return Created body or null when out of bodies
	Body *						CreateBody(const BodyCreationSettings &inSettings);
	
	/// Destroy a body
	void						DestroyBody(const BodyID &inBodyID);
	
	/// Destroy multiple bodies
	void						DestroyBodies(const BodyID *inBodyIDs, int inNumber);

	/// Add body to the world.
	/// Note that if you need to add multiple bodies, use the AddBodiesPrepare/AddBodiesFinalize function.
	/// Adding many bodies, one at a time, results in a really inefficient broadphase until PhysicsSystem::OptimizeBroadPhase is called or when PhysicsSystem::Update rebuilds the tree!
	/// After adding, to get a body by ID use the BodyLockRead or BodyLockWrite interface!
	void						AddBody(const BodyID &inBodyID, EActivation inActivationMode);
	
	/// Remove body from the world.
	void						RemoveBody(const BodyID &inBodyID);
	
	/// Check if a body has been added to the world.
	bool						IsAdded(const BodyID &inBodyID) const;

	/// Combines CreateBody and AddBody
	/// @return Created body ID or an invalid ID when out of bodies
	BodyID						CreateAndAddBody(const BodyCreationSettings &inSettings, EActivation inActivationMode);

	/// Broadphase add state handle, used to keep track of a batch while ading to the broadphase.
	using AddState = void *;

	///@name Batch adding interface, see Broadphase for further documentation.
	/// Note that ioBodies array must be kept constant while the add is in progress.
	///@{
	AddState					AddBodiesPrepare(BodyID *ioBodies, int inNumber);
	void						AddBodiesFinalize(BodyID *ioBodies, int inNumber, AddState inAddState, EActivation inActivationMode);
	void						AddBodiesAbort(BodyID *ioBodies, int inNumber, AddState inAddState);
	void						RemoveBodies(BodyID *ioBodies, int inNumber);
	///@}

	///@name Activate / deactivate a body
	///@{
	void						ActivateBody(const BodyID &inBodyID);
	void						ActivateBodies(const BodyID *inBodyIDs, int inNumber);
	void						DeactivateBody(const BodyID &inBodyID);
	void						DeactivateBodies(const BodyID *inBodyIDs, int inNumber);
	bool						IsActive(const BodyID &inBodyID) const;
	///@}

	/// Create a two body constraint
	TwoBodyConstraint *			CreateConstraint(const TwoBodyConstraintSettings *inSettings, const BodyID &inBodyID1, const BodyID &inBodyID2);

	/// Activate non-static bodies attached to a constraint
	void						ActivateConstraint(const TwoBodyConstraint *inConstraint);

	///@name Access to the shape of a body
	///@{

	/// Get the current shape
	RefConst<Shape>				GetShape(const BodyID &inBodyID) const;

	/// Set a new shape on the body
	/// @param inBodyID Body ID of body that had its shape changed
	/// @param inShape The new shape
	/// @param inUpdateMassProperties When true, the mass and inertia tensor is recalculated
	/// @param inActivationMode Weather or not to activate the body
	void						SetShape(const BodyID &inBodyID, const Shape *inShape, bool inUpdateMassProperties, EActivation inActivationMode) const;

	/// Notify all systems to indicate that a shape has changed (usable for MutableCompoundShapes)
	/// @param inBodyID Body ID of body that had its shape changed
	/// @param inPreviousCenterOfMass Center of mass of the shape before the alterations
	/// @param inUpdateMassProperties When true, the mass and inertia tensor is recalculated
	/// @param inActivationMode Weather or not to activate the body
	void						NotifyShapeChanged(const BodyID &inBodyID, Vec3Arg inPreviousCenterOfMass, bool inUpdateMassProperties, EActivation inActivationMode) const;
	///@}

	///@name Object layer of a body
	///@{
	void						SetObjectLayer(const BodyID &inBodyID, ObjectLayer inLayer);
	ObjectLayer					GetObjectLayer(const BodyID &inBodyID) const;
	///@}

	///@name Position and rotation of a body
	///@{
	void						SetPositionAndRotation(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, EActivation inActivationMode);
	void						SetPositionAndRotationWhenChanged(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, EActivation inActivationMode); ///< Will only update the position/rotation and activate the body when the difference is larger than a very small number. This avoids updating the broadphase/waking up a body when the resulting position/orientation doesn't really change.
	void						GetPositionAndRotation(const BodyID &inBodyID, Vec3 &outPosition, Quat &outRotation) const;
	void						SetPosition(const BodyID &inBodyID, Vec3Arg inPosition, EActivation inActivationMode);
	Vec3						GetPosition(const BodyID &inBodyID) const;
	Vec3						GetCenterOfMassPosition(const BodyID &inBodyID) const;
	void						SetRotation(const BodyID &inBodyID, QuatArg inRotation, EActivation inActivationMode);
	Quat						GetRotation(const BodyID &inBodyID) const;
	Mat44						GetWorldTransform(const BodyID &inBodyID) const;
	Mat44						GetCenterOfMassTransform(const BodyID &inBodyID) const;
	///@}

	/// Set velocity of body such that it will be positioned at inTargetPosition/Rotation in inDeltaTime seconds (will activate body if needed)
	void						MoveKinematic(const BodyID &inBodyID, Vec3Arg inTargetPosition, QuatArg inTargetRotation, float inDeltaTime);

	/// Linear or angular velocity (functions will activate body if needed).
	/// Note that the linear velocity is the velocity of the center of mass, which may not coincide with the position of your object, to correct for this: \f$VelocityCOM = Velocity - AngularVelocity \times ShapeCOM\f$
	void						SetLinearAndAngularVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity);
	void						GetLinearAndAngularVelocity(const BodyID &inBodyID, Vec3 &outLinearVelocity, Vec3 &outAngularVelocity) const;
	void						SetLinearVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity);
	Vec3						GetLinearVelocity(const BodyID &inBodyID) const;
	void						AddLinearVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity); ///< Add velocity to current velocity
	void						AddLinearAndAngularVelocity(const BodyID &inBodyID, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity); ///< Add linear and angular to current velocities
	void						SetAngularVelocity(const BodyID &inBodyID, Vec3Arg inAngularVelocity);
	Vec3						GetAngularVelocity(const BodyID &inBodyID) const;
	Vec3						GetPointVelocity(const BodyID &inBodyID, Vec3Arg inPoint) const; ///< Velocity of point inPoint (in world space, e.g. on the surface of the body) of the body

	/// Set the complete motion state of a body.
	/// Note that the linear velocity is the velocity of the center of mass, which may not coincide with the position of your object, to correct for this: \f$VelocityCOM = Velocity - AngularVelocity \times ShapeCOM\f$
	void						SetPositionRotationAndVelocity(const BodyID &inBodyID, Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity);

	///@name Add forces to the body
	///@{
	void						AddForce(const BodyID &inBodyID, Vec3Arg inForce); ///< See Body::AddForce
	void						AddForce(const BodyID &inBodyID, Vec3Arg inForce, Vec3Arg inPoint); ///< Applied at inPoint
	void						AddTorque(const BodyID &inBodyID, Vec3Arg inTorque); ///< See Body::AddTorque
	void						AddForceAndTorque(const BodyID &inBodyID, Vec3Arg inForce, Vec3Arg inTorque); ///< A combination of Body::AddForce and Body::AddTorque
	///@}

	///@name Add an impulse to the body
	///@{
	void						AddImpulse(const BodyID &inBodyID, Vec3Arg inImpulse); ///< Applied at center of mass
	void						AddImpulse(const BodyID &inBodyID, Vec3Arg inImpulse, Vec3Arg inPoint); ///< Applied at inPoint
	void						AddAngularImpulse(const BodyID &inBodyID, Vec3Arg inAngularImpulse);
	///@}

	/// Update the body motion type
	void						SetMotionType(const BodyID &inBodyID, EMotionType inMotionType, EActivation inActivationMode);

	/// Get inverse inertia tensor in world space
	Mat44						GetInverseInertia(const BodyID &inBodyID) const;

	///@name Restitution
	///@{
	void						SetRestitution(const BodyID &inBodyID, float inRestitution);
	float						GetRestitution(const BodyID &inBodyID) const;
	///@}

	///@name Friction
	///@{
	void						SetFriction(const BodyID &inBodyID, float inFriction);
	float						GetFriction(const BodyID &inBodyID) const;
	///@}

	///@name Gravity factor
	///@{
	void						SetGravityFactor(const BodyID &inBodyID, float inGravityFactor);
	float						GetGravityFactor(const BodyID &inBodyID) const;
	///@}

	/// Get transform and shape for this body, used to perform collision detection
	TransformedShape			GetTransformedShape(const BodyID &inBodyID) const;

	/// Get the user data for a body
	uint64						GetUserData(const BodyID &inBodyID) const;

	/// Get the material for a particular sub shape
	const PhysicsMaterial *		GetMaterial(const BodyID &inBodyID, const SubShapeID &inSubShapeID) const;

	/// Set the Body::EFlags::InvalidateContactCache flag for the specified body. This means that the collision cache is invalid for any body pair involving that body until the next physics step.
	void						InvalidateContactCache(const BodyID &inBodyID);

private:
	BodyLockInterface *			mBodyLockInterface = nullptr;
	BodyManager *				mBodyManager = nullptr;
	BroadPhase *				mBroadPhase = nullptr;
};

JPH_NAMESPACE_END

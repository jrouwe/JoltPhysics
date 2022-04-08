// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

JPH_NAMESPACE_BEGIN

class CharacterVirtual;
class PhysicsSystem;

/// Contains the configuration of a character
class CharacterVirtualSettings : public RefTarget<CharacterVirtualSettings>
{
public:
	/// Vector indicating the up direction of the character
	Vec3								mUp = Vec3::sAxisY();

	/// Character mass (kg). Used to push down objects with gravity when the character is standing on top.
	float								mMass = 70.0f;

	/// Maximum force with which the character can push other bodies (N).
	float								mMaxStrength = 100.0f;

	/// Maximum angle of slope that character can still walk on (radians).
	float								mMaxSlopeAngle = DegreesToRadians(50.0f);

	/// This value governs how fast a penetration will be resolved, 0 = nothing is resolved, 1 = everything in one update
	float								mPenetrationRecoverySpeed = 1.0f;

	/// Initial shape that represents the character's volume.
	/// Usually this is a capsule, make sure the shape is made so that the bottom of the shape is at (0, 0, 0).
	RefConst<Shape>						mShape;
};

/// This class contains settings that allow you to override the behavior of a character's collision response
class CharacterContactSettings
{
public:
	bool								mCanPushCharacter = true;								///< True when the object can push the virtual character
	bool								mCanReceiveImpulses = true;								///< True when the virtual character can apply impulses (push) the body
};

/// This class receives callbacks when a virtual character hits something.
class CharacterContactListener
{
public:
	/// Destructor
	virtual								~CharacterContactListener() = default;

	/// Checks if a character can collide with specified body. Return true if the contact is valid.
	virtual bool						OnContactValidate(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2) { return true; }

	/// Called whenever the character collides with a body. Returns true if the contact can push the character.
	virtual void						OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) { }
};

/// Runtime character object.
/// This object usually represents the player. Contrary to the Character class it doesn't use a rigid body but moves doing collision checks only (hence the name virtual).
/// The advantage of this is that you can determine when the character moves in the frame (usually this has to happen at a very particular point in the frame)
/// but the downside is that other objects don't see this virtual character. In order to make this work it is recommended to pair a CharacterVirtual with a Character that
/// moves along. This Character should be keyframed (or at least have no gravity) and move along with the CharacterVirtual so that other rigid bodies can collide with it.
class CharacterVirtual : public RefTarget<CharacterVirtual>
{
public:
	/// Constructor
	/// @param inSettings The settings for the character
	/// @param inPosition Initial position for the character
	/// @param inRotation Initial rotation for the character (usually only around the up-axis)
	/// @param inSystem Physics system that this character will be added to later
										CharacterVirtual(CharacterVirtualSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, PhysicsSystem *inSystem);

	/// Set the contact listener
	void								SetListener(CharacterContactListener *inListener)		{ mListener = inListener; }

	/// Get the current contact listener
	CharacterContactListener *			GetListener() const										{ return mListener; }

	/// Get the linear velocity of the character (m / s)
	Vec3								GetLinearVelocity() const								{ return mLinearVelocity; }

	/// Set the linear velocity of the character (m / s)
	void								SetLinearVelocity(Vec3Arg inLinearVelocity)				{ mLinearVelocity = inLinearVelocity; }

	/// Get the position of the character
	Vec3								GetPosition() const										{ return mPosition; }

	/// Set the position of the character
	void								SetPosition(Vec3Arg inPosition)							{ mPosition = inPosition; }

	/// Get the rotation of the character
	Quat								GetRotation() const										{ return mRotation; }
	
	/// Set the rotation of the character
	void								SetRotation(QuatArg inRotation)							{ mRotation = inRotation; }

	/// Calculate the world transform of the character
	Mat44								GetWorldTransform() const								{ return Mat44::sRotationTranslation(mRotation, mPosition); }

	/// Calculates the transform for this character's center of mass
	Mat44								GetCenterOfMassTransform() const						{ Mat44 mat = Mat44::sRotation(mRotation); mat.SetTranslation(mPosition + mat.Multiply3x3(mShape->GetCenterOfMass())); return mat; }

	/// Character mass (kg)
	void								SetMass(float inMass)									{ mMass = inMass; }

	/// Maximum force with which the character can push other bodies (N)
	void								SetMaxStrength(float inMaxStrength)						{ mMaxStrength = inMaxStrength; }

	/// Set the maximum angle of slope that character can still walk on (radians)
	void								SetMaxSlopeAngle(float inMaxSlopeAngle)					{ mCosMaxSlopeAngle = cos(inMaxSlopeAngle); }

	/// This value governs how fast a penetration will be resolved, 0 = nothing is resolved, 1 = everything in one update
	void								SetPenetrationRecoverySpeed(float inSpeed)				{ mPenetrationRecoverySpeed = inSpeed; }

	/// This is the main update function. It moves the character according to its current velocity. Note it's your own responsibility to apply gravity!
	/// @param inDeltaTime Time step to simulate.
	/// @param inGravity Gravity vector (m/s^2)
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	void								Update(float inDeltaTime, Vec3Arg inGravity, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter);

	/// This function can be used after a character has teleported to determine the new contacts with the world.
	void								RefreshContacts(const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter);

	/// Switch the shape of the character (e.g. for stance).
	/// @param inMaxPenetrationDepth When inMaxPenetrationDepth is not FLT_MAX, it checks if the new shape collides before switching shape. This is the max penetration we're willing to accept after the switch.
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	/// @return Returns true if the switch succeeded.
	bool								SetShape(const Shape *inShape, float inMaxPenetrationDepth, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter);

	/// Get the current shape that the character is using.
	const Shape *						GetShape() const										{ return mShape; }

	enum class EGroundState
	{
		OnGround,						///< Character is on the ground and can move freely
		Sliding,						///< Character is on a slope that is too steep and should start sliding
		InAir,							///< Character is in the air
	};

	///@name Properties of the ground this character is standing on

	/// Current ground state
	EGroundState						GetGroundState() const									{ return mGroundState; }

	/// Get the contact point with the ground which has the most upright normal
	Vec3								GetGroundPosition() const								{ return mSupportingContact != nullptr? mSupportingContact->mPosition : Vec3::sZero(); }

	/// Get the average contact normal with the ground
	Vec3								GetGroundNormal() const									{ return mGroundNormal; }

	/// Average velocity in world space of the surface that we're standing on
	Vec3								GetGroundVelocity() const								{ return mGroundVelocity; }

	/// Material that the character is standing on.
	const PhysicsMaterial *				GetGroundMaterial() const								{ return mSupportingContact != nullptr? mSupportingContact->mMaterial : nullptr; }

	/// BodyID of the object the character is standing on. Note may have been removed!
	BodyID								GetGroundBodyID() const									{ return mSupportingContact != nullptr? mSupportingContact->mBodyB : BodyID(); }

	/// Sub part of the body that we're standing on.
	SubShapeID							GetGroundSubShapeID() const								{ return mSupportingContact != nullptr? mSupportingContact->mSubShapeIDB : SubShapeID(); }

	/// User data value of the body that we're standing on
	uint64								GetGroundUserData() const								{ return mSupportingContact != nullptr? mSupportingContact->mUserData : 0; }

private:
	// Constants
	static constexpr float				cPredictiveContactDistance = 0.1f;						///< How far to scan outside of the shape for predictive contacts
	static constexpr int				cMaxCollisionIterations = 5;							///< Max amount of collision loops
	static constexpr int				cMaxConstraintIterations = 15;							///< How often to try stepping in the constraint solving
	static constexpr float				cMinTimeRemaining = 1.0e-4f;							///< Early out condition: If this much time is left to simulate we are done
	static constexpr float				cCollisionTolerance = 1.0e-3f;							///< How far we're willing to penetrate geometry
	static constexpr float				cCharacterPadding = 0.02f;								///< How far we try to stay away from the geometry, this ensures that the sweep will hit as little as possible lowering the collision cost and reducing the risk of getting stuck
	static constexpr int				cMaxNumHits = 256;										///< Max num hits to collect in order to avoid excess of contact points collection

	// Encapsulates a collision contact
	struct Contact
	{
		Vec3							mPosition;												///< Position where the character makes contact
		Vec3							mLinearVelocity;										///< Velocity of the contact point
		Vec3							mNormal;												///< Contact normal, pointing towards the character
		float							mDistance;												///< Distance to the contact <= 0 means that it is an actual contact, > 0 means predictive
		float							mFraction;												///< Fraction along the path where this contact takes place
		BodyID							mBodyB;													///< ID of body we're colliding with
		SubShapeID						mSubShapeIDB;											///< Sub shape ID of body we're colliding with
		EMotionType						mMotionTypeB;											///< Motion type of B, used to determine the priority of the contact
		uint64							mUserData;												///< User data of B
		const PhysicsMaterial *			mMaterial;												///< Material of B
		bool							mHadCollision = false;									///< If the character actually collided with the contact (can be false if a predictive contact never becomes a real one)
		bool							mWasDiscarded = false;									///< If the contact validate callback chose to discard this contact
		bool							mCanPushCharacter = true;								///< When true, the velocity of the contact point can push the character
	};

	// A contact that needs to be ignored
	struct IgnoredContact
	{
										IgnoredContact() = default;
										IgnoredContact(const BodyID &inBodyID, const SubShapeID &inSubShapeID) : mBodyID(inBodyID), mSubShapeID(inSubShapeID) { }

		BodyID							mBodyID;												///< ID of body we're colliding with
		SubShapeID						mSubShapeID;											///< Sub shape of body we're colliding with
	};

	// A constraint that limits the movement of the character
	struct Constraint
	{
		Contact *						mContact;												///< Contact that this constraint was generated from
		float							mTOI;													///< Calculated time of impact (can be negative if penetrating)
		float							mProjectedVelocity;										///< Velocity of the contact projected on the contact normal (negative if separating)
		Vec3							mLinearVelocity;										///< Velocity of the contact (can contain a corrective velocity to resolve penetration)
		Plane							mPlane;													///< Plane around the origin that describes how far we can displace (from the origin)
	};

	// Collision collector that collects hits for CollideShape
	class ContactCollector : public CollideShapeCollector
	{
	public:
										ContactCollector(PhysicsSystem *inSystem, vector<Contact> &outContacts) : mSystem(inSystem), mContacts(outContacts) { }

		virtual void					AddHit(const CollideShapeResult &inResult) override;

		PhysicsSystem *					mSystem;
		vector<Contact> &				mContacts;
	};

	// A collision collector that collects hits for CastShape
	class ContactCastCollector : public CastShapeCollector
	{
	public:
										ContactCastCollector(PhysicsSystem *inSystem, Vec3Arg inDisplacement, const vector<IgnoredContact> &inIgnoredContacts, vector<Contact> &outContacts) : mSystem(inSystem), mDisplacement(inDisplacement), mIgnoredContacts(inIgnoredContacts), mContacts(outContacts) { }

		virtual void					AddHit(const ShapeCastResult &inResult) override;

		PhysicsSystem *					mSystem;
		Vec3							mDisplacement;
		const vector<IgnoredContact> &	mIgnoredContacts;
		vector<Contact> &				mContacts;
	};

	// Helper function to convert a Jolt collision result into a contact
	template <class taCollector>
	inline static void					sFillContactProperties(Contact &outContact, const Body &inBody, const taCollector &inCollector, const CollideShapeResult &inResult);

	// Move the shape from ioPosition and try to displace it by inVelocity * inDeltaTime, this will try to slide the shape along the world geometry
	void								MoveShape(Vec3 &ioPosition, Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, vector<Contact> *outActiveContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter);

	// Ask the callback if inContact is a valid contact point
	bool								ValidateContact(const Contact &inContact) const;

	// Tests the shape for collision around inPosition
	void								GetContactsAtPosition(Vec3Arg inPosition, Vec3Arg inMovementDirection, const Shape *inShape, vector<Contact> &outContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const;

	// Remove penetrating contacts with the same body that have conflicting normals, leaving these will make the character mover get stuck
	void								RemoveConflictingContacts(vector<Contact> &ioContacts, vector<IgnoredContact> &outIgnoredContacts) const;

	// Convert contacts into constraints. The character is assumed to start at the origin and the constraints are planes around the origin that confine the movement of the character.
	void								DetermineConstraints(Vec3Arg inCharacterVelocity, vector<Contact> &inContacts, vector<Constraint> &outConstraints) const;

	// Use the constraints to solve the displacement of the character. This will slide the character on the planes around the origin for as far as possible.
	void								SolveConstraints(Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, float inTimeRemaining, vector<Constraint> &ioConstraints, vector<IgnoredContact> &ioIgnoredContacts, float &outTimeSimulated, Vec3 &outDisplacement) const;

	// Handle contact with physics object that we're colliding against
	bool								HandleContact(Vec3Arg inVelocity, Constraint &ioConstraint, Vec3Arg inGravity, float inDeltaTime) const;

	// Does a swept test of the shape from inPosition with displacement inDisplacement, returns true if there was a collision
	bool								GetFirstContactForSweep(Vec3Arg inPosition, Vec3Arg inDisplacement, Contact &outContact, const vector<IgnoredContact> &inIgnoredContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const;

	// Store contacts so that CheckSupport and GetStandingPhysicsInstance etc. can return information
	void								StoreActiveContacts(const vector<Contact> &inContacts);

	// This function will determine which contacts are touching the character and will calculate the one that is supporting us
	void								UpdateSupportingContact();

	// This function returns the actual position of the shape, corrected for the character padding
	Vec3								GetShapePosition(Vec3Arg inPosition) const				{ return inPosition + mRotation * Vec3(0, cCharacterPadding, 0); }

	// The physics system that we belong to
	PhysicsSystem *						mSystem;

	// The shape that represents the volume of the character
	RefConst<Shape>						mShape;

	// Our main listener for contacts
	CharacterContactListener *			mListener = nullptr;

	// The character's world space up axis
	Vec3								mUp;

	// This value governs how fast a penetration will be resolved, 0 = nothing is resolved, 1 = everything in one update
	float								mPenetrationRecoverySpeed;

	/// Character mass (kg)
	float								mMass;

	/// Maximum force with which the character can push other bodies (N)
	float								mMaxStrength;

	// Cosine of mMaxSlopeRadians, avoids having to recompute it every time
	float								mCosMaxSlopeAngle;

	// Current position (of the base, not the center of mass)
	Vec3								mPosition = Vec3::sZero();

	// Current rotation (of the base, not of the center of mass)
	Quat								mRotation = Quat::sIdentity();

	// Current linear velocity
	Vec3								mLinearVelocity = Vec3::sZero();

	// List of contacts that were active in the last frame
	vector<Contact>						mActiveContacts;

	// Points into mActiveContacts to the contact that has the most upward pointing normal
	const Contact *						mSupportingContact = nullptr;

	// Remembers the delta time of the last update
	float								mLastDeltaTime = 1.0f / 60.0f;

	// If the character is on the ground
	EGroundState						mGroundState = EGroundState::InAir;

	// Average velocity of the ground
	Vec3								mGroundVelocity = Vec3::sZero();

	// Average normal of the ground
	Vec3								mGroundNormal = Vec3::sZero();
};

JPH_NAMESPACE_END

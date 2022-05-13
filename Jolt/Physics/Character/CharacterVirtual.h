// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Character/CharacterBase.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/STLTempAllocator.h>

JPH_NAMESPACE_BEGIN

class CharacterVirtual;

/// Contains the configuration of a character
class CharacterVirtualSettings : public CharacterBaseSettings
{
public:
	/// Vector indicating the up direction of the character
	Vec3								mUp = Vec3::sAxisY();

	/// Character mass (kg). Used to push down objects with gravity when the character is standing on top.
	float								mMass = 70.0f;

	/// Maximum force with which the character can push other bodies (N).
	float								mMaxStrength = 100.0f;

	///@name Movement settings
	float								mPredictiveContactDistance = 0.1f;						///< How far to scan outside of the shape for predictive contacts
	uint								mMaxCollisionIterations = 5;							///< Max amount of collision loops
	uint								mMaxConstraintIterations = 15;							///< How often to try stepping in the constraint solving
	float								mMinTimeRemaining = 1.0e-4f;							///< Early out condition: If this much time is left to simulate we are done
	float								mCollisionTolerance = 1.0e-3f;							///< How far we're willing to penetrate geometry
	float								mCharacterPadding = 0.02f;								///< How far we try to stay away from the geometry, this ensures that the sweep will hit as little as possible lowering the collision cost and reducing the risk of getting stuck
	uint								mMaxNumHits = 256;										///< Max num hits to collect in order to avoid excess of contact points collection
	float								mPenetrationRecoverySpeed = 1.0f;						///< This value governs how fast a penetration will be resolved, 0 = nothing is resolved, 1 = everything in one update
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
	virtual void						OnContactAdded(const CharacterVirtual *inCharacter, const BodyID &inBodyID2, const SubShapeID &inSubShapeID2, Vec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings &ioSettings) { /* Default do nothing */ }
};

/// Runtime character object.
/// This object usually represents the player. Contrary to the Character class it doesn't use a rigid body but moves doing collision checks only (hence the name virtual).
/// The advantage of this is that you can determine when the character moves in the frame (usually this has to happen at a very particular point in the frame)
/// but the downside is that other objects don't see this virtual character. In order to make this work it is recommended to pair a CharacterVirtual with a Character that
/// moves along. This Character should be keyframed (or at least have no gravity) and move along with the CharacterVirtual so that other rigid bodies can collide with it.
class CharacterVirtual : public CharacterBase
{
public:
	/// Constructor
	/// @param inSettings The settings for the character
	/// @param inPosition Initial position for the character
	/// @param inRotation Initial rotation for the character (usually only around the up-axis)
	/// @param inSystem Physics system that this character will be added to later
										CharacterVirtual(const CharacterVirtualSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, PhysicsSystem *inSystem);

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
	Mat44								GetCenterOfMassTransform() const						{ return GetCenterOfMassTransform(mPosition, mRotation, mShape); }

	/// Character mass (kg)
	void								SetMass(float inMass)									{ mMass = inMass; }

	/// Maximum force with which the character can push other bodies (N)
	void								SetMaxStrength(float inMaxStrength)						{ mMaxStrength = inMaxStrength; }

	/// Character padding
	float								GetCharacterPadding() const								{ return mCharacterPadding; }

	/// This is the main update function. It moves the character according to its current velocity. Note it's your own responsibility to apply gravity!
	/// @param inDeltaTime Time step to simulate.
	/// @param inGravity Gravity vector (m/s^2)
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	/// @param inAllocator An allocator for temporary allocations. All memory will be freed by the time this function returns.
	void								Update(float inDeltaTime, Vec3Arg inGravity, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator);

	/// This function will return true if the character has moved into a slope that is too steep (e.g. a vertical wall).
	/// You would call WalkStairs to attempt to step up stairs.
	bool								CanWalkStairs() const;

	/// When stair walking is needed, you can call the WalkStairs function to cast up, forward and down again to try to find a valid position
	/// @param inDeltaTime Time step to simulate.
	/// @param inGravity Gravity vector (m/s^2)
	/// @param inStepUp The direction and distance to step up (this corresponds to the max step height)
	/// @param inStepForward The direction and distance to step forward after the step up
	/// @param inStepForwardTest When running at a high frequency, inStepForward can be very small and it's likely that you hit the side of the stairs on the way down. This could produce a normal that violates the max slope angle. If this happens, we test again using this distance from the up position to see if we find a valid slope.
	/// @param inStepDownExtra An additional translation that is added when stepping down at the end. Allows you to step further down than up. Set to zero if you don't want this.
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	/// @param inAllocator An allocator for temporary allocations. All memory will be freed by the time this function returns.
	/// @return true if the stair walk was successful
	bool								WalkStairs(float inDeltaTime, Vec3Arg inGravity, Vec3Arg inStepUp, Vec3Arg inStepForward, Vec3Arg inStepForwardTest, Vec3Arg inStepDownExtra, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator);

	/// This function can be used after a character has teleported to determine the new contacts with the world.
	void								RefreshContacts(const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator);

	/// Switch the shape of the character (e.g. for stance).
	/// @param inShape The shape to switch to.
	/// @param inMaxPenetrationDepth When inMaxPenetrationDepth is not FLT_MAX, it checks if the new shape collides before switching shape. This is the max penetration we're willing to accept after the switch.
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	/// @param inAllocator An allocator for temporary allocations. All memory will be freed by the time this function returns.
	/// @return Returns true if the switch succeeded.
	bool								SetShape(const Shape *inShape, float inMaxPenetrationDepth, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator);

	/// @brief Get all contacts for the character at a particular location
	/// @param inPosition Position to test, note that this position will be corrected for the character padding.
	/// @param inRotation Rotation at which to test the shape.
	/// @param inMovementDirection A hint in which direction the character is moving, will be used to calculate a proper normal.
	/// @param inMaxSeparationDistance How much distance around the character you want to report contacts in (can be 0 to match the character exactly).
	/// @param inShape Shape to test collision with.
	/// @param ioCollector Collision collector that receives the collision results.
	/// @param inBroadPhaseLayerFilter Filter that is used to check if the character collides with something in the broadphase.
	/// @param inObjectLayerFilter Filter that is used to check if a character collides with a layer.
	/// @param inBodyFilter Filter that is used to check if a character collides with a body.
	void								CheckCollision(Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inMovementDirection, float inMaxSeparationDistance, const Shape *inShape, CollideShapeCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const;

	// Saving / restoring state for replay
	virtual void						SaveState(StateRecorder &inStream) const override;
	virtual void						RestoreState(StateRecorder &inStream) override;

#ifdef JPH_DEBUG_RENDERER
	static inline bool					sDrawConstraints = false;								///< Draw the current state of the constraints for iteration 0 when creating them
	static inline bool					sDrawWalkStairs = false;								///< Draw the state of the walk stairs algorithm
#endif

private:
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

	using TempContactList = vector<Contact, STLTempAllocator<Contact>>;
	using ContactList = vector<Contact>;

	// A contact that needs to be ignored
	struct IgnoredContact
	{
										IgnoredContact() = default;
										IgnoredContact(const BodyID &inBodyID, const SubShapeID &inSubShapeID) : mBodyID(inBodyID), mSubShapeID(inSubShapeID) { }

		BodyID							mBodyID;												///< ID of body we're colliding with
		SubShapeID						mSubShapeID;											///< Sub shape of body we're colliding with
	};

	using IgnoredContactList = vector<IgnoredContact, STLTempAllocator<IgnoredContact>>;

	// A constraint that limits the movement of the character
	struct Constraint
	{
		Contact *						mContact;												///< Contact that this constraint was generated from
		float							mTOI;													///< Calculated time of impact (can be negative if penetrating)
		float							mProjectedVelocity;										///< Velocity of the contact projected on the contact normal (negative if separating)
		Vec3							mLinearVelocity;										///< Velocity of the contact (can contain a corrective velocity to resolve penetration)
		Plane							mPlane;													///< Plane around the origin that describes how far we can displace (from the origin)
	};

	using ConstraintList = vector<Constraint, STLTempAllocator<Constraint>>;

	// Collision collector that collects hits for CollideShape
	class ContactCollector : public CollideShapeCollector
	{
	public:
										ContactCollector(PhysicsSystem *inSystem, uint inMaxHits, TempContactList &outContacts) : mSystem(inSystem), mContacts(outContacts), mMaxHits(inMaxHits) { }

		virtual void					AddHit(const CollideShapeResult &inResult) override;

		PhysicsSystem *					mSystem;
		TempContactList &				mContacts;
		uint							mMaxHits;
	};

	// A collision collector that collects hits for CastShape
	class ContactCastCollector : public CastShapeCollector
	{
	public:
										ContactCastCollector(PhysicsSystem *inSystem, Vec3Arg inDisplacement, uint inMaxHits, const IgnoredContactList &inIgnoredContacts, TempContactList &outContacts) : mSystem(inSystem), mDisplacement(inDisplacement), mIgnoredContacts(inIgnoredContacts), mContacts(outContacts), mMaxHits(inMaxHits) { }

		virtual void					AddHit(const ShapeCastResult &inResult) override;

		PhysicsSystem *					mSystem;
		Vec3							mDisplacement;
		const IgnoredContactList &		mIgnoredContacts;
		TempContactList &				mContacts;
		uint							mMaxHits;
	};

	// Helper function to convert a Jolt collision result into a contact
	template <class taCollector>
	inline static void					sFillContactProperties(Contact &outContact, const Body &inBody, const taCollector &inCollector, const CollideShapeResult &inResult);

	// Move the shape from ioPosition and try to displace it by inVelocity * inDeltaTime, this will try to slide the shape along the world geometry
	void								MoveShape(Vec3 &ioPosition, Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, ContactList *outActiveContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator) const;

	// Ask the callback if inContact is a valid contact point
	bool								ValidateContact(const Contact &inContact) const;

	// Tests the shape for collision around inPosition
	void								GetContactsAtPosition(Vec3Arg inPosition, Vec3Arg inMovementDirection, const Shape *inShape, TempContactList &outContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const;

	// Remove penetrating contacts with the same body that have conflicting normals, leaving these will make the character mover get stuck
	void								RemoveConflictingContacts(TempContactList &ioContacts, IgnoredContactList &outIgnoredContacts) const;

	// Convert contacts into constraints. The character is assumed to start at the origin and the constraints are planes around the origin that confine the movement of the character.
	void								DetermineConstraints(TempContactList &inContacts, ConstraintList &outConstraints) const;

	// Use the constraints to solve the displacement of the character. This will slide the character on the planes around the origin for as far as possible.
	void								SolveConstraints(Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, float inTimeRemaining, ConstraintList &ioConstraints, IgnoredContactList &ioIgnoredContacts, float &outTimeSimulated, Vec3 &outDisplacement, TempAllocator &inAllocator) const;

	// Handle contact with physics object that we're colliding against
	bool								HandleContact(Vec3Arg inVelocity, Constraint &ioConstraint, Vec3Arg inGravity, float inDeltaTime) const;

	// Does a swept test of the shape from inPosition with displacement inDisplacement, returns true if there was a collision
	bool								GetFirstContactForSweep(Vec3Arg inPosition, Vec3Arg inDisplacement, Contact &outContact, const IgnoredContactList &inIgnoredContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator) const;

	// Store contacts so that CheckSupport and GetStandingPhysicsInstance etc. can return information
	void								StoreActiveContacts(const TempContactList &inContacts, TempAllocator &inAllocator);

	// This function will determine which contacts are touching the character and will calculate the one that is supporting us
	void								UpdateSupportingContact(TempAllocator &inAllocator);

	// This function returns the actual center of mass of the shape, not corrected for the character padding
	inline Mat44						GetCenterOfMassTransform(Vec3Arg inPosition, QuatArg inRotation, const Shape *inShape) const
	{
		return Mat44::sRotationTranslation(inRotation, inPosition).PreTranslated(inShape->GetCenterOfMass()).PostTranslated(mCharacterPadding * mUp);
	}

	// Our main listener for contacts
	CharacterContactListener *			mListener = nullptr;

	// The character's world space up axis
	Vec3								mUp;

	// Movement settings
	float								mPredictiveContactDistance;								// How far to scan outside of the shape for predictive contacts
	uint								mMaxCollisionIterations;								// Max amount of collision loops
	uint								mMaxConstraintIterations;								// How often to try stepping in the constraint solving
	float								mMinTimeRemaining;										// Early out condition: If this much time is left to simulate we are done
	float								mCollisionTolerance;									// How far we're willing to penetrate geometry
	float								mCharacterPadding;										// How far we try to stay away from the geometry, this ensures that the sweep will hit as little as possible lowering the collision cost and reducing the risk of getting stuck
	uint								mMaxNumHits;											// Max num hits to collect in order to avoid excess of contact points collection
	float								mPenetrationRecoverySpeed;								// This value governs how fast a penetration will be resolved, 0 = nothing is resolved, 1 = everything in one update

	// Character mass (kg)
	float								mMass;

	// Maximum force with which the character can push other bodies (N)
	float								mMaxStrength;

	// Current position (of the base, not the center of mass)
	Vec3								mPosition = Vec3::sZero();

	// Current rotation (of the base, not of the center of mass)
	Quat								mRotation = Quat::sIdentity();

	// Current linear velocity
	Vec3								mLinearVelocity = Vec3::sZero();

	// List of contacts that were active in the last frame
	ContactList							mActiveContacts;

	// Remembers the delta time of the last update
	float								mLastDeltaTime = 1.0f / 60.0f;
};

JPH_NAMESPACE_END

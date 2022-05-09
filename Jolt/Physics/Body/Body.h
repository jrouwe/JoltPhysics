// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/CollisionGroup.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyAccess.h>
#include <Jolt/Core/StringTools.h>

JPH_NAMESPACE_BEGIN

class StateRecorder;
class BodyCreationSettings;

/// A rigid body that can be simulated using the physics system
///
/// Note that internally all properties (position, velocity etc.) are tracked relative to the center of mass of the object to simplify the simulation of the object. 
/// 
/// The offset between the position of the body and the center of mass position of the body is GetShape()->GetCenterOfMass().
/// The functions that get/set the position of the body all indicate if they are relative to the center of mass or to the original position in which the shape was created.
///
/// The linear velocity is also velocity of the center of mass, to correct for this: \f$VelocityCOM = Velocity - AngularVelocity \times ShapeCOM\f$.
class Body : public NonCopyable
{
public:
	/// Default constructor
							Body() = default;

	/// Destructor							
							~Body()															{ JPH_ASSERT(mMotionProperties == nullptr); }

	/// Get the id of this body
	inline const BodyID &	GetID() const													{ return mID; }

	/// If this body is currently actively simulating (true) or sleeping (false)
	inline bool				IsActive() const												{ return mMotionProperties != nullptr && mMotionProperties->mIndexInActiveBodies != cInactiveIndex; }

	/// Check if this body is static (not movable)
	inline bool				IsStatic() const												{ return mMotionType == EMotionType::Static; }

	/// Check if this body is kinematic (keyframed), which means that it will move according to its current velocity, but forces don't affect it
	inline bool				IsKinematic() const												{ return mMotionType == EMotionType::Kinematic; }

	/// Check if this body is dynamic, which means that it moves and forces can act on it
	inline bool				IsDynamic() const												{ return mMotionType == EMotionType::Dynamic; }

	/// Check if a body could be made kinematic or dynamic (if it was created dynamic or with mAllowDynamicOrKinematic set to true)
	inline bool				CanBeKinematicOrDynamic() const									{ return mMotionProperties != nullptr; }

	/// Change the body to a sensor. A sensor will receive collision callbacks, but will not cause any collision responses and can be used as a trigger volume.
	/// The cheapest sensor (in terms of CPU usage) is a sensor with motion type Static (they can be moved around using BodyInterface::SetPosition/SetPositionAndRotation).
	/// These sensors will only detect collisions with active Dynamic or Kinematic bodies. As soon as a body go to sleep, the contact point with the sensor will be lost.
	/// If you make a sensor Dynamic or Kinematic and activate them, the sensor will be able to detect collisions with sleeping bodies too. An active sensor will never go to sleep automatically.
	/// When you make a Dynamic or Kinematic sensor, make sure it is in an ObjectLayer that does not collide with Static bodies or other sensors to avoid extra overhead in the broad phase.
	inline void				SetIsSensor(bool inIsSensor)									{ if (inIsSensor) mFlags.fetch_or(uint8(EFlags::IsSensor), memory_order_relaxed); else mFlags.fetch_and(uint8(~uint8(EFlags::IsSensor)), memory_order_relaxed); }

	/// Check if this body is a sensor.
	inline bool				IsSensor() const												{ return (mFlags.load(memory_order_relaxed) & uint8(EFlags::IsSensor)) != 0; }

	/// Motion type of this body
	inline EMotionType		GetMotionType() const											{ return mMotionType; }
	void					SetMotionType(EMotionType inMotionType);

	/// Get broadphase layer, this determines in which broad phase sub-tree the object is placed
	inline BroadPhaseLayer	GetBroadPhaseLayer() const										{ return mBroadPhaseLayer; }	

	/// Get object layer, this determines which other objects it collides with
	inline ObjectLayer		GetObjectLayer() const											{ return mObjectLayer; }

	/// Collision group and sub-group ID, determines which other objects it collides with
	const CollisionGroup &	GetCollisionGroup() const										{ return mCollisionGroup; }
	CollisionGroup &		GetCollisionGroup()												{ return mCollisionGroup; }
	void					SetCollisionGroup(const CollisionGroup &inGroup)				{ mCollisionGroup = inGroup; }

	/// If this body can go to sleep. Note that disabling sleeping on a sleeping object wil not wake it up.
	bool					GetAllowSleeping() const										{ return mMotionProperties->mAllowSleeping; }
	void					SetAllowSleeping(bool inAllow);

	/// Friction (dimensionless number, usually between 0 and 1, 0 = no friction, 1 = friction force equals force that presses the two bodies together)
	inline float			GetFriction() const												{ return mFriction; }
	void					SetFriction(float inFriction)									{ JPH_ASSERT(inFriction >= 0.0f); mFriction = inFriction; }

	/// Restitution (dimensionless number, usually between 0 and 1, 0 = completely inelastic collision response, 1 = completely elastic collision response)
	inline float			GetRestitution() const											{ return mRestitution; }
	void					SetRestitution(float inRestitution)								{ JPH_ASSERT(inRestitution >= 0.0f && inRestitution <= 1.0f); mRestitution = inRestitution; }

	/// Get world space linear velocity of the center of mass (unit: m/s)
	inline Vec3				GetLinearVelocity() const										{ return !IsStatic()? mMotionProperties->GetLinearVelocity() : Vec3::sZero(); }

	/// Set world space linear velocity of the center of mass (unit: m/s)
	void					SetLinearVelocity(Vec3Arg inLinearVelocity)						{ JPH_ASSERT(!IsStatic()); mMotionProperties->SetLinearVelocity(inLinearVelocity); }

	/// Set world space linear velocity of the center of mass, will make sure the value is clamped against the maximum linear velocity
	void					SetLinearVelocityClamped(Vec3Arg inLinearVelocity)				{ JPH_ASSERT(!IsStatic()); mMotionProperties->SetLinearVelocityClamped(inLinearVelocity); }

	/// Get world space angular velocity of the center of mass (unit: rad/s)
	inline Vec3				GetAngularVelocity() const										{ return !IsStatic()? mMotionProperties->GetAngularVelocity() : Vec3::sZero(); }

	/// Set world space angular velocity of the center of mass (unit: rad/s)
	void					SetAngularVelocity(Vec3Arg inAngularVelocity)					{ JPH_ASSERT(!IsStatic()); mMotionProperties->SetAngularVelocity(inAngularVelocity); }

	/// Set world space angular velocity of the center of mass, will make sure the value is clamped against the maximum angular velocity
	void					SetAngularVelocityClamped(Vec3Arg inAngularVelocity)			{ JPH_ASSERT(!IsStatic()); mMotionProperties->SetAngularVelocityClamped(inAngularVelocity); }

	/// Velocity of point inPoint (in center of mass space, e.g. on the surface of the body) of the body (unit: m/s)
	inline Vec3				GetPointVelocityCOM(Vec3Arg inPointRelativeToCOM) const			{ return !IsStatic()? mMotionProperties->GetPointVelocityCOM(inPointRelativeToCOM) : Vec3::sZero(); }

	/// Velocity of point inPoint (in world space, e.g. on the surface of the body) of the body (unit: m/s)
	inline Vec3				GetPointVelocity(Vec3Arg inPoint) const							{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read)); return GetPointVelocityCOM(inPoint - mPosition); }

	/// Add force (unit: N) at center of mass for the next time step, will be reset after the next call to PhysicsSimulation::Update
	inline void				AddForce(Vec3Arg inForce)										{ JPH_ASSERT(IsDynamic()); (Vec3::sLoadFloat3Unsafe(mMotionProperties->mForce) + inForce).StoreFloat3(&mMotionProperties->mForce); }

	/// Add force (unit: N) at inPosition for the next time step, will be reset after the next call to PhysicsSimulation::Update
	inline void				AddForce(Vec3Arg inForce, Vec3Arg inPosition);

	/// Add torque (unit: N m) for the next time step, will be reset after the next call to PhysicsSimulation::Update
	inline void				AddTorque(Vec3Arg inTorque)										{ JPH_ASSERT(IsDynamic()); (Vec3::sLoadFloat3Unsafe(mMotionProperties->mTorque) + inTorque).StoreFloat3(&mMotionProperties->mTorque); }

	/// Get inverse inertia tensor in world space
	inline Mat44			GetInverseInertia() const;

	/// Add impulse to center of mass (unit: kg m/s)
	inline void				AddImpulse(Vec3Arg inImpulse);

	/// Add impulse to point in world space (unit: kg m/s)
	inline void				AddImpulse(Vec3Arg inImpulse, Vec3Arg inPosition);

	/// Add angular impulse in world space (unit: N m s)
	inline void				AddAngularImpulse(Vec3Arg inAngularImpulse);
	
	/// Set velocity of body such that it will be positioned at inTargetPosition/Rotation in inDeltaTime seconds.
	void					MoveKinematic(Vec3Arg inTargetPosition, QuatArg inTargetRotation, float inDeltaTime);

	/// Applies an impulse to the body that simulates fluid buoyancy and drag
	/// @param inSurface The fluid surface (normal should point up) in world space
	/// @param inBuoyancy The buoyancy factor for the body. 1 = neutral body, < 1 sinks, > 1 floats. Note that we don't use the fluid density since it is harder to configure than a simple number between [0, 2]
	/// @param inLinearDrag Linear drag factor that slows down the body when in the fluid (approx. 0.5)
	/// @param inAngularDrag Angular drag factor that slows down rotation when the body is in the fluid (approx. 0.01)
	/// @param inFluidVelocity The average velocity of the fluid (in m/s) in which the body resides
	/// @param inGravity The graviy vector (pointing down)
	/// @param inDeltaTime Delta time of the next simulation step (in s)
	/// @return true if an impulse was applied, false if the body was not in the fluid
	bool					ApplyBuoyancyImpulse(const Plane &inSurface, float inBuoyancy, float inLinearDrag, float inAngularDrag, Vec3Arg inFluidVelocity, Vec3Arg inGravity, float inDeltaTime);

	/// Check if this body has been added to the physics system
	inline bool				IsInBroadPhase() const											{ return (mFlags.load(memory_order_relaxed) & uint8(EFlags::IsInBroadPhase)) != 0; }

	/// Check if this body has been changed in such a way that the collision cache should be considered invalid for any body interacting with this body
	inline bool				IsCollisionCacheInvalid() const									{ return (mFlags.load(memory_order_relaxed) & uint8(EFlags::InvalidateContactCache)) != 0; }

	/// Get the shape of this body
	inline const Shape *	GetShape() const												{ return mShape; }

	/// World space position of the body
	inline Vec3				GetPosition() const												{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read)); return mPosition - mRotation * mShape->GetCenterOfMass(); }

	/// World space rotation of the body
	inline Quat 			GetRotation() const												{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read)); return mRotation; }

	/// Calculates the transform of this body
	inline Mat44			GetWorldTransform() const;

	/// Gets the world space position of this body's center of mass
	inline Vec3 			GetCenterOfMassPosition() const									{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read)); return mPosition; }

	/// Calculates the transform for this body's center of mass
	inline Mat44			GetCenterOfMassTransform() const;

	/// Calculates the inverse of the transform for this body's center of mass
	inline Mat44			GetInverseCenterOfMassTransform() const;

	/// Get world space bounding box
	inline const AABox &	GetWorldSpaceBounds() const										{ return mBounds; }

	/// Access to the motion properties
	const MotionProperties *GetMotionProperties() const										{ JPH_ASSERT(!IsStatic()); return mMotionProperties; }
	MotionProperties *		GetMotionProperties()											{ JPH_ASSERT(!IsStatic()); return mMotionProperties; }

	/// Access to the motion properties (version that does not check if the object is kinematic or dynamic)
	const MotionProperties *GetMotionPropertiesUnchecked() const							{ return mMotionProperties; }
	MotionProperties *		GetMotionPropertiesUnchecked()									{ return mMotionProperties; }

	/// Access to the user data, can be used for anything by the application
	uint64					GetUserData() const												{ return mUserData; }
	void					SetUserData(uint64 inUserData)									{ mUserData = inUserData; }

	/// Get surface normal of a particular sub shape and its world space surface position on this body
	inline Vec3				GetWorldSpaceSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inPosition) const;

	/// Get the transformed shape of this body, which can be used to do collision detection outside of a body lock
	inline TransformedShape	GetTransformedShape() const										{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read)); return TransformedShape(mPosition, mRotation, mShape, mID); }

	/// Debug function to convert a body back to a body creation settings object to be able to save/recreate the body later
	BodyCreationSettings	GetBodyCreationSettings() const;

	/// A dummy body that can be used by constraints to attach a constraint to the world instead of another body
	static Body				sFixedToWorld;

	///@name THESE FUNCTIONS ARE FOR INTERNAL USE ONLY AND SHOULD NOT BE CALLED BY THE APPLICATION
	///@{

	/// Helper function for BroadPhase::FindCollidingPairs that returns true when two bodies can collide
	/// It assumes that body 1 is dynamic and active and guarantees that it body 1 collides with body 2 that body 2 will not collide with body 1 in order to avoid finding duplicate collision pairs
	static inline bool		sFindCollidingPairsCanCollide(const Body &inBody1, const Body &inBody2);

	/// Update position using an Euler step (used during position integrate & constraint solving)
	inline void				AddPositionStep(Vec3Arg inLinearVelocityTimesDeltaTime)			{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::ReadWrite)); mPosition += inLinearVelocityTimesDeltaTime; JPH_ASSERT(!mPosition.IsNaN()); }
	inline void				SubPositionStep(Vec3Arg inLinearVelocityTimesDeltaTime) 		{ JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::ReadWrite)); mPosition -= inLinearVelocityTimesDeltaTime; JPH_ASSERT(!mPosition.IsNaN()); }

	/// Update rotation using an Euler step (using during position integrate & constraint solving)
	inline void				AddRotationStep(Vec3Arg inAngularVelocityTimesDeltaTime);
	inline void				SubRotationStep(Vec3Arg inAngularVelocityTimesDeltaTime);

	/// Flag if body is in the broadphase (should only be called by the BroadPhase)
	inline void				SetInBroadPhaseInternal(bool inInBroadPhase)					{ if (inInBroadPhase) mFlags.fetch_or(uint8(EFlags::IsInBroadPhase), memory_order_relaxed); else mFlags.fetch_and(uint8(~uint8(EFlags::IsInBroadPhase)), memory_order_relaxed); }

	/// Invalidate the contact cache (should only be called by the BodyManager), will be reset the next simulation step. Returns true if the contact cache was still valid.
	inline bool				InvalidateContactCacheInternal()								{ return (mFlags.fetch_or(uint8(EFlags::InvalidateContactCache), memory_order_relaxed) & uint8(EFlags::InvalidateContactCache)) == 0; }

	/// Reset the collision cache invalid flag (should only be called by the BodyManager).
	inline void				ValidateContactCacheInternal()									{ JPH_IF_ENABLE_ASSERTS(uint8 old_val = ) mFlags.fetch_and(uint8(~uint8(EFlags::InvalidateContactCache)), memory_order_relaxed); JPH_ASSERT((old_val & uint8(EFlags::InvalidateContactCache)) != 0); }

	/// Updates world space bounding box (should only be called by the PhysicsSystem)
	void					CalculateWorldSpaceBoundsInternal();

	/// Function to update body's position (should only be called by the BodyInterface since it also requires updating the broadphase)
	void					SetPositionAndRotationInternal(Vec3Arg inPosition, QuatArg inRotation);

	/// Updates the center of mass and optionally mass propertes after shifting the center of mass or changes to the shape (should only be called by the BodyInterface since it also requires updating the broadphase)
	/// @param inPreviousCenterOfMass Center of mass of the shape before the alterations
	/// @param inUpdateMassProperties When true, the mass and inertia tensor is recalculated
	void					UpdateCenterOfMassInternal(Vec3Arg inPreviousCenterOfMass, bool inUpdateMassProperties);

	/// Function to update a body's shape (should only be called by the BodyInterface since it also requires updating the broadphase)
	/// @param inShape The new shape for this body
	/// @param inUpdateMassProperties When true, the mass and inertia tensor is recalculated
	void					SetShapeInternal(const Shape *inShape, bool inUpdateMassProperties);

	/// Access to the index in the BodyManager::mActiveBodies list
	uint32					GetIndexInActiveBodiesInternal() const							{ return mMotionProperties != nullptr? mMotionProperties->mIndexInActiveBodies : cInactiveIndex; }

	enum class ECanSleep
	{
		CannotSleep = 0,																	///< Object cannot go to sleep
		CanSleep = 1,																		///< Object can go to sleep
	};

	/// Update eligibility for sleeping
	ECanSleep				UpdateSleepStateInternal(float inDeltaTime, float inMaxMovement, float inTimeBeforeSleep);

	/// Saving state for replay
	void					SaveState(StateRecorder &inStream) const;

	/// Restoring state for replay
	void					RestoreState(StateRecorder &inStream);

	///@}

	static constexpr uint32	cInactiveIndex = uint32(-1);									///< Constant indicating that body is not active

private:
	friend class BodyManager;

	explicit				Body(bool);														///< Alternative constructor that initializes all members

	inline void				GetSleepTestPoints(Vec3 *outPoints) const;						///< Determine points to test for checking if body is sleeping: COM, COM + largest bounding box axis, COM + second largest bounding box axis
	inline void				ResetSleepTestSpheres();										///< Reset spheres to current position as returned by GetSleepTestPoints

	enum class EFlags : uint8
	{
		IsSensor				= 1 << 0,													///< If this object is a sensor. A sensor will receive collision callbacks, but will not cause any collision responses and can be used as a trigger volume.
		IsInBroadPhase			= 1 << 1,													///< Set this bit to indicate that the body is in the broadphase
		InvalidateContactCache	= 1 << 2													///< Set this bit to indicate that all collision caches for this body are invalid, will be reset the next simulation step.
	};

	// 16 byte aligned
	Vec3					mPosition;														///< World space position of center of mass
	Quat					mRotation;														///< World space rotation of center of mass
	AABox					mBounds;														///< World space bounding box of the body

	// 8 byte aligned
	RefConst<Shape>			mShape;															///< Shape representing the volume of this body
	MotionProperties *		mMotionProperties = nullptr;									///< If this is a keyframed or dynamic object, this object holds all information about the movement
	uint64					mUserData = 0;													///< User data, can be used for anything by the application
	CollisionGroup			mCollisionGroup;												///< The collision group this body belongs to (determines if two objects can collide)

	// 4 byte aligned
	float					mFriction;														///< Friction of the body (dimensionless number, usually between 0 and 1, 0 = no friction, 1 = friction force equals force that presses the two bodies together)
	float					mRestitution;													///< Restitution of body (dimensionless number, usually between 0 and 1, 0 = completely inelastic collision response, 1 = completely elastic collision response)
	BodyID					mID;															///< ID of the body (index in the bodies array)

	// 2 bytes aligned
	ObjectLayer				mObjectLayer;													///< The collision layer this body belongs to (determines if two objects can collide)

	// 1 byte aligned
	BroadPhaseLayer			mBroadPhaseLayer;												///< The broad phase layer this body belongs to
	EMotionType				mMotionType;													///< Type of motion (static, dynamic or kinematic)
	atomic<uint8>			mFlags = 0;														///< See EFlags for possible flags
	
	// 121 bytes up to here
};

static_assert(sizeof(Body) == 128, "Body should be 128 bytes");
static_assert(alignof(Body) == 16, "Body should align to 16 bytes");

JPH_NAMESPACE_END

#include "Body.inl"

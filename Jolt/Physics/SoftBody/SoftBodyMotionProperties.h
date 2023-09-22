// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyVertex.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
class Body;
class Shape;
class SoftBodyCreationSettings;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// This class contains the runtime information of a soft body. Soft bodies are implemented using XPBD, a particle and springs based approach.
class JPH_EXPORT SoftBodyMotionProperties : public MotionProperties
{
public:
	using Vertex = SoftBodyVertex;
	using Edge = SoftBodySharedSettings::Edge;
	using Face = SoftBodySharedSettings::Face;
	using Volume = SoftBodySharedSettings::Volume;

	/// Initialize the soft body motion properties
	void								Initialize(const SoftBodyCreationSettings &inSettings);

	/// Update the soft body
	ECanSleep							Update(float inDeltaTime, Body &inSoftBody, Vec3 &outDeltaPosition, PhysicsSystem &inSystem);

	/// Get the shared settings of the soft body
	const SoftBodySharedSettings *		GetSettings() const							{ return mSettings; }

	/// Get the vertices of the soft body
	const Array<Vertex> &				GetVertices() const							{ return mVertices; }
	Array<Vertex> &						GetVertices()								{ return mVertices; }

	/// Access an individual vertex
	const Vertex &						GetVertex(uint inIndex) const				{ return mVertices[inIndex]; }
	Vertex &							GetVertex(uint inIndex)						{ return mVertices[inIndex]; }

	/// Get the materials of the soft body
	const PhysicsMaterialList &			GetMaterials() const						{ return mSettings->mMaterials; }

	/// Get the faces of the soft body
	const Array<Face> &					GetFaces() const							{ return mSettings->mFaces; }

	/// Access to an individual face
	const Face &						GetFace(uint inIndex) const					{ return mSettings->mFaces[inIndex]; }

	/// Get the number of solver iterations
	uint32								GetNumIterations() const					{ return mNumIterations; }
	void								SetNumIterations(uint32 inNumIterations)	{ mNumIterations = inNumIterations; }

	/// Get the pressure of the soft body
	float								GetPressure() const							{ return mPressure; }
	void								SetPressure(float inPressure)				{ mPressure = inPressure; }

	/// Update the position of the body while simulating (set to false for something that is attached to the static world)
	bool								GetUpdatePosition() const					{ return mUpdatePosition; }
	void								SetUpdatePosition(bool inUpdatePosition)	{ mUpdatePosition = inUpdatePosition; }

	/// Get local bounding box
	const AABox &						GetLocalBounds() const						{ return mLocalBounds; }

	/// Get the volume of the soft body. Note can become negative if the shape is inside out!
	float								GetVolume() const							{ return GetVolumeTimesSix() / 6.0f; }

	/// Calculate the total mass and inertia of this body based on the current state of the vertices
	void								CalculateMassAndInertia();

#ifdef JPH_DEBUG_RENDERER
	/// Draw the state of a soft body
	void								DrawVertices(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const;
	void								DrawEdgeConstraints(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const;
	void								DrawVolumeConstraints(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const;
	void								DrawPredictedBounds(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform) const;
#endif // JPH_DEBUG_RENDERER

	/// Saving state for replay
	void								SaveState(StateRecorder &inStream) const;

	/// Restoring state for replay
	void								RestoreState(StateRecorder &inStream);

private:
	// Temporary data used by the update of a soft body
	struct UpdateContext
	{
		RMat44							mCenterOfMassTransform;						///< Transform of the body relative to the soft body
		Vec3							mGravity;									///< Gravity vector in local space of the soft body
		float							mSubStepDeltaTime;							///< Delta time for each sub step
		Vec3							mDisplacementDueToGravity;					///< Displacement of the center of mass due to gravity in the current time step
	};

	// Collect information about the colliding bodies
	struct CollidingShape
	{
		/// Get the velocity of a point on this body
		Vec3			GetPointVelocity(Vec3Arg inPointRelativeToCOM) const
		{
			return mLinearVelocity + mAngularVelocity.Cross(inPointRelativeToCOM);
		}

		Mat44							mCenterOfMassTransform;						///< Transform of the body relative to the soft body
		RefConst<Shape>					mShape;										///< Shape of the body we hit
		BodyID							mBodyID;									///< Body ID of the body we hit
		EMotionType						mMotionType;								///< Motion type of the body we hit
		float							mInvMass;									///< Inverse mass of the body we hit
		float							mFriction;									///< Combined friction of the two bodies
		float							mRestitution;								///< Combined restitution of the two bodies
		bool 							mUpdateVelocities;							///< If the linear/angular velocity changed and the body needs to be updated
		Mat44							mInvInertia;								///< Inverse inertia in local space to the soft body
		Vec3							mLinearVelocity;							///< Linear velocity of the body in local space to the soft body
		Vec3							mAngularVelocity;							///< Angular velocity of the body in local space to the soft body
	};

	/// Do a broad phase check and collect all bodies that can possibly collide with this soft body
	void								DetermineCollidingShapes(const UpdateContext &inContext, Body &inSoftBody, PhysicsSystem &inSystem);

	/// Do a narrow phase check and determine the closest feature that we can collide with
	void								DetermineCollisionPlanes(const UpdateContext &inContext, float inDeltaTime, uint inVertexStart, uint inNumVertices);

	/// Apply pressure force and update the vertex velocities
	void								ApplyPressure(const UpdateContext &inContext);

	/// Integrate the positions of all vertices by 1 sub step
	void								IntegratePositions(const UpdateContext &inContext);

	/// Enforce all volume constraints
	void								ApplyVolumeConstraints(const UpdateContext &inContext);

	/// Enforce all edge constraints
	void								ApplyEdgeConstraints(const UpdateContext &inContext);

	/// Enforce all collision constraints & update all velocities according the the XPBD algorithm
	void								ApplyCollisionConstraintsAndUpdateVelocities(const UpdateContext &inContext);

	/// Update the velocities of all rigid bodies that we collided with
	void								UpdateRigidBodyVelocities(const UpdateContext &inContext, PhysicsSystem &inSystem);

	/// Update the state of the soft body (position, velocity, bounds)
	ECanSleep							UpdateSoftBodyState(const UpdateContext &inContext, float inDeltaTime, Vec3 &outDeltaPosition, PhysicsSystem &inSystem);

	/// Returns 6 times the volume of the soft body
	float								GetVolumeTimesSix() const;

	RefConst<SoftBodySharedSettings>	mSettings;									///< Configuration of the particles and constraints
	Array<Vertex>						mVertices;									///< Current state of all vertices in the simulation
	Array<CollidingShape>				mCollidingShapes;							///< List of colliding shapes retrieved during the last update
	AABox								mLocalBounds;								///< Bounding box of all vertices
	AABox								mLocalPredictedBounds;						///< Predicted bounding box for all vertices using extrapolation of velocity by last step delta time
	uint32								mNumIterations;								///< Number of solver iterations
	float								mPressure;									///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool								mUpdatePosition;							///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyVertex.h>
#include <Jolt/Physics/SoftBody/SoftBodyUpdateContext.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
struct PhysicsSettings;
class Body;
class Shape;
class SoftBodyCreationSettings;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// This class contains the runtime information of a soft body.
//
// Based on: XPBD, Extended Position Based Dynamics, Matthias Muller, Ten Minute Physics
// See: https://matthias-research.github.io/pages/tenMinutePhysics/09-xpbd.pdf
class JPH_EXPORT SoftBodyMotionProperties : public MotionProperties
{
public:
	using Vertex = SoftBodyVertex;
	using Edge = SoftBodySharedSettings::Edge;
	using Face = SoftBodySharedSettings::Face;
	using Volume = SoftBodySharedSettings::Volume;

	/// Initialize the soft body motion properties
	void								Initialize(const SoftBodyCreationSettings &inSettings);

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

	/// Initialize the update context (used internally by the PhysicsSystem)
	void								InitializeUpdateContext(float inDeltaTime, Body &inSoftBody, const PhysicsSystem &inSystem, SoftBodyUpdateContext &ioContext);

	/// Do a broad phase check and collect all bodies that can possibly collide with this soft body
	void								DetermineCollidingShapes(const SoftBodyUpdateContext &inContext, const PhysicsSystem &inSystem);

	/// Return code for ParallelUpdate
	enum class EStatus
	{
		NoWork	= 1 << 0,				///< No work was done because other threads were still working on a batch that cannot run concurrently
		DidWork	= 1 << 1,				///< Work was done to progress the update
		Done	= 1 << 2,				///< All work is done
	};

	/// Update the soft body, will process a batch of work. Used internally.
	EStatus								ParallelUpdate(SoftBodyUpdateContext &ioContext, const PhysicsSettings &inPhysicsSettings);

	/// Update the velocities of all rigid bodies that we collided with. Used internally.
	void								UpdateRigidBodyVelocities(const SoftBodyUpdateContext &inContext, PhysicsSystem &inSystem);

private:
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
		Vec3							mOriginalLinearVelocity;					///< Linear velocity of the body in local space to the soft body at start
		Vec3							mOriginalAngularVelocity;					///< Angular velocity of the body in local space to the soft body at start
	};

	/// Do a narrow phase check and determine the closest feature that we can collide with
	void								DetermineCollisionPlanes(const SoftBodyUpdateContext &inContext, uint inVertexStart, uint inNumVertices);

	/// Apply pressure force and update the vertex velocities
	void								ApplyPressure(const SoftBodyUpdateContext &inContext);

	/// Integrate the positions of all vertices by 1 sub step
	void								IntegratePositions(const SoftBodyUpdateContext &inContext);

	/// Enforce all volume constraints
	void								ApplyVolumeConstraints(const SoftBodyUpdateContext &inContext);

	/// Enforce all edge constraints
	void								ApplyEdgeConstraints(const SoftBodyUpdateContext &inContext, uint inStartIndex, uint inEndIndex);

	/// Enforce all collision constraints & update all velocities according the the XPBD algorithm
	void								ApplyCollisionConstraintsAndUpdateVelocities(const SoftBodyUpdateContext &inContext);

	/// Update the state of the soft body (position, velocity, bounds)
	void								UpdateSoftBodyState(SoftBodyUpdateContext &ioContext, const PhysicsSettings &inPhysicsSettings);

	/// Executes tasks that need to run on the start of an iteration (i.e. the stuff that can't run in parallel)
	void								StartNextIteration(const SoftBodyUpdateContext &ioContext);

	/// Helper function for ParallelUpdate that works on batches of collision planes
	EStatus								ParallelDetermineCollisionPlanes(SoftBodyUpdateContext &ioContext);

	/// Helper function for ParallelUpdate that works on batches of edges
	EStatus								ParallelApplyEdgeConstraints(SoftBodyUpdateContext &ioContext, const PhysicsSettings &inPhysicsSettings);

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

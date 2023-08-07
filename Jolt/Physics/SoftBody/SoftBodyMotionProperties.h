// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/SoftBody/SoftBodySharedSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyVertex.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
class Body;
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
	/// Returns 6 times the volume of the soft body
	float								GetVolumeTimesSix() const;

	RefConst<SoftBodySharedSettings>	mSettings;									///< Configuration of the particles and constraints
	Array<Vertex>						mVertices;									///< Current state of all vertices in the simulation
	AABox								mLocalBounds;								///< Bounding box of all vertices
	AABox								mLocalPredictedBounds;						///< Predicted bounding box for all vertices using extrapolation of velocity by last step delta time
	uint32								mNumIterations;								///< Number of solver iterations
	float								mPressure;									///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool								mUpdatePosition;							///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Geometry/AABox.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// This class defines the setup of all particles and their constraints.
/// It is used during the simulation and can be shared between multiple soft bodies.
class JPH_EXPORT SoftBodyParticleSettings : public RefTarget<SoftBodyParticleSettings>
{
public:
	/// Calculate the initial lengths of all springs of the edges of this soft body
	void				CalculateEdgeLengths()
	{
		for (Edge &e : mEdgeConstraints)
		{
			e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
			JPH_ASSERT(e.mRestLength > 0.0f);
		}
	}

	/// Calculates the initial volume of all tetrahedra of this soft body
	void				CalculateVolumeConstraintVolumes()
	{
		for (Volume &v : mVolumeConstraints)
		{
			Vec3 x1(mVertices[v.mVertex[0]].mPosition);
			Vec3 x2(mVertices[v.mVertex[1]].mPosition);
			Vec3 x3(mVertices[v.mVertex[2]].mPosition);
			Vec3 x4(mVertices[v.mVertex[3]].mPosition);

			Vec3 x1x2 = x2 - x1;
			Vec3 x1x3 = x3 - x1;
			Vec3 x1x4 = x4 - x1;

			v.mSixRestVolume = abs(x1x2.Cross(x1x3).Dot(x1x4));
		}
	}

	/// A vertex is a particle
	struct Vertex
	{
		Float3			mPosition { 0, 0, 0 };				///< Initial position of the vertex
		Float3			mVelocity { 0, 0, 0 };				///< Initial velocity of the vertex	
		float			mInvMass = 1.0f;					///< Inverse of the mass of the vertex
	};

	/// A face defines the surface of the body
	struct Face
	{
		uint32			mVertex[3];							///< Indices of the vertices that form the face
	};

	/// An edge keeps two vertices at a constant distance using a spring: |x1 - x2| = rest length
	struct Edge
	{
		uint32			mVertex[2];							///< Indices of the vertices that form the edge
		float			mRestLength = 1.0f;					///< Rest length of the spring
		float			mCompliance = 0.0f;					///< Inverse of the stiffness of the spring
	};

	/// Volume constraint, keeps the volume of a tetrahedron constant
	struct Volume
	{
		uint32			mVertex[4];							///< Indices of the vertices that form the tetrhedron
		float			mSixRestVolume = 1.0f;				///< 6 times the rest volume of the tetrahedron
		float			mCompliance = 0.0f;					///< Inverse of the stiffness of the constraint
	};

	Array<Vertex>		mVertices;							///< The list of vertices or particles of the body
	Array<Face>			mFaces;								///< The list of faces of the body
	Array<Edge>			mEdgeConstraints;					///< The list of edges or springs of the body
	Array<Volume>		mVolumeConstraints;					///< The list of volume constraints of the body that keep the volume of tetrahedra in the soft body constant
};

/// This class contains the information needed to create a soft body object
class JPH_EXPORT SoftBodyCreationSettings
{
public:
	/// Constructor
						SoftBodyCreationSettings() = default;
						SoftBodyCreationSettings(const SoftBodyParticleSettings *inSettings, RVec3Arg inPosition = RVec3::sZero(), QuatArg inRotation = Quat::sIdentity()) : mSettings(inSettings), mPosition(inPosition), mRotation(inRotation) { }

	RefConst<SoftBodyParticleSettings> mSettings;			///< Defines the configuration of this soft body
	RVec3				mPosition { RVec3::sZero() };		///< Initial position of the soft body
	Quat				mRotation { Quat::sIdentity() };	///< Initial rotation of the soft body
	uint32				mNumIterations = 5;					///< Number of solver iterations
	float				mLinearDamping = 0.05f;				///< Linear damping: dv/dt = -mLinearDamping * v
	float				mRestitution = 0.0f;				///< Restitution when colliding
	float				mFriction = 0.2f;					///< Friction coefficient when colliding
	float				mPressure = 0.0f;					///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool				mUpdatePosition = true;				///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

/// This class contains the runtime information of a soft body. Soft bodies are implemented using XPBD, a particle and springs based approach.
class JPH_EXPORT SoftBody
{
public:
	/// Constructor
						SoftBody(const SoftBodyCreationSettings &inCreationSettings);

	/// Update the soft body
	void				Update(float inDeltaTime, PhysicsSystem &inSystem);

#ifdef JPH_DEBUG_RENDERER
	/// Settings for drawing soft bodies
	struct DrawSettings
	{
		bool			mDrawPosition = false;
		bool			mDrawBounds = false;
		bool			mDrawVertices = true;
		bool			mDrawFaces = true;
		bool			mDrawEdges = true;
		bool			mDrawVolumeConstraints = true;
	};

	/// Draw the state of a soft body
	void				Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const;
#endif // JPH_DEBUG_RENDERER

	/// Run time information for a single particle
	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		Plane			mCollisionPlane;					///< Nearest collision plane
		int				mCollidingShapeIndex;				///< Index in the colliding shapes list of the body we may collide with
		float			mInvMass;
		float			mProjectedDistance;
	};

	using Edge = SoftBodyParticleSettings::Edge;
	using Face = SoftBodyParticleSettings::Face;
	using Volume = SoftBodyParticleSettings::Volume;

	RefConst<SoftBodyParticleSettings> mSettings;
	Array<Vertex>		mVertices;							///< Current state of all vertices in the simulation
	RVec3				mPosition;							///< Current position of the body (average particle position)
	AABox				mLocalBounds;						///< Current bounding box for all vertices (relative to mPosition)
	AABox				mLocalPredictedBounds;				///< Predicted bounding box for all vertices using extrapolation of velocity by last step delta time (relative to mPosition)
	uint32				mNumIterations;						///< Number of solver iterations
	float				mLinearDamping;						///< Linear damping: dv/dt = -mLinearDamping * v
	float				mRestitution;						///< Restitution when colliding
	float				mFriction;							///< Friction coefficient when colliding
	float				mPressure;							///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool				mUpdatePosition;					///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

JPH_NAMESPACE_END

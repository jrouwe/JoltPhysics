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

class SoftBodySettings : public RefTarget<SoftBodySettings>
{
public:
	void				CalculateEdgeLengths()
	{
		for (Edge &e : mEdgeConstraints)
		{
			e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
			JPH_ASSERT(e.mRestLength > 0.0f);
		}
	}

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

	struct Vertex
	{
		Float3			mPosition;					///< Initial position of the vertex
		Float3			mVelocity { 0, 0, 0 };		///< Initial velocity of the vertex	
		float			mInvMass = 1.0f;			///< Inverse of the mass of the vertex
	};

	struct Face
	{
		uint32			mVertex[3];					///< Indices of the vertices that form the face
	};

	/// An edge is kept at a constant length: |x1 - x2| = rest length
	struct Edge
	{
		uint32			mVertex[2];					///< Indices of the vertices that form the edge
		float			mRestLength = 1.0f;			///< Rest length of the spring
		float			mCompliance = 0.0f;			///< Inverse of the stiffness of the spring
	};

	/// Volume constraint, keeps the volume of a tetrahedron constant
	struct Volume
	{
		uint32			mVertex[4];					///< Indices of the vertices that form the tetrhedron
		float			mSixRestVolume = 1.0f;		///< 6 times the rest volume of the tetrahedron
		float			mCompliance = 0.0f;			///< Inverse of the stiffness of the constraint
	};

	Array<Vertex>		mVertices;
	Array<Face>			mFaces;
	Array<Edge>			mEdgeConstraints;
	Array<Volume>		mVolumeConstraints;

	uint32				mNumIterations = 5;			///< Number of solver iterations
	float				mLinearDamping = 0.05f;		///< Linear damping: dv/dt = -mLinearDamping * v
	float				mRestitution = 0.0f;		///< Restitution when colliding
	float				mFriction = 0.2f;			///< Friction coefficient when colliding
	float				mPressure = 0.0f;			///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool				mUpdatePosition = true;		///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

class SoftBody
{
public:
						SoftBody(const SoftBodySettings *inSettings, RVec3 inPosition, Quat inOrientation);

	void				Update(float inDeltaTime, PhysicsSystem &inSystem);

#ifdef JPH_DEBUG_RENDERER
	struct DrawSettings
	{
		bool			mDrawPosition = false;
		bool			mDrawBounds = false;
		bool			mDrawVertices = true;
		bool			mDrawFaces = true;
		bool			mDrawEdges = true;
		bool			mDrawVolumeConstraints = true;
	};

	void				Draw(DebugRenderer *inRenderer, const DrawSettings &inDrawSettings) const;
#endif // JPH_DEBUG_RENDERER

	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		Plane			mCollisionPlane;			///< Nearest collision plane
		int				mCollidingShapeIndex;		///< Index in the colliding shapes list of the body we may collide with
		float			mInvMass;
		float			mProjectedDistance;
	};

	using Edge = SoftBodySettings::Edge;
	using Face = SoftBodySettings::Face;
	using Volume = SoftBodySettings::Volume;

	RefConst<SoftBodySettings> mSettings;
	Array<Vertex>		mVertices;
	RVec3				mPosition;
	AABox				mLocalBounds;
	AABox				mLocalPredictedBounds;
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

JPH_NAMESPACE_BEGIN

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
		Float3			mPosition { 0, 0, 0 };						///< Initial position of the vertex
		Float3			mVelocity { 0, 0, 0 };						///< Initial velocity of the vertex
		float			mInvMass = 1.0f;							///< Inverse of the mass of the vertex
	};

	/// A face defines the surface of the body
	struct Face
	{
		uint32			mVertex[3];									///< Indices of the vertices that form the face
		uint32			mMaterialIndex = 0;							///< Index of the material of the face in SoftBodyParticleSettings::mMaterials
	};

	/// An edge keeps two vertices at a constant distance using a spring: |x1 - x2| = rest length
	struct Edge
	{
		uint32			mVertex[2];									///< Indices of the vertices that form the edge
		float			mRestLength = 1.0f;							///< Rest length of the spring
		float			mCompliance = 0.0f;							///< Inverse of the stiffness of the spring
	};

	/// Volume constraint, keeps the volume of a tetrahedron constant
	struct Volume
	{
		uint32			mVertex[4];									///< Indices of the vertices that form the tetrhedron
		float			mSixRestVolume = 1.0f;						///< 6 times the rest volume of the tetrahedron
		float			mCompliance = 0.0f;							///< Inverse of the stiffness of the constraint
	};

	Array<Vertex>		mVertices;									///< The list of vertices or particles of the body
	Array<Face>			mFaces;										///< The list of faces of the body
	Array<Edge>			mEdgeConstraints;							///< The list of edges or springs of the body
	Array<Volume>		mVolumeConstraints;							///< The list of volume constraints of the body that keep the volume of tetrahedra in the soft body constant
	PhysicsMaterialList mMaterials { PhysicsMaterial::sDefault };	///< The materials of the faces of the body, referenced by Face::mMaterialIndex
};

JPH_NAMESPACE_END

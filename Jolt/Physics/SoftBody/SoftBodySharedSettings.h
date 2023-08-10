// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include <Jolt/Core/StreamUtils.h>

JPH_NAMESPACE_BEGIN

/// This class defines the setup of all particles and their constraints.
/// It is used during the simulation and can be shared between multiple soft bodies.
class JPH_EXPORT SoftBodySharedSettings : public RefTarget<SoftBodySharedSettings>
{
public:
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SoftBodySharedSettings)

	/// Calculate the initial lengths of all springs of the edges of this soft body
	void				CalculateEdgeLengths();

	/// Calculates the initial volume of all tetrahedra of this soft body
	void				CalculateVolumeConstraintVolumes();

	/// Saves the state of this object in binary form to inStream. Doesn't store the material list.
	void				SaveBinaryState(StreamOut &inStream) const;

	/// Restore the state of this object from inStream. Doesn't restore the material list.
	void				RestoreBinaryState(StreamIn &inStream);

	using SharedSettingsToIDMap = StreamUtils::ObjectToIDMap<SoftBodySharedSettings>;
	using IDToSharedSettingsMap = StreamUtils::IDToObjectMap<SoftBodySharedSettings>;
	using MaterialToIDMap = StreamUtils::ObjectToIDMap<PhysicsMaterial>;
	using IDToMaterialMap = StreamUtils::IDToObjectMap<PhysicsMaterial>;

	/// Save this shared settings and its materials. Pass in an empty map ioSettingsMap / ioMaterialMap or reuse the same map while saving multiple settings objects to the same stream in order to avoid writing duplicates.
	void				SaveWithMaterials(StreamOut &inStream, SharedSettingsToIDMap &ioSettingsMap, MaterialToIDMap &ioMaterialMap) const;

	using SettingsResult = Result<Ref<SoftBodySharedSettings>>;

	/// Restore a shape and materials. Pass in an empty map in ioSettingsMap / ioMaterialMap or reuse the same map while reading multiple settings objects from the same stream in order to restore duplicates.
	static SettingsResult sRestoreWithMaterials(StreamIn &inStream, IDToSharedSettingsMap &ioSettingsMap, IDToMaterialMap &ioMaterialMap);

	/// A vertex is a particle, the data in this structure is only used during creation of the soft body and not during simulation
	struct JPH_EXPORT Vertex
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Vertex)

		/// Constructor
						Vertex() = default;
						Vertex(const Float3 &inPosition, const Float3 &inVelocity = Float3(0, 0, 0), float inInvMass = 1.0f) : mPosition(inPosition), mVelocity(inVelocity), mInvMass(inInvMass) { }

		Float3			mPosition { 0, 0, 0 };						///< Initial position of the vertex
		Float3			mVelocity { 0, 0, 0 };						///< Initial velocity of the vertex
		float			mInvMass = 1.0f;							///< Initial inverse of the mass of the vertex
	};

	/// A face defines the surface of the body
	struct JPH_EXPORT Face
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Face)

		/// Constructor
						Face() = default;
						Face(uint32 inVertex1, uint32 inVertex2, uint32 inVertex3, uint32 inMaterialIndex = 0) : mVertex { inVertex1, inVertex2, inVertex3 }, mMaterialIndex(inMaterialIndex) { }

		/// Check if this is a degenerate face (a face which points to the same vertex twice)
		bool			IsDegenerate() const						{ return mVertex[0] == mVertex[1] || mVertex[0] == mVertex[2] || mVertex[1] == mVertex[2]; }

		uint32			mVertex[3];									///< Indices of the vertices that form the face
		uint32			mMaterialIndex = 0;							///< Index of the material of the face in SoftBodySharedSettings::mMaterials
	};

	/// An edge keeps two vertices at a constant distance using a spring: |x1 - x2| = rest length
	struct JPH_EXPORT Edge
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Edge)

		/// Constructor
						Edge() = default;
						Edge(uint32 inVertex1, uint32 inVertex2, float inCompliance = 0.0f) : mVertex { inVertex1, inVertex2 }, mCompliance(inCompliance) { }

		uint32			mVertex[2];									///< Indices of the vertices that form the edge
		float			mRestLength = 1.0f;							///< Rest length of the spring
		float			mCompliance = 0.0f;							///< Inverse of the stiffness of the spring
	};

	/// Volume constraint, keeps the volume of a tetrahedron constant
	struct JPH_EXPORT Volume
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Volume)

		/// Constructor
						Volume() = default;
						Volume(uint32 inVertex1, uint32 inVertex2, uint32 inVertex3, uint32 inVertex4, float inCompliance = 0.0f) : mVertex { inVertex1, inVertex2, inVertex3, inVertex4 }, mCompliance(inCompliance) { }

		uint32			mVertex[4];									///< Indices of the vertices that form the tetrhedron
		float			mSixRestVolume = 1.0f;						///< 6 times the rest volume of the tetrahedron
		float			mCompliance = 0.0f;							///< Inverse of the stiffness of the constraint
	};

	/// Add a face to this soft body
	void				AddFace(const Face &inFace)					{ JPH_ASSERT(!inFace.IsDegenerate()); mFaces.push_back(inFace); }

	Array<Vertex>		mVertices;									///< The list of vertices or particles of the body
	Array<Face>			mFaces;										///< The list of faces of the body
	Array<Edge>			mEdgeConstraints;							///< The list of edges or springs of the body
	Array<Volume>		mVolumeConstraints;							///< The list of volume constraints of the body that keep the volume of tetrahedra in the soft body constant
	PhysicsMaterialList mMaterials { PhysicsMaterial::sDefault };	///< The materials of the faces of the body, referenced by Face::mMaterialIndex
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/Result.h>
#include <Jolt/Core/UnorderedMap.h>
#include <Jolt/ObjectStream/SerializableObject.h>

JPH_NAMESPACE_BEGIN

class StreamIn;
class StreamOut;
class PhysicsMaterial;

using PhysicsMaterialList = Array<RefConst<PhysicsMaterial>>;

/// This structure describes the surface of (part of) a shape. You should inherit from it to define additional
/// information that is interesting for the simulation. The 2 materials involved in a contact could be used
/// to decide which sound or particle effects to play.
///
/// If you inherit from this material, don't forget to create a suitable default material in sDefault
class JPH_EXPORT PhysicsMaterial : public SerializableObject, public RefTarget<PhysicsMaterial>
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(JPH_EXPORT, PhysicsMaterial)

	/// Virtual destructor
	virtual									~PhysicsMaterial() override = default;

	/// Default material that is used when a shape has no materials defined
	static RefConst<PhysicsMaterial>		sDefault;

	// Properties
	virtual const char *					GetDebugName() const			{ return "Unknown"; }
	virtual Color							GetDebugColor() const			{ return Color::sGrey; }

	/// Saves the contents of the material in binary form to inStream.
	virtual void							SaveBinaryState(StreamOut &inStream) const;

	using PhysicsMaterialResult = Result<Ref<PhysicsMaterial>>;

	/// Creates a PhysicsMaterial of the correct type and restores its contents from the binary stream inStream.
	static PhysicsMaterialResult			sRestoreFromBinaryState(StreamIn &inStream);

	using MaterialToIDMap = UnorderedMap<const PhysicsMaterial *, uint32>;
	using IDToMaterialMap = Array<Ref<PhysicsMaterial>>;

	/// Save a list of materials. Pass in an empty map in ioMaterialMap or reuse the same map while saving multiple shapes to the same stream in order to avoid writing duplicates.
	static void								sSaveMaterialList(StreamOut &inStream, const PhysicsMaterialList &inMaterials, MaterialToIDMap &ioMaterialMap);

	using MLResult = Result<PhysicsMaterialList>;

	/// Restore a list of materials. Pass in an empty map in ioShapeMap / ioMaterialMap or reuse the same map while reading multiple shapes from the same stream in order to restore duplicates.
	static MLResult							sRestoreMaterialList(StreamIn &inStream, IDToMaterialMap &ioMaterialMap);

protected:
	/// This function should not be called directly, it is used by sRestoreFromBinaryState.
	virtual void							RestoreBinaryState(StreamIn &inStream);
};

JPH_NAMESPACE_END

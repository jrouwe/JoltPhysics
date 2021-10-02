// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/PhysicsScene.h>
#include <Physics/PhysicsSystem.h>
#include <ObjectStream/TypeDeclarations.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(PhysicsScene)
{
	JPH_ADD_ATTRIBUTE(PhysicsScene, mBodies)
}

void PhysicsScene::AddBody(const BodyCreationSettings &inBody)
{
	mBodies.push_back(inBody);
}

bool PhysicsScene::FixInvalidScales()
{
	const Vec3 unit_scale = Vec3::sReplicate(1.0f);

	bool success = true;
	for (BodyCreationSettings &b : mBodies)
	{
		// Test if there is an invalid scale in the shape hierarchy
		const Shape *shape = b.GetShape();
		if (!shape->IsValidScale(unit_scale))
		{
			// Fix it up
			Shape::ShapeResult result = shape->ScaleShape(unit_scale);
			if (result.IsValid())
				b.SetShape(result.Get());
			else
				success = false;
		}
	}
	return success;
}

void PhysicsScene::CreateBodies(PhysicsSystem *inSystem)
{
	BodyInterface &bi = inSystem->GetBodyInterface();
	for (const BodyCreationSettings &b : mBodies)
		bi.CreateAndAddBody(b, EActivation::Activate);
}

void PhysicsScene::SaveBinaryState(StreamOut &inStream, bool inSaveShapes, bool inSaveGroupFilter) const
{
	BodyCreationSettings::ShapeToIDMap shape_to_id;
	BodyCreationSettings::MaterialToIDMap material_to_id;
	BodyCreationSettings::GroupFilterToIDMap group_filter_to_id;

	// Save bodies
	inStream.Write((uint32)mBodies.size());
	for (const BodyCreationSettings &b : mBodies)
		b.SaveWithChildren(inStream, inSaveShapes? &shape_to_id : nullptr, inSaveShapes? &material_to_id : nullptr, inSaveGroupFilter? &group_filter_to_id : nullptr);
}

PhysicsScene::PhysicsSceneResult PhysicsScene::sRestoreFromBinaryState(StreamIn &inStream)
{
	PhysicsSceneResult result;

	// Create scene
	Ref<PhysicsScene> scene = new PhysicsScene();

	BodyCreationSettings::IDToShapeMap id_to_shape;
	BodyCreationSettings::IDToMaterialMap id_to_material;
	BodyCreationSettings::IDToGroupFilterMap id_to_group_filter;

	// Reserve some memory to avoid frequent reallocations
	id_to_shape.reserve(1024);
	id_to_material.reserve(128);
	id_to_group_filter.reserve(128);

	// Read bodies
	uint32 len = 0;
	inStream.Read(len);
	scene->mBodies.resize(len);
	for (BodyCreationSettings &b : scene->mBodies)
	{
		// Read creation settings
		BodyCreationSettings::BCSResult bcs_result = BodyCreationSettings::sRestoreWithChildren(inStream, id_to_shape, id_to_material, id_to_group_filter);
		if (bcs_result.HasError())
		{
			result.SetError(bcs_result.GetError());
			return result;
		}
		b = bcs_result.Get();
	}

	result.Set(scene);
	return result;
}

} // JPH
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/DynamicMeshTest.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DynamicMeshTest) 
{ 
	JPH_ADD_BASE_CLASS(DynamicMeshTest, Test) 
}

void DynamicMeshTest::Initialize()
{
	// Floor
	CreateFloor();

	constexpr uint cNumTorusSegments = 16;
	constexpr uint cNumTubeSegments = 16;
	constexpr float cTorusRadius = 3.0f;
	constexpr float cTubeRadius = 1.0f;
	constexpr uint cNumVertices = cNumTorusSegments * cNumTubeSegments;

	// Create torus
	MeshShapeSettings mesh;
	for (uint torus_segment = 0; torus_segment < cNumTorusSegments; ++torus_segment)
	{
		Mat44 rotation = Mat44::sRotation(Vec3::sAxisY(), float(torus_segment) * 2.0f * JPH_PI / cNumTorusSegments);
		for (uint tube_segment = 0; tube_segment < cNumTubeSegments; ++tube_segment)
		{
			// Create vertices
			float tube_angle = float(tube_segment) * 2.0f * JPH_PI / cNumTubeSegments;
			Vec3 pos = rotation * Vec3(cTorusRadius + cTubeRadius * sin(tube_angle), cTubeRadius * cos(tube_angle), 0);
			Float3 v;
			pos.StoreFloat3(&v);
			mesh.mTriangleVertices.push_back(v);

			// Create indices
			uint start_idx = torus_segment * cNumTubeSegments + tube_segment;
			mesh.mIndexedTriangles.emplace_back(start_idx, (start_idx + 1) % cNumVertices, (start_idx + cNumTubeSegments) % cNumVertices);
			mesh.mIndexedTriangles.emplace_back((start_idx + 1) % cNumVertices, (start_idx + cNumTubeSegments + 1) % cNumVertices, (start_idx + cNumTubeSegments) % cNumVertices);
		}
	}
	RefConst<Shape> mesh_shape = mesh.Create().Get();

	// Dynamic mesh
	BodyCreationSettings settings(mesh_shape, Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

	// Mesh cannot calculate its mass, we must provide it
	settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
	settings.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(2.0f * Vec3(cTorusRadius, cTubeRadius, cTorusRadius), 1000.0f);

	mBodyInterface->CreateAndAddBody(settings, EActivation::Activate);
}

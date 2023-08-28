// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/PyramidTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(PyramidTest)
{
	JPH_ADD_BASE_CLASS(PyramidTest, Test)
}

void PyramidTest::Initialize()
{
	// Floor
	CreateFloor();

	const float cBoxSize = 2.0f;
	const float cBoxSeparation = 0.5f;
	const float cHalfBoxSize = 0.5f * cBoxSize;
	const int cPyramidHeight = 15;

	RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(cHalfBoxSize));

	// Pyramid
	for (int i = 0; i < cPyramidHeight; ++i)
		for (int j = i / 2; j < cPyramidHeight - (i + 1) / 2; ++j)
			for (int k = i / 2; k < cPyramidHeight - (i + 1) / 2; ++k)
			{
				RVec3 position(-cPyramidHeight + cBoxSize * j + (i & 1? cHalfBoxSize : 0.0f), 1.0f + (cBoxSize + cBoxSeparation) * i, -cPyramidHeight + cBoxSize * k + (i & 1? cHalfBoxSize : 0.0f));
				mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
			}
}

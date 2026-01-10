// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Hair/HairCollisionTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Application/DebugUI.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(HairCollisionTest)
{
	JPH_ADD_BASE_CLASS(HairCollisionTest, Test)
}

void HairCollisionTest::Initialize()
{
	// Load shaders
	mHairShaders.Init(mComputeSystem);

	// Create a single strand
	mHairSettings = new HairSettings;
	HairSettings::Material m;
	m.mHairRadius = HairSettings::Gradient(0, 0); // Override radius to 0 so we can see it touch the moving body
	mHairSettings->mMaterials.push_back(m);
	mHairSettings->mSimulationBoundsPadding = Vec3::sReplicate(1.0f);
	Array<HairSettings::SVertex> hair_vertices = { HairSettings::SVertex(Float3(0, 2, 0), 0), HairSettings::SVertex(Float3(0, 0, 0), 1) };
	Array<HairSettings::SStrand> hair_strands = { HairSettings::SStrand(0, 2, 0) };
	mHairSettings->InitRenderAndSimulationStrands(hair_vertices, hair_strands);
	float max_dist_sq = 0.0f;
	mHairSettings->Init(max_dist_sq);
	mHairSettings->InitCompute(mComputeSystem);

	mHair = new Hair(mHairSettings, RVec3::sZero(), Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI), Layers::MOVING); // Ensure hair is rotated
	mHair->Init(mComputeSystem);
	mHair->Update(0.0f, Mat44::sIdentity(), nullptr, *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	mHair->ReadBackGPUState(mComputeQueue);

	// Create moving body that moves through the strand
	ConvexHullShapeSettings shape1;
	shape1.SetEmbedded();
	constexpr float cWidth = 0.01f, cHeight = 0.5f, cLength1 = 0.6f;
	shape1.mPoints = {
		Vec3( cWidth,  cHeight,  cLength1),
		Vec3(-cWidth,  cHeight,  cLength1),
		Vec3( cWidth, -cHeight,  cLength1),
		Vec3(-cWidth, -cHeight,  cLength1),
		Vec3( cWidth,  cHeight, -cLength1),
		Vec3(-cWidth,  cHeight, -cLength1),
		Vec3( cWidth, -cHeight, -cLength1),
		Vec3(-cWidth, -cHeight, -cLength1)
	};
	ConvexHullShapeSettings shape2;
	shape2.SetEmbedded();
	constexpr float cLength2 = 0.5f;
	shape2.mPoints = {
		Vec3( cWidth,  cHeight,  cLength2),
		Vec3(-cWidth,  cHeight,  cLength2),
		Vec3( cWidth, -cHeight,  cLength2),
		Vec3(-cWidth, -cHeight,  cLength2),
		Vec3( cWidth,  cHeight, -cLength2),
		Vec3(-cWidth,  cHeight, -cLength2),
		Vec3( cWidth, -cHeight, -cLength2),
		Vec3(-cWidth, -cHeight, -cLength2)
	};
	StaticCompoundShapeSettings compound; // Use a compound to test center of mass differences between body and shape
	compound.SetEmbedded();
	compound.AddShape(Vec3(0, 0, -cLength2), Quat::sIdentity(), &shape1);
	compound.AddShape(Vec3(0, 0, cLength1), Quat::sIdentity(), &shape2);
	BodyCreationSettings moving_body(&compound, RVec3(-1, 0, 0), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING);
	mMovingBodyID = mBodyInterface->CreateAndAddBody(moving_body, EActivation::Activate);
}

void HairCollisionTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
#ifdef JPH_DEBUG_RENDERER
	Hair::DrawSettings settings;
	settings.mDrawRods = true;
	settings.mDrawOrientations = true;
	mHair->Draw(settings, mDebugRenderer);
#endif // JPH_DEBUG_RENDERER

	// Set moving body velocity
	++mFrame;
	if (sRotating)
		mBodyInterface->SetLinearAndAngularVelocity(mMovingBodyID, Vec3::sZero(), Vec3(0, 1, 0));
	else
		mBodyInterface->SetLinearAndAngularVelocity(mMovingBodyID, mFrame % 240 < 120? Vec3(1, 0, 0) : Vec3(-1, 0, 0), Vec3::sZero());

	// Update the hair
	mHair->Update(inParams.mDeltaTime, Mat44::sIdentity(), nullptr, *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	mComputeQueue->ExecuteAndWait();
	mHair->ReadBackGPUState(mComputeQueue);
}

void HairCollisionTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mFrame);
}

void HairCollisionTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mFrame);
}

void HairCollisionTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateCheckBox(inSubMenu, "Rotating", sRotating, [](UICheckBox::EState inState) { sRotating = inState == UICheckBox::STATE_CHECKED; });
}

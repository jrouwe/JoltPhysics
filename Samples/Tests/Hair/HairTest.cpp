// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Samples.h>

#include <Tests/Hair/HairTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Utils/ReadData.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>
#include <Application/DebugUI.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <filesystem>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_IMPLEMENT_RTTI_VIRTUAL(HairTest)
{
	JPH_ADD_BASE_CLASS(HairTest, Test)
}

auto tenth_of_inch_to_m = [](Mat44Arg inInvNeckTransform, Vec3Arg inVertex) { return inInvNeckTransform * ((2.54f / 1000.0f) * inVertex.Swizzle<SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_X>()); }; // Original model seems to be in 10ths of inches

const HairTest::Groom HairTest::sGrooms[] =
{
	{ "Straight", tenth_of_inch_to_m, false },
	{ "Curly", tenth_of_inch_to_m, false },
	{ "Wavy", tenth_of_inch_to_m, false },
};

const HairTest::Groom *HairTest::sSelectedGroom = &sGrooms[0];

void HairTest::Initialize()
{
	// Check groom file exists
	String groom_file = "w" + String(sSelectedGroom->mName) + ".hair";
	String full_path = AssetStream::sGetAssetsBasePath() + groom_file;
	if (!std::filesystem::exists(full_path))
		FatalError("File %s not found.\n\n"
			"wCurly.hair, wStraight.hair and wWavy.hair should be downloaded from https://www.cemyuksel.com/research/hairmodels/ (or by running Assets/download_hair.sh)", full_path.c_str());

	// Read face mesh and animation
	AssetStream asset_stream("face.bin", std::ios::in | std::ios::binary);
	StreamInWrapper stream(asset_stream.Get());

	// Neck joint index
	stream.Read(mHeadJointIdx);

	// Vertices
	uint32 num_vertices;
	stream.Read(num_vertices);
	Array<Float3> vertices;
	vertices.resize(num_vertices);
	stream.ReadBytes(vertices.data(), sizeof(Float3) * num_vertices);

	// Indices
	uint32 num_indices;
	stream.Read(num_indices);
	Array<IndexedTriangleNoMaterial> indices;
	indices.resize(num_indices);
	stream.ReadBytes(indices.data(), sizeof(IndexedTriangleNoMaterial) * num_indices);

	// Inverse Bind Matrices
	uint32 num_joints;
	stream.Read(num_joints);
	Array<Mat44> inv_bind_pose;
	inv_bind_pose.resize(num_joints);
	stream.ReadBytes(inv_bind_pose.data(), sizeof(Mat44) * num_joints);

	// Skin Weights
	uint num_skin_weights_per_vertex;
	Array<HairSettings::SkinWeight> skin_weights;
	stream.Read(num_skin_weights_per_vertex);
	skin_weights.resize(num_skin_weights_per_vertex * num_vertices);
	stream.ReadBytes(skin_weights.data(), sizeof(HairSettings::SkinWeight) * num_skin_weights_per_vertex * num_vertices);

	// Animation
	uint32 num_frames;
	stream.Read(num_frames);
	mFaceAnimation.resize(num_frames);
	for (uint32 frame = 0; frame < num_frames; ++frame)
	{
		mFaceAnimation[frame].resize(num_joints);
		for (uint32 joint = 0; joint < num_joints; ++joint)
		{
			Float3 translation, rotation;
			stream.Read(translation);
			stream.Read(rotation);
			Quat rotation_quat(rotation.x, rotation.y, rotation.z, sqrt(max(0.0f, 1.0f - Vec3(rotation).LengthSq())));
			mFaceAnimation[frame][joint] = Mat44::sRotationTranslation(rotation_quat, Vec3(translation));
		}
	}

	// Read collision hulls
	uint32 num_hulls;
	stream.Read(num_hulls);
	for (uint32 i = 0; i < num_hulls; ++i)
	{
		// Attached to joint
		uint32 joint_index;
		stream.Read(joint_index);

		// Read number of vertices
		uint32 num_hull_vertices;
		stream.Read(num_hull_vertices);

		// Read vertices
		ConvexHullShapeSettings shape_settings;
		shape_settings.SetEmbedded();
		shape_settings.mPoints.resize(num_hull_vertices);
		for (uint32 j = 0; j < num_hull_vertices; ++j)
			stream.Read(shape_settings.mPoints[j]);

		Mat44 transform = joint_index != 0xffffffff? mFaceAnimation[0][joint_index] : Mat44::sIdentity();
		Mat44 inv_transform = transform.Inversed();
		for (Vec3 &v : shape_settings.mPoints)
			v = inv_transform * v;

		// Create the body
		BodyCreationSettings body(&shape_settings, RVec3(transform.GetTranslation()), transform.GetQuaternion(), EMotionType::Kinematic, Layers::MOVING);
		BodyID body_id = mBodyInterface->CreateAndAddBody(body, EActivation::DontActivate);

		mAttachedBodies.push_back({ joint_index, body_id });
	}

	// Make mesh relative to neck bind pose
	Mat44 inv_bind_neck = inv_bind_pose[mHeadJointIdx];
	Mat44 bind_neck = inv_bind_neck.Inversed();
	for (Float3 &v : vertices)
		(inv_bind_neck * Vec3(v)).StoreFloat3(&v);
	for (Mat44 &m : inv_bind_pose)
		m = m * bind_neck;

	// Read hair file
	Array<uint8> data = ReadData(groom_file.c_str());
	if (data[0] != 'H' || data[1] != 'A' || data[2] != 'I' || data[3] != 'R')
		FatalError("Invalid hair file");

	uint32 features = *reinterpret_cast<const uint32 *>(&data[12]);
	if ((features & 0b10) != 0b10)
		FatalError("We require points to be defined");

	uint32 num_strands = *reinterpret_cast<const uint32 *>(&data[4]);
	uint32 num_points = *reinterpret_cast<const uint32 *>(&data[8]);

	const uint16 *num_segments = nullptr;
	int num_segments_delta = 0;
	const Float3 *points = nullptr;
	if (features & 0b01)
	{
		// Num segments differs per strand
		num_segments = reinterpret_cast<const uint16 *>(&data[128]);
		num_segments_delta = 1;
		points = reinterpret_cast<const Float3 *>(&data[128 + num_strands * sizeof(uint16)]);
	}
	else
	{
		// Num segments is constant
		num_segments = reinterpret_cast<const uint16 *>(&data[16]);
		num_segments_delta = 0;
		points = reinterpret_cast<const Float3 *>(&data[128]);
	}

	// Init strands
	if (sLimitMaxStrands)
		num_strands = std::min(num_strands, sMaxStrands);
	Array<HairSettings::SVertex> hair_vertices;
	hair_vertices.resize(num_points);
	Array<HairSettings::SStrand> hair_strands;
	hair_strands.reserve(num_strands);
	const Mat44 &neck_transform = mFaceAnimation[0][mHeadJointIdx];
	Mat44 inv_neck_transform = neck_transform.Inversed();
	for (uint32 strand = 0; strand < num_strands; ++strand)
	{
		// Transform relative to neck
		Array<Vec3> out_points;
		for (uint16 point = 0; point < *num_segments + 1; ++point)
			out_points.push_back(sSelectedGroom->mVertexTransform(inv_neck_transform, Vec3(points[point])));

		// Attach the first vertex to the skull collision
		if (sSelectedGroom->mAttachToHull)
		{
			const float cMaxDist = 10.0f;
			Vec3 direction = cMaxDist * (out_points[0] - out_points[1]).NormalizedOr(-Vec3::sAxisY());
			Vec3 origin = out_points[0] - 0.5f * direction;
			RRayCast ray(RVec3(neck_transform * origin), neck_transform.Multiply3x3(direction));
			RayCastResult hit;
			if (mPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, hit))
			{
				Vec3 delta = origin + hit.mFraction * direction - out_points[0];
				for (Vec3 &v : out_points)
					v += delta;
			}
		}

		// Add the strand to the hair settings
		uint32 first_point = uint32(hair_vertices.size());
		for (uint32 point = 0; point < uint32(out_points.size()); ++point)
		{
			HairSettings::SVertex v;
			out_points[point].StoreFloat3(&v.mPosition);
			v.mInvMass = point == 0? 0.0f : 1.0f;
			hair_vertices.push_back(v);
		}
		hair_strands.push_back(HairSettings::SStrand(first_point, uint32(hair_vertices.size()), 0));

		points += *num_segments + 1;
		num_segments += num_segments_delta;
	}

	// Resample if requested
	if (sOverrideVerticesPerStrand > 1)
		HairSettings::sResample(hair_vertices, hair_strands, sOverrideVerticesPerStrand);

	// Load shaders
	mHairShaders.Init(mComputeSystem);

	// Init hair settings
	mHairSettings = new HairSettings;
	mHairSettings->mScalpVertices = std::move(vertices);
	mHairSettings->mScalpTriangles = std::move(indices);
	mHairSettings->mScalpInverseBindPose = std::move(inv_bind_pose);
	mHairSettings->mScalpSkinWeights = std::move(skin_weights);
	mHairSettings->mScalpNumSkinWeightsPerVertex = num_skin_weights_per_vertex;
	mHairSettings->mNumIterationsPerSecond = sNumSolverIterationsPerSecond;
	HairSettings::Material m;
	m.mEnableCollision = sEnableCollision;
	m.mEnableLRA = sEnableLRA;
	m.mLinearDamping = sLinearDamping;
	m.mAngularDamping = sAngularDamping;
	m.mFriction = sFriction;
	m.mMaxLinearVelocity = sMaxLinearVelocity;
	m.mMaxAngularVelocity = sMaxAngularVelocity;
	m.mGravityFactor = sGravityFactor;
	m.mGravityPreloadFactor = sGravityPreloadFactor;
	m.mBendCompliance = std::pow(10.0f, sBendComplianceExponent);
	m.mStretchCompliance = std::pow(10.0f, sStretchComplianceExponent);
	m.mInertiaMultiplier = sInertiaMultiplier;
	m.mHairRadius = sHairRadius;
	m.mWorldTransformInfluence = sWorldTransformInfluence;
	m.mGridVelocityFactor = sGridVelocityFactor;
	m.mGridDensityForceFactor = sGridDensityForceFactor;
	m.mGlobalPose = sGlobalPose;
	m.mSkinGlobalPose = sSkinGlobalPose;
	m.mMaxLinearVelocity = 10.0f;
	m.mSimulationStrandsFraction = 0.01f * sSimulationStrandsPercentage;
	mHairSettings->mMaterials.push_back(m);
	mHairSettings->mSimulationBoundsPadding = Vec3::sReplicate(0.1f);
	mHairSettings->mInitialGravity = inv_bind_neck.Multiply3x3(mPhysicsSystem->GetGravity());
	mHairSettings->InitRenderAndSimulationStrands(hair_vertices, hair_strands);
	float max_dist_sq = 0.0f;
	mHairSettings->Init(max_dist_sq);
	JPH_ASSERT(max_dist_sq < 1.0e-4f);

	// Write and read back to test SaveBinaryState
	stringstream stream_data;
	{
		StreamOutWrapper stream_out(stream_data);
		mHairSettings->SaveBinaryState(stream_out);
	}
	mHairSettings = new HairSettings;
	{
		StreamInWrapper stream_in(stream_data);
		mHairSettings->RestoreBinaryState(stream_in);
	}
	mHairSettings->InitCompute(mComputeSystem);

	mHair = new Hair(mHairSettings, RVec3(neck_transform.GetTranslation()), neck_transform.GetQuaternion(), Layers::MOVING);
	mHair->Init(mComputeSystem);
	mHair->Update(0.0f, inv_neck_transform, mFaceAnimation[0].data(), *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	mHair->ReadBackGPUState(mComputeQueue);

#ifdef JPH_DEBUG_RENDERER
	// Update drawing range
	sDrawSimulationStrandCount = (uint)mHairSettings->mSimStrands.size();
#endif // JPH_DEBUG_RENDERER
}

void HairTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	BodyInterface &bi = mPhysicsSystem->GetBodyInterfaceNoLock();

#ifdef JPH_DEBUG_RENDERER
	Hair::DrawSettings settings;
	settings.mSimulationStrandBegin = sDrawSimulationStrandBegin;
	settings.mSimulationStrandEnd = sDrawSimulationStrandBegin + sDrawSimulationStrandCount;
	settings.mDrawRods = sDrawRods;
	settings.mDrawUnloadedRods = sDrawUnloadedRods;
	settings.mDrawRenderStrands = sDrawRenderStrands;
	settings.mRenderStrandColor = sRenderStrandColor;
	settings.mDrawVertexVelocity = sDrawVertexVelocity;
	settings.mDrawAngularVelocity = sDrawAngularVelocity;
	settings.mDrawOrientations = sDrawOrientations;
	settings.mDrawGridVelocity = sDrawGridVelocity;
	settings.mDrawGridDensity = sDrawGridDensity;
	settings.mDrawSkinPoints = sDrawSkinPoints;
	settings.mDrawNeutralDensity = sDrawNeutralDensity;
	settings.mDrawInitialGravity = sDrawInitialGravity;
	mHair->Draw(settings, mDebugRenderer);
#else
	// Draw the rods
	mHair->LockReadBackBuffers();
	const Float3 *positions = mHair->GetRenderPositions();
	RMat44 com = mHair->GetWorldTransform();
	if (sDrawRenderStrands)
	{
		JPH_PROFILE("Draw Render Strands");

		Color color = Color::sWhite;
		Hash<uint32> hasher;
		for (const HairSettings::RStrand &render_strand : mHairSettings->mRenderStrands)
		{
			RVec3 x0 = com * Vec3(positions[render_strand.mStartVtx]);
			for (uint32 v = render_strand.mStartVtx + 1; v < render_strand.mEndVtx; ++v)
			{
				RVec3 x1 = com * Vec3(positions[v]);
				mDebugRenderer->DrawLine(x0, x1, color);
				x0 = x1;
			}
			color = Color(uint32(hasher(color.GetUInt32())) | 0xff000000);
		}
	}
	mHair->UnlockReadBackBuffers();
#endif // JPH_DEBUG_RENDERER

	// Get skinned vertices
	RMat44 neck_transform = mHair->GetWorldTransform();

	if (sDrawHeadMesh)
	{
		JPH_PROFILE("Draw Head Mesh");

		const Float3 *scalp_vertices = mHair->GetScalpVertices();
		Ref<DebugRenderer::Geometry> geometry = new DebugRenderer::Geometry(mDebugRenderer->CreateTriangleBatch(scalp_vertices, (uint)mHairSettings->mScalpVertices.size(), mHairSettings->mScalpTriangles.data(), (uint)mHairSettings->mScalpTriangles.size()), mHairSettings->mSimulationBounds);
		mDebugRenderer->DrawGeometry(neck_transform, Color::sGrey, geometry, DebugRenderer::ECullMode::CullBackFace, DebugRenderer::ECastShadow::On, DebugRenderer::EDrawMode::Solid);
	}

	// Select the next animation frame
	++mFrame;
	mFrame = mFrame % (uint32)mFaceAnimation.size();
	Array<Mat44> &joints = mFaceAnimation[mFrame];

	// Position the collision hulls
	for (const AttachedBody &ab : mAttachedBodies)
	{
		Mat44 body_transform = ab.mJointIdx != 0xffffffff? joints[ab.mJointIdx] : Mat44::sIdentity();
		bi.MoveKinematic(ab.mBodyID, RVec3(body_transform.GetTranslation()), body_transform.GetQuaternion(), inParams.mDeltaTime);
	}

	// Set the new rotation of the hair
	RVec3 position = RVec3(joints[mHeadJointIdx].GetTranslation());
	Quat rotation = joints[mHeadJointIdx].GetQuaternion();
	mHair->SetPosition(position);
	mHair->SetRotation(rotation);

	// Update the hair
	mHair->Update(inParams.mDeltaTime, joints[mHeadJointIdx].Inversed(), joints.data(), *mPhysicsSystem, mHairShaders, mComputeSystem, mComputeQueue);
	{
		JPH_PROFILE("Hair Compute");
		mComputeQueue->ExecuteAndWait();
	}
	mHair->ReadBackGPUState(mComputeQueue);
}

void HairTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mFrame);
}

void HairTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mFrame);
}

void HairTest::GradientSetting(DebugUI *inUI, UIElement *inSubMenu, const String &inName, float inMax, float inStep, HairSettings::Gradient &inStaticStorage, HairSettings::Gradient &inDynamicStorage)
{
	inUI->CreateTextButton(inSubMenu, inName, [inUI, inName, inMax, inStep, &inStaticStorage, &inDynamicStorage]() {
		UIElement *gradient_setting = inUI->CreateMenu();
		inUI->CreateSlider(gradient_setting, inName + " Min", inStaticStorage.mMin, 0.0f, inMax, inStep, [&inStaticStorage, &inDynamicStorage](float inValue) { inStaticStorage.mMin = inDynamicStorage.mMin = inValue; });
		inUI->CreateSlider(gradient_setting, inName + " Max", inStaticStorage.mMax, 0.0f, inMax, inStep, [&inStaticStorage, &inDynamicStorage](float inValue) { inStaticStorage.mMax = inDynamicStorage.mMax = inValue; });
		inUI->CreateSlider(gradient_setting, inName + " Min Fraction", inStaticStorage.mMinFraction, 0.0f, 1.0f, 0.01f, [&inStaticStorage, &inDynamicStorage](float inValue) { inStaticStorage.mMinFraction = inDynamicStorage.mMinFraction = min(inStaticStorage.mMaxFraction - 0.001f, inValue); });
		inUI->CreateSlider(gradient_setting, inName + " Max Fraction", inStaticStorage.mMaxFraction, 0.0f, 1.0f, 0.01f, [&inStaticStorage, &inDynamicStorage](float inValue) { inStaticStorage.mMaxFraction = inDynamicStorage.mMaxFraction = max(inStaticStorage.mMinFraction + 0.001f, inValue); });
		inUI->ShowMenu(gradient_setting);
	});
}

void HairTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Groom", [this, inUI]() {
		UIElement *groom_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sGrooms); ++i)
			inUI->CreateTextButton(groom_name, sGrooms[i].mName, [this, i]() { sSelectedGroom = &sGrooms[i]; RestartTest(); });
		inUI->ShowMenu(groom_name);
	});
	inUI->CreateCheckBox(inSubMenu, "Limit Max Strands", sLimitMaxStrands, [](UICheckBox::EState inState) { sLimitMaxStrands = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateSlider(inSubMenu, "Max Strands", float(sMaxStrands), 1.0f, 10000.0f, 1.0f, [](float inValue) { sMaxStrands = uint(inValue); });
	inUI->CreateSlider(inSubMenu, "Simulation Strands Percentage", float(sSimulationStrandsPercentage), 1.0f, 100.0f, 1.0f, [](float inValue) { sSimulationStrandsPercentage = inValue; });
	inUI->CreateSlider(inSubMenu, "Override Vertices Per Strand", float(sOverrideVerticesPerStrand), 1.0f, 64.0f, 1.0f, [](float inValue) { sOverrideVerticesPerStrand = uint(inValue); });
	inUI->CreateSlider(inSubMenu, "Num Solver Iterations Per Second", float(sNumSolverIterationsPerSecond), 1.0f, 960.0f, 1.0f, [settings = mHairSettings](float inValue) { sNumSolverIterationsPerSecond = uint(inValue); settings->mNumIterationsPerSecond = sNumSolverIterationsPerSecond; });
	GradientSetting(inUI, inSubMenu, "Hair Radius", 0.01f, 0.001f, sHairRadius, mHairSettings->mMaterials[0].mHairRadius);
	inUI->CreateCheckBox(inSubMenu, "Enable Collision", sEnableCollision, [settings = mHairSettings](UICheckBox::EState inState) { sEnableCollision = inState == UICheckBox::STATE_CHECKED; settings->mMaterials[0].mEnableCollision = sEnableCollision; });
	inUI->CreateCheckBox(inSubMenu, "Enable LRA", sEnableLRA, [settings = mHairSettings](UICheckBox::EState inState) { sEnableLRA = inState == UICheckBox::STATE_CHECKED; settings->mMaterials[0].mEnableLRA = sEnableLRA; });
	inUI->CreateSlider(inSubMenu, "Bend Compliance (10^x)", sBendComplianceExponent, -10.0f, 0.0f, 0.01f, [settings = mHairSettings](float inValue) { sBendComplianceExponent = inValue; settings->mMaterials[0].mBendCompliance = std::pow(10.0f, inValue); });
	inUI->CreateSlider(inSubMenu, "Stretch Compliance (10^x)", sStretchComplianceExponent, -10.0f, 0.0f, 0.01f, [settings = mHairSettings](float inValue) { sStretchComplianceExponent = inValue; settings->mMaterials[0].mStretchCompliance = std::pow(10.0f, inValue); });
	inUI->CreateSlider(inSubMenu, "Inertia Multiplier", sInertiaMultiplier, 1.0f, 100.0f, 0.1f, [settings = mHairSettings](float inValue) { sInertiaMultiplier = inValue; settings->mMaterials[0].mInertiaMultiplier = inValue; });
	inUI->CreateSlider(inSubMenu, "Linear Damping", sLinearDamping, 0.0f, 5.0f, 0.01f, [settings = mHairSettings](float inValue) { sLinearDamping = inValue; settings->mMaterials[0].mLinearDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Angular Damping", sAngularDamping, 0.0f, 5.0f, 0.01f, [settings = mHairSettings](float inValue) { sAngularDamping = inValue; settings->mMaterials[0].mAngularDamping = inValue; });
	inUI->CreateSlider(inSubMenu, "Friction", sFriction, 0.0f, 1.0f, 0.01f, [settings = mHairSettings](float inValue) { sFriction = inValue; settings->mMaterials[0].mFriction = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Linear Velocity", sMaxLinearVelocity, 0.01f, 10.0f, 0.01f, [settings = mHairSettings](float inValue) { sMaxLinearVelocity = inValue; settings->mMaterials[0].mMaxLinearVelocity = inValue; });
	inUI->CreateSlider(inSubMenu, "Max Angular Velocity", sMaxAngularVelocity, 0.01f, 50.0f, 0.01f, [settings = mHairSettings](float inValue) { sMaxAngularVelocity = inValue; settings->mMaterials[0].mMaxAngularVelocity = inValue; });
	GradientSetting(inUI, inSubMenu, "World Transform Influence", 1.0f, 0.01f, sWorldTransformInfluence, mHairSettings->mMaterials[0].mWorldTransformInfluence);
	GradientSetting(inUI, inSubMenu, "Gravity Factor", 1.0f, 0.01f, sGravityFactor, mHairSettings->mMaterials[0].mGravityFactor);
	inUI->CreateSlider(inSubMenu, "Gravity Preload Factor", sGravityPreloadFactor, 0.0f, 1.0f, 0.01f, [settings = mHairSettings](float inValue) { sGravityPreloadFactor = inValue; });
	GradientSetting(inUI, inSubMenu, "Grid Velocity Factor", 1.0f, 0.01f, sGridVelocityFactor, mHairSettings->mMaterials[0].mGridVelocityFactor);
	inUI->CreateSlider(inSubMenu, "Grid Density Force Factor", sGridDensityForceFactor, 0.0f, 10.0f, 0.1f, [settings = mHairSettings](float inValue) { sGridDensityForceFactor = inValue; settings->mMaterials[0].mGridDensityForceFactor = sGridDensityForceFactor; });
	GradientSetting(inUI, inSubMenu, "Global Pose", 1.0f, 0.001f, sGlobalPose, mHairSettings->mMaterials[0].mGlobalPose);
	GradientSetting(inUI, inSubMenu, "Skin Global Pose", 1.0f, 0.001f, sSkinGlobalPose, mHairSettings->mMaterials[0].mSkinGlobalPose);
#ifdef JPH_DEBUG_RENDERER
	if (mHairSettings->mSimStrands.size() > 1)
	{
		inUI->CreateSlider(inSubMenu, "Draw Simulation Strand Begin", (float)sDrawSimulationStrandBegin, 0.0f, float(mHairSettings->mSimStrands.size() - 1), 1.0f, [](float inValue) { sDrawSimulationStrandBegin = (uint)inValue; });
		inUI->CreateSlider(inSubMenu, "Draw Simulation Strand Count", (float)sDrawSimulationStrandCount, 1.0f, (float)mHairSettings->mSimStrands.size(), 1.0f, [](float inValue) { sDrawSimulationStrandCount = (uint)inValue; });
	}
	inUI->CreateCheckBox(inSubMenu, "Draw Rods", sDrawRods, [](UICheckBox::EState inState) { sDrawRods = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Unloaded Rods", sDrawUnloadedRods, [](UICheckBox::EState inState) { sDrawUnloadedRods = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Vertex Velocity", sDrawVertexVelocity, [](UICheckBox::EState inState) { sDrawVertexVelocity = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Angular Velocity", sDrawAngularVelocity, [](UICheckBox::EState inState) { sDrawAngularVelocity = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Rod Orientations", sDrawOrientations, [](UICheckBox::EState inState) { sDrawOrientations = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Neutral Density", sDrawNeutralDensity, [](UICheckBox::EState inState) { sDrawNeutralDensity = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Grid Density", sDrawGridDensity, [](UICheckBox::EState inState) { sDrawGridDensity = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Grid Velocity", sDrawGridVelocity, [](UICheckBox::EState inState) { sDrawGridVelocity = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Skin Points", sDrawSkinPoints, [](UICheckBox::EState inState) { sDrawSkinPoints = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Draw Render Strands", sDrawRenderStrands, [](UICheckBox::EState inState) { sDrawRenderStrands = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateComboBox(inSubMenu, "Render Strands Color", { "PerRenderStrand", "PerSimulatedStrand", "GravityFactor", "WorldInfluence", "GridVelocityFactor", "GlobalPose", "SkinGlobalPose" }, (int)sRenderStrandColor, [](int inItem) { sRenderStrandColor = (Hair::ERenderStrandColor)inItem; });
	inUI->CreateCheckBox(inSubMenu, "Draw Initial Gravity", sDrawInitialGravity, [](UICheckBox::EState inState) { sDrawInitialGravity = inState == UICheckBox::STATE_CHECKED; });
#endif // JPH_DEBUG_RENDERER
	inUI->CreateCheckBox(inSubMenu, "Draw Head Mesh", sDrawHeadMesh, [](UICheckBox::EState inState) { sDrawHeadMesh = inState == UICheckBox::STATE_CHECKED; });
}

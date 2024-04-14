// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodySkinnedConstraintTest.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyMotionProperties.h>
#include <Utils/SoftBodyCreator.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>
#include <Application/DebugUI.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodySkinnedConstraintTest)
{
	JPH_ADD_BASE_CLASS(SoftBodySkinnedConstraintTest, Test)
}

Array<Mat44> SoftBodySkinnedConstraintTest::GetWorldSpacePose(float inTime) const
{
	Array<Mat44> pose;
	pose.resize(cNumJoints);

	// Create local space pose
	pose[0] = Mat44::sTranslation(Vec3(0.0f, cBodyPosY, -0.5f * (cNumVerticesZ - 1) * cVertexSpacing));
	for (int i = 1; i < cNumJoints; ++i)
	{
		float amplitude = 0.25f * min(inTime, 2.0f); // Fade effect in over time
		Mat44 rotation = Mat44::sRotationX(amplitude * Sin(0.25f * JPH_PI * i + 2.0f * inTime));
		Mat44 translation = Mat44::sTranslation(Vec3(0, 0, (cNumVerticesZ - 1) * cVertexSpacing / (cNumJoints - 1)));
		pose[i] = rotation * translation;
	}

	// Convert to world space
	for (int i = 1; i < cNumJoints; ++i)
		pose[i] = pose[i - 1] * pose[i];

	return pose;
}

void SoftBodySkinnedConstraintTest::SkinVertices(bool inHardSkinAll)
{
	RMat44 com = mBody->GetCenterOfMassTransform();

	// Make pose relative to the center of mass of the body
	Array<Mat44> pose = GetWorldSpacePose(mTime);
	Mat44 offset = com.InversedRotationTranslation().ToMat44();
	for (Mat44 &m : pose)
		m = offset * m;

	SoftBodyMotionProperties *mp = static_cast<SoftBodyMotionProperties *>(mBody->GetMotionProperties());
	mp->SetEnableSkinConstraints(sEnableSkinConstraints);
	mp->SetSkinnedMaxDistanceMultiplier(sMaxDistanceMultiplier);
	if (sUpdateSkinning || inHardSkinAll)
		mp->SkinVertices(com, pose.data(), cNumJoints, inHardSkinAll, *mTempAllocator);
}

void SoftBodySkinnedConstraintTest::Initialize()
{
	CreateFloor();

	// Where we'll place the body
	RVec3 body_translation(0.0f, cBodyPosY, 0);

	// Make first and last row kinematic
	auto inv_mass = [](uint, uint inZ) { return inZ == 0 || inZ == cNumVerticesZ - 1? 0.0f : 1.0f; };
	Ref<SoftBodySharedSettings> settings = SoftBodyCreator::CreateCloth(cNumVerticesX, cNumVerticesZ, cVertexSpacing, inv_mass);

	// Make edges soft
	for (SoftBodySharedSettings::Edge &e : settings->mEdgeConstraints)
		e.mCompliance = 1.0e-3f;

	// Create inverse bind matrices by moving the bind pose to the center of mass space for the body
	Array<Mat44> bind_pose = GetWorldSpacePose(0.0f);
	Mat44 offset = Mat44::sTranslation(Vec3(-body_translation));
	for (Mat44 &m : bind_pose)
		m = offset * m;
	for (int i = 0; i < cNumJoints; ++i)
		settings->mInvBindMatrices.push_back(SoftBodySharedSettings::InvBind(i, bind_pose[i].Inversed()));

	// Create skinned vertices
	auto get_vertex = [](uint inX, uint inZ) { return inX + inZ * cNumVerticesX; };
	for (int z = 0; z < cNumVerticesZ; ++z)
		for (int x = 0; x < cNumVerticesX; ++x)
		{
			uint vertex_idx = get_vertex(x, z);
			SoftBodySharedSettings::Skinned skinned(vertex_idx, settings->mVertices[vertex_idx].mInvMass > 0.0f? 2.0f : 0.0f, 0.1f, 40.0f);

			// Find closest joints
			int closest_joint = -1, prev_closest_joint = -1;
			float closest_joint_dist = FLT_MAX, prev_closest_joint_dist = FLT_MAX;
			for (int i = 0; i < cNumJoints; ++i)
			{
				float dist = abs(settings->mVertices[vertex_idx].mPosition.z - bind_pose[i].GetTranslation().GetZ());
				if (dist < closest_joint_dist)
				{
					prev_closest_joint = closest_joint;
					prev_closest_joint_dist = closest_joint_dist;
					closest_joint = i;
					closest_joint_dist = dist;
				}
				else if (dist < prev_closest_joint_dist)
				{
					prev_closest_joint = i;
					prev_closest_joint_dist = dist;
				}
			}
			if (closest_joint_dist == 0.0f)
			{
				// Hard skin to closest joint
				skinned.mWeights[0] = SoftBodySharedSettings::SkinWeight(closest_joint, 1.0f);
			}
			else
			{
				// Skin to two closest joints
				skinned.mWeights[0] = SoftBodySharedSettings::SkinWeight(closest_joint, 1.0f / closest_joint_dist);
				skinned.mWeights[1] = SoftBodySharedSettings::SkinWeight(prev_closest_joint, 1.0f / prev_closest_joint_dist);
				skinned.NormalizeWeights();
			}

			settings->mSkinnedConstraints.push_back(skinned);
		}

	// Calculate the information needed for skinned constraints
	settings->CalculateSkinnedConstraintNormals();

	// Optimize the settings (note that this is the second time we call this, the first time was in SoftBodyCreator::CreateCloth,
	// this is a bit wasteful but we must do it because we added more constraints)
	settings->Optimize();

	// Create the body
	SoftBodyCreationSettings cloth(settings, body_translation, Quat::sIdentity(), Layers::MOVING);
	mBody = mBodyInterface->CreateSoftBody(cloth);
	mBodyInterface->AddBody(mBody->GetID(), EActivation::Activate);

	// Initially hard skin all vertices to the pose
	SkinVertices(true);
}

void SoftBodySkinnedConstraintTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Draw the pose pre step
	Array<Mat44> pose = GetWorldSpacePose(mTime);
	for (int i = 1; i < cNumJoints; ++i)
	{
		mDebugRenderer->DrawArrow(RVec3(pose[i - 1].GetTranslation()), RVec3(pose[i].GetTranslation()), Color::sGreen, 0.1f);
		mDebugRenderer->DrawCoordinateSystem(RMat44(pose[i]), 0.5f);
	}

	// Update time
	mTime += sTimeScale * inParams.mDeltaTime;

	// Calculate skinned vertices but do not hard skin them
	SkinVertices(false);
}

void SoftBodySkinnedConstraintTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void SoftBodySkinnedConstraintTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);
}

void SoftBodySkinnedConstraintTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateSlider(inSubMenu, "Time Scale", sTimeScale, 0.0f, 10.0f, 0.1f, [](float inValue) { sTimeScale = inValue; });
	inUI->CreateCheckBox(inSubMenu, "Update Skinning", sUpdateSkinning, [](UICheckBox::EState inState) { sUpdateSkinning = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateCheckBox(inSubMenu, "Enable Skin Constraints", sEnableSkinConstraints, [](UICheckBox::EState inState) { sEnableSkinConstraints = inState == UICheckBox::STATE_CHECKED; });
	inUI->CreateSlider(inSubMenu, "Max Distance Multiplier", sMaxDistanceMultiplier, 0.0f, 10.0f, 0.1f, [](float inValue) { sMaxDistanceMultiplier = inValue; });
}

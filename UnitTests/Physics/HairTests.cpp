// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#if defined(JPH_USE_DX12) || defined(JPH_USE_VK) || defined(JPH_USE_MTL) || defined(JPH_USE_CPU_COMPUTE)

#include "PhysicsTestContext.h"
#include "Layers.h"
#include "Compute/ComputeShaderLoader.h"
#include <Jolt/Physics/Hair/Hair.h>
#include <Jolt/Physics/Hair/HairShaders.h>
#include <Jolt/Physics/Hair/RegisterHair.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Compute/CPU/ComputeSystemCPU.h>
#include <Jolt/Shaders/HairWrapper.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>

TEST_SUITE("HairTests")
{
	using SVertex = HairSettings::SVertex;
	using SStrand = HairSettings::SStrand;
	using Material = HairSettings::Material;
	using Gradient = HairSettings::Gradient;

	// Add a straight strand starting at inRoot, the root is fixed
	static void sAddStrand(Array<SVertex> &ioVertices, Array<SStrand> &ioStrands, Vec3Arg inRoot, Vec3Arg inDelta, uint inNumVertices, uint32 inMaterialIndex = 0)
	{
		uint32 start = uint32(ioVertices.size());
		for (uint i = 0; i < inNumVertices; ++i)
		{
			Float3 pos;
			(inRoot + float(i) * inDelta).StoreFloat3(&pos);
			ioVertices.push_back(SVertex(pos, i == 0? 0.0f : 1.0f));
		}
		ioStrands.push_back(SStrand(start, start + inNumVertices, inMaterialIndex));
	}

	// A soft material that simulates all strands and doesn't pull them towards the modeled pose
	static Material sCreateMaterial()
	{
		Material m;
		m.mSimulationStrandsFraction = 1.0f;
		m.mGlobalPose = Gradient(0);
		m.mGravityFactor = Gradient(1);
		m.mBendCompliance = 1.0e-4f;
		return m;
	}

	// Create and initialize hair settings
	static Ref<HairSettings> sCreateSettings(const Array<SVertex> &inVertices, const Array<SStrand> &inStrands, const Array<Material> &inMaterials, const function<void(HairSettings &)> &inConfigure = { })
	{
		Ref<HairSettings> settings = new HairSettings;
		settings->mMaterials = inMaterials;
		if (inConfigure)
			inConfigure(*settings);
		settings->InitRenderAndSimulationStrands(inVertices, inStrands);
		float max_dist_sq = 0.0f;
		settings->Init(max_dist_sq);
		return settings;
	}

	// Create settings with a single horizontal strand along the X axis
	static Ref<HairSettings> sCreateHorizontalStrand(const Material &inMaterial, uint inNumVertices = 10, float inSpacing = 0.05f, const function<void(HairSettings &)> &inConfigure = { })
	{
		Array<SVertex> vertices;
		Array<SStrand> strands;
		sAddStrand(vertices, strands, Vec3::sZero(), Vec3(inSpacing, 0, 0), inNumVertices);
		return sCreateSettings(vertices, strands, { inMaterial }, inConfigure);
	}

	// Update the hair and read back the results
	static void sUpdate(Hair &ioHair, ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders, const PhysicsSystem &inSystem, float inDeltaTime, const Mat44 *inJointMatrices = nullptr)
	{
		ioHair.Update(inDeltaTime, Mat44::sIdentity(), inJointMatrices, inSystem, inShaders, inComputeSystem, inComputeQueue);
		ioHair.ReadBackGPUState(inComputeQueue);
	}

	// Simulate the hair for inTime seconds and read back the results
	static void sSimulate(Hair &ioHair, ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders, const PhysicsSystem &inSystem, float inTime, const Mat44 *inJointMatrices = nullptr)
	{
		constexpr float cDeltaTime = 1.0f / 60.0f;
		for (int i = 0, n = int(std::round(inTime / cDeltaTime)); i < n; ++i)
			ioHair.Update(cDeltaTime, Mat44::sIdentity(), inJointMatrices, inSystem, inShaders, inComputeSystem, inComputeQueue);
		ioHair.ReadBackGPUState(inComputeQueue);
	}

	// Get the position of a simulated vertex
	static Vec3 sGetPosition(const Hair &inHair, uint inIndex)
	{
		return Vec3(inHair.GetPositions()[inIndex]);
	}

	// Get the length of a simulated strand in its current pose
	static float sGetStrandLength(const Hair &inHair, const SStrand &inStrand)
	{
		float length = 0.0f;
		for (uint32 v = inStrand.mStartVtx + 1; v < inStrand.mEndVtx; ++v)
			length += (sGetPosition(inHair, v) - sGetPosition(inHair, v - 1)).Length();
		return length;
	}

	TEST_CASE("TestHairGradientAndMaterial")
	{
		// Sample a gradient
		HairSettings::GradientSampler sampler(Gradient(1, 3, 0.25f, 0.75f));
		CHECK_APPROX_EQUAL(sampler.Sample(0.0f), 1.0f);
		CHECK_APPROX_EQUAL(sampler.Sample(0.25f), 1.0f);
		CHECK_APPROX_EQUAL(sampler.Sample(0.5f), 2.0f);
		CHECK_APPROX_EQUAL(sampler.Sample(0.75f), 3.0f);
		CHECK_APPROX_EQUAL(sampler.Sample(1.0f), 3.0f);
		CHECK(sampler.ToFloat4() == Float4(4.0f, 0.0f, 1.0f, 3.0f));

		// Sample a decreasing gradient
		HairSettings::GradientSampler reverse_sampler(Gradient(3, 1));
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(-1.0f), 3.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(0.0f), 3.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(0.5f), 2.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(1.0f), 1.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(2.0f), 1.0f);

		// Sample along a strand
		SStrand strand(10, 15, 0);
		CHECK(strand.VertexCount() == 5);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(strand, 10), 3.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(strand, 12), 2.0f);
		CHECK_APPROX_EQUAL(reverse_sampler.Sample(strand, 14), 1.0f);

		// Making a gradient step dependent with a ratio of 1 should not change it
		Gradient g(0.1f, 0.5f, 0.2f, 0.8f);
		Gradient g1 = g.MakeStepDependent(1.0f);
		CHECK_APPROX_EQUAL(g1.mMin, 0.1f);
		CHECK_APPROX_EQUAL(g1.mMax, 0.5f);
		CHECK(g1.mMinFraction == 0.2f);
		CHECK(g1.mMaxFraction == 0.8f);

		// Taking 2 steps of 0.1 should result in 1 - 0.9^2
		Gradient g2 = g.MakeStepDependent(2.0f);
		CHECK_APPROX_EQUAL(g2.mMin, 1.0f - Square(0.9f));
		CHECK_APPROX_EQUAL(g2.mMax, 1.0f - Square(0.5f));

		// Bend compliance is interpolated between the 4 multipliers
		Material m;
		m.mBendCompliance = 2.0f;
		m.mBendComplianceMultiplier = { 1.0f, 3.0f, 5.0f, 7.0f };
		CHECK_APPROX_EQUAL(m.GetBendCompliance(0.0f), 2.0f);
		CHECK_APPROX_EQUAL(m.GetBendCompliance(1.0f / 6.0f), 4.0f);
		CHECK_APPROX_EQUAL(m.GetBendCompliance(1.0f / 3.0f), 6.0f, 1.0e-5f);
		CHECK_APPROX_EQUAL(m.GetBendCompliance(1.0f), 14.0f);

		// Default material needs the grid since the grid velocity factor is non zero
		CHECK(m.NeedsGrid());
		CHECK(!m.GlobalPoseOnly());
		m.mGridVelocityFactor = Gradient(0);
		CHECK(!m.NeedsGrid());
		m.mGridDensityForceFactor = 0.1f;
		CHECK(m.NeedsGrid());

		// A material without collision that fully follows the global pose doesn't need to be simulated
		m.mEnableCollision = false;
		CHECK(!m.GlobalPoseOnly());
		m.mGlobalPose = Gradient(1);
		CHECK(m.GlobalPoseOnly());
	}

	TEST_CASE("TestHairResample")
	{
		Array<SVertex> vertices = {
			// L shaped strand with length 2
			SVertex(Float3(0, 0, 0), 0.0f),
			SVertex(Float3(1, 0, 0), 1.0f),
			SVertex(Float3(1, 1, 0), 1.0f),

			// Straight strand with length 3
			SVertex(Float3(0, 0, 5), 0.0f),
			SVertex(Float3(0, -3, 5), 1.0f),
		};
		Array<SStrand> strands = { SStrand(0, 3, 0), SStrand(3, 5, 1) };

		HairSettings::sResample(vertices, strands, 5);

		CHECK(strands.size() == 2);
		CHECK(strands[0].mStartVtx == 0);
		CHECK(strands[0].mEndVtx == 5);
		CHECK(strands[0].mMaterialIndex == 0);
		CHECK(strands[1].mStartVtx == 5);
		CHECK(strands[1].mEndVtx == 10);
		CHECK(strands[1].mMaterialIndex == 1);
		CHECK(vertices.size() == 10);

		// Vertices should be evenly spaced along the strand
		const Vec3 expected[] = {
			Vec3(0, 0, 0), Vec3(0.5f, 0, 0), Vec3(1, 0, 0), Vec3(1, 0.5f, 0), Vec3(1, 1, 0),
			Vec3(0, 0, 5), Vec3(0, -0.75f, 5), Vec3(0, -1.5f, 5), Vec3(0, -2.25f, 5), Vec3(0, -3, 5)
		};
		const float expected_inv_mass[] = { 0, 1, 1, 1, 1, 0, 0, 1, 1, 1 }; // Inverse mass is interpolated and then rounded to 0 or 1
		for (uint i = 0; i < 10; ++i)
		{
			CHECK_APPROX_EQUAL(Vec3(vertices[i].mPosition), expected[i], 1.0e-5f);
			CHECK(vertices[i].mInvMass == expected_inv_mass[i]);
		}
		CHECK_APPROX_EQUAL(strands[0].MeasureLength(vertices), 2.0f, 1.0e-5f);
		CHECK_APPROX_EQUAL(strands[1].MeasureLength(vertices), 3.0f, 1.0e-5f);
	}

	TEST_CASE("TestHairInitRenderAndSimulationStrands")
	{
		Material m0 = sCreateMaterial();
		m0.mSimulationStrandsFraction = 0.5f;
		Material m1 = sCreateMaterial();
		m1.mSimulationStrandsFraction = 0.0f; // Should still result in 1 simulated strand

		// Create interleaved strands for 2 materials
		Array<SVertex> vertices;
		Array<SStrand> strands;
		sAddStrand(vertices, strands, Vec3(0, 0, 0), Vec3(0, -0.1f, 0), 5, 0);
		sAddStrand(vertices, strands, Vec3(0, 0, 1), Vec3(0, -0.1f, 0), 4, 1);
		sAddStrand(vertices, strands, Vec3(0.1f, 0, 0), Vec3(0, -0.1f, 0), 5, 0);
		sAddStrand(vertices, strands, Vec3(0.2f, 0, 0), Vec3(0, -0.1f, 0), 5, 0);
		sAddStrand(vertices, strands, Vec3(0.1f, 0, 1), Vec3(0, -0.1f, 0), 4, 1);
		sAddStrand(vertices, strands, Vec3(0.3f, 0, 0), Vec3(0, -0.1f, 0), 5, 0);

		Ref<HairSettings> settings = new HairSettings;
		settings->mMaterials = { m0, m1 };
		settings->InitRenderAndSimulationStrands(vertices, strands);

		// All strands should be rendered
		CHECK(settings->mRenderStrands.size() == strands.size());
		for (size_t i = 0; i < strands.size(); ++i)
		{
			CHECK(settings->mRenderStrands[i].mStartVtx == strands[i].mStartVtx);
			CHECK(settings->mRenderStrands[i].mEndVtx == strands[i].mEndVtx);
		}
		CHECK(settings->mRenderVertices.size() == vertices.size());
		for (size_t i = 0; i < vertices.size(); ++i)
			CHECK(settings->mRenderVertices[i].mPosition == vertices[i].mPosition);

		// Check the number of simulated strands per material
		uint num_simulated[2] = { 0, 0 };
		for (const SStrand &s : settings->mSimStrands)
			++num_simulated[s.mMaterialIndex];
		CHECK(num_simulated[0] == 2);
		CHECK(num_simulated[1] == 1);

		// Each simulated strand should be a copy of an input strand with the same material
		for (const SStrand &s : settings->mSimStrands)
		{
			bool found = false;
			for (const SStrand &in : strands)
				if (in.mMaterialIndex == s.mMaterialIndex
					&& in.VertexCount() == s.VertexCount()
					&& vertices[in.mStartVtx].mPosition == settings->mSimVertices[s.mStartVtx].mPosition)
				{
					found = true;
					for (uint32 v = 0; v < s.VertexCount(); ++v)
					{
						CHECK(settings->mSimVertices[s.mStartVtx + v].mPosition == vertices[in.mStartVtx + v].mPosition);
						CHECK(settings->mSimVertices[s.mStartVtx + v].mInvMass == vertices[in.mStartVtx + v].mInvMass);
					}
				}
			CHECK(found);
		}

		// Each render vertex should be attached to the closest vertex of a simulated strand with the same material
		for (const SStrand &in : strands)
			for (uint32 v = in.mStartVtx; v < in.mEndVtx; ++v)
			{
				uint32 sim_vtx = settings->mRenderVertices[v].mInfluences[0].mVertexIndex;
				CHECK(sim_vtx < settings->mSimVertices.size());
				const SStrand *sim_strand = nullptr;
				for (const SStrand &s : settings->mSimStrands)
					if (sim_vtx >= s.mStartVtx && sim_vtx < s.mEndVtx)
						sim_strand = &s;
				CHECK(sim_strand != nullptr);
				CHECK(sim_strand->mMaterialIndex == in.mMaterialIndex);
				CHECK(sim_vtx - sim_strand->mStartVtx == v - in.mStartVtx); // All strands are parallel, so the vertex at the same height is the closest
			}

		float max_dist_sq = -1.0f;
		settings->Init(max_dist_sq);
		CHECK(max_dist_sq == 0.0f); // No scalp
		for (const HairSettings::RVertex &rv : settings->mRenderVertices)
		{
			// Weights should be normalized
			float total_weight = 0.0f;
			for (uint i = 0; i < cHairNumSVertexInfluences; ++i)
				total_weight += rv.mInfluences[i].mWeight;
			CHECK_APPROX_EQUAL(total_weight, 1.0f);

			// The position relative to the simulated vertex should rotate back to the original offset
			const HairSettings::SVertexInfluence &inf = rv.mInfluences[0];
			const SVertex &sv = settings->mSimVertices[inf.mVertexIndex];
			CHECK_APPROX_EQUAL(Quat(sv.mBishop) * Vec3(inf.mRelativePosition), Vec3(rv.mPosition) - Vec3(sv.mPosition), 1.0e-5f);
		}
	}

	static void sTestHairSettingsObjectStreamRestoreSimulatesIdentically(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		Ref<HairSettings> settings = sCreateHorizontalStrand(sCreateMaterial());

		// Save and restore the settings
		Ref<HairSettings> restored;
		std::stringstream data;
		CHECK(ObjectStreamOut::sWriteObject(data, ObjectStreamOut::EStreamType::Text, *settings));
		CHECK(ObjectStreamIn::sReadObject(data, restored));
		float max_dist_sq = 0.0f;
		restored->Init(max_dist_sq);

		settings->InitCompute(inComputeSystem);
		restored->InitCompute(inComputeSystem);

		PhysicsTestContext c;
		Hair hair1(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair1.Init(inComputeSystem);
		Hair hair2(restored, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair2.Init(inComputeSystem);
		for (int i = 0; i < 30; ++i)
		{
			sUpdate(hair1, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
			sUpdate(hair2, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
		}
		for (uint v = 0; v < settings->mSimVertices.size(); ++v)
			CHECK(hair1.GetPositions()[v] == hair2.GetPositions()[v]);
	}

	static void sTestHairSettingsBinaryStateRestoreSimulatesIdentically(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		Ref<HairSettings> settings = sCreateHorizontalStrand(sCreateMaterial());

		// Save and restore the settings
		Ref<HairSettings> restored = new HairSettings;
		std::stringstream data;
		StreamOutWrapper stream_out(data);
		settings->SaveBinaryState(stream_out);
		StreamInWrapper stream_in(data);
		restored->RestoreBinaryState(stream_in);

		settings->InitCompute(inComputeSystem);
		restored->InitCompute(inComputeSystem);

		PhysicsTestContext c;
		Hair hair1(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair1.Init(inComputeSystem);
		Hair hair2(restored, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair2.Init(inComputeSystem);
		for (int i = 0; i < 30; ++i)
		{
			sUpdate(hair1, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
			sUpdate(hair2, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
		}
		for (uint v = 0; v < settings->mSimVertices.size(); ++v)
			CHECK(hair1.GetPositions()[v] == hair2.GetPositions()[v]);
	}

	static void sTestHairFallsUnderGravity(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		Ref<HairSettings> settings = sCreateHorizontalStrand(sCreateMaterial());
		settings->InitCompute(inComputeSystem);
		const SStrand &strand = settings->mSimStrands[0];
		float rest_length = strand.MeasureLength(settings->mSimVertices);

		PhysicsTestContext c;
		Hair hair(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair.Init(inComputeSystem);

		// The first update teleports the hair to its modeled pose
		sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 0.0f);
		for (uint v = strand.mStartVtx; v < strand.mEndVtx; ++v)
		{
			CHECK_APPROX_EQUAL(sGetPosition(hair, v), Vec3(settings->mSimVertices[v].mPosition), 1.0e-5f);
			CHECK(hair.GetVelocities()[v] == Float3(0, 0, 0));
			CHECK(hair.GetAngularVelocities()[v] == Float3(0, 0, 0));
		}

		sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);

		// The root is fixed
		CHECK(sGetPosition(hair, strand.mStartVtx) == Vec3::sZero());
		CHECK(hair.GetVelocities()[strand.mStartVtx] == Float3(0, 0, 0));

		// The tip should have dropped
		Vec3 tip = sGetPosition(hair, strand.mEndVtx - 1);
		CHECK(tip.GetY() < -0.2f);

		// The strand should not have stretched and should be ordered from root to tip
		CHECK_APPROX_EQUAL(sGetStrandLength(hair, strand), rest_length, 0.01f * rest_length);
		for (uint v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
		{
			CHECK(sGetPosition(hair, v).GetY() < sGetPosition(hair, v - 1).GetY());
			CHECK(hair.GetRotations()[v].IsNormalized());
		}

		// The render positions of a simulated strand are the simulated positions
		hair.LockReadBackBuffers();
		for (uint v = 0; v < settings->mRenderVertices.size(); ++v)
			CHECK_APPROX_EQUAL(Vec3(hair.GetRenderPositions()[v]), sGetPosition(hair, settings->mRenderVertices[v].mInfluences[0].mVertexIndex), 1.0e-5f);
		hair.UnlockReadBackBuffers();

		// Teleporting the hair should restore the modeled pose
		hair.OnTeleported();
		sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 0.0f);
		for (uint v = strand.mStartVtx; v < strand.mEndVtx; ++v)
		{
			CHECK_APPROX_EQUAL(sGetPosition(hair, v), Vec3(settings->mSimVertices[v].mPosition), 1.0e-5f);
			CHECK(hair.GetVelocities()[v] == Float3(0, 0, 0));
		}
	}

	static void sTestHairGlobalPoseOnly(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		// A material that fully follows the modeled pose
		Material m = sCreateMaterial();
		m.mEnableCollision = false;
		m.mGlobalPose = Gradient(1);
		Ref<HairSettings> settings = sCreateHorizontalStrand(m);
		settings->InitCompute(inComputeSystem);

		PhysicsTestContext c;
		Hair hair(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair.Init(inComputeSystem);

		// Move the hair, this should have no effect on the hair in local space
		hair.SetPosition(RVec3(1, 2, 3));
		hair.SetRotation(Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI));
		sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);

		// The hair should not have been affected by gravity
		for (uint v = 0; v < settings->mSimVertices.size(); ++v)
			CHECK_APPROX_EQUAL(sGetPosition(hair, v), Vec3(settings->mSimVertices[v].mPosition), 1.0e-5f);
	}

	static void sTestHairWorldTransformInfluence(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		for (float influence : { 0.0f, 1.0f })
		{
			// A material without gravity so that the hair stays in place
			Material m = sCreateMaterial();
			m.mGravityFactor =	Gradient(0);
			m.mWorldTransformInfluence = Gradient(influence);
			Ref<HairSettings> settings = sCreateHorizontalStrand(m);
			settings->InitCompute(inComputeSystem);
			const SStrand &strand = settings->mSimStrands[0];
			Vec3 rest_tip(settings->mSimVertices[strand.mEndVtx - 1].mPosition);

			PhysicsTestContext c;
			Hair hair(settings, RVec3(1, 0, 0), Quat::sIdentity(), Layers::MOVING);
			hair.Init(inComputeSystem);
			sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);

			// Without gravity, the hair stays in its modeled pose
			CHECK_APPROX_EQUAL(sGetPosition(hair, strand.mEndVtx - 1), rest_tip, 1.0e-5f);

			// Move the head perpendicular to the strand
			hair.SetPosition(RVec3(1, 0, 0.1f));
			CHECK(hair.GetWorldTransform() == RMat44::sTranslation(RVec3(1, 0, 0.1f)));
			sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);

			Vec3 tip = sGetPosition(hair, strand.mEndVtx - 1);
			if (influence == 0.0f)
			{
				// Hair moves along with the head
				CHECK_APPROX_EQUAL(tip, rest_tip, 1.0e-5f);
			}
			else
			{
				// Hair lags behind in local space
				CHECK(tip.GetZ() < -0.01f);
			}
		}
	}

	static void sTestHairMaxDeltaTime(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		Ref<HairSettings> settings = sCreateHorizontalStrand(sCreateMaterial());
		settings->InitCompute(inComputeSystem);

		PhysicsTestContext c;
		Hair hair1(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair1.Init(inComputeSystem);
		Hair hair2(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair2.Init(inComputeSystem);

		// A large time step should be clamped to the max delta time
		sUpdate(hair1, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);
		sUpdate(hair2, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), settings->mMaxDeltaTime);
		Vec3 tip1 = sGetPosition(hair1, settings->mSimStrands[0].mEndVtx - 1);
		Vec3 tip2 = sGetPosition(hair2, settings->mSimStrands[0].mEndVtx - 1);
		CHECK(tip1.GetY() < 0.0f);
		CHECK_APPROX_EQUAL(tip1, tip2, 1.0e-6f);
	}

	static void sTestHairGrid(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		Material m = sCreateMaterial();
		m.mGridVelocityFactor = Gradient(0.05f);
		m.mGridDensityForceFactor = 0.1f;
		Array<SVertex> vertices;
		Array<SStrand> strands;
		for (int i = 0; i < 3; ++i)
			sAddStrand(vertices, strands, Vec3(0, 0, 0.1f * i), Vec3(0.05f, 0, 0), 10);
		Ref<HairSettings> settings = sCreateSettings(vertices, strands, { m });
		settings->InitCompute(inComputeSystem);

		PhysicsTestContext c;
		Hair hair(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair.Init(inComputeSystem);
		sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);

		// The density in the grid should add up to the number of non fixed vertices
		hair.LockReadBackBuffers();
		float total_density = 0.0f;
		const Float4 *grid = hair.GetGridVelocityAndDensity();
		for (size_t i = 0; i < settings->mNeutralDensity.size(); ++i)
		{
			CHECK(grid[i].w >= 0.0f);
			total_density += grid[i].w;
		}
		hair.UnlockReadBackBuffers();
		CHECK_APPROX_EQUAL(total_density, 27.0f);

		// Simulate and check that the simulation remains stable
		sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);
		for (const SStrand &s : settings->mSimStrands)
		{
			Vec3 tip = sGetPosition(hair, s.mEndVtx - 1);
			CHECK(!tip.IsNaN());
			CHECK(tip.GetY() < -0.1f);
			float rest_length = s.MeasureLength(settings->mSimVertices);
			CHECK_APPROX_EQUAL(sGetStrandLength(hair, s), rest_length, 0.01f * rest_length);
		}
	}

	static void sTestHairCollision(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		enum class EType
		{
			ConvexHull,
			InsideOutConvexHull,
			CompoundWithSphere,
			Sensor,
			CollisionDisabled,
			SphereOnly,
		};

		for (EType type : { EType::ConvexHull, EType::InsideOutConvexHull, EType::CompoundWithSphere, EType::Sensor, EType::CollisionDisabled, EType::SphereOnly })
		{
			INFO("Type: ", int(type));

			Material m = sCreateMaterial();
			m.mHairRadius = Gradient(0.01f);
			m.mEnableCollision = type != EType::CollisionDisabled;
			Ref<HairSettings> settings = sCreateHorizontalStrand(m, 10, 0.05f, [](HairSettings &ioSettings) { ioSettings.mSimulationBoundsPadding = Vec3::sReplicate(0.5f); });
			settings->InitCompute(inComputeSystem);
			const SStrand &strand = settings->mSimStrands[0];

			// Create a floor below the hair
			constexpr float cFloorTop = -0.05f;
			PhysicsTestContext c;
			ConvexHullShapeSettings hull;
			for (float x : { -0.5f, 0.5f })
				for (float y : { -0.05f, 0.05f })
					for (float z : { -0.5f, 0.5f })
						hull.mPoints.push_back(Vec3(x, y, z));
			hull.SetEmbedded();
			RVec3 floor_position(0.5f, cFloorTop - 0.05f, 0);
			switch (type)
			{
			case EType::ConvexHull:
			case EType::CollisionDisabled:
				c.CreateBody(&hull, floor_position, Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
				break;

			case EType::InsideOutConvexHull:
				{
					ScaledShapeSettings scaled(&hull, Vec3(-1, 1, 1));
					scaled.SetEmbedded();
					c.CreateBody(&scaled, floor_position, Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
				}
				break;

			case EType::CompoundWithSphere:
				{
					// Add a sphere that is within the simulation bounds but doesn't touch the hair, it should be ignored as it is not a convex hull
					SphereShapeSettings sphere(1.0f);
					sphere.SetEmbedded();
					StaticCompoundShapeSettings compound;
					compound.SetEmbedded();
					compound.AddShape(Vec3::sZero(), Quat::sIdentity(), &hull);
					compound.AddShape(Vec3::sZero(), Quat::sIdentity(), &sphere);
					c.CreateBody(&compound, floor_position, Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
				}
				break;

			case EType::SphereOnly:
				{
					// Only convex hulls are supported, so the hair should not collide with this sphere
					SphereShapeSettings sphere(1.0f);
					sphere.SetEmbedded();
					c.CreateBody(&sphere, floor_position, Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
				}
				break;

			case EType::Sensor:
				{
					BodyCreationSettings bcs(&hull, floor_position, Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
					bcs.mIsSensor = true;
					c.CreateBody(bcs, EActivation::DontActivate);
				}
				break;
			}

			Hair hair(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
			hair.Init(inComputeSystem);
			sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f);

			if (type == EType::ConvexHull)
			{
				// Add a 2nd floor next to the first one so that the collision buffers need to grow
				c.CreateBody(&hull, floor_position + RVec3(0, 0, 0.1f), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
				sSimulate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 0.5f);
			}

			float min_y = FLT_MAX;
			for (uint v = strand.mStartVtx; v < strand.mEndVtx; ++v)
				min_y = min(min_y, sGetPosition(hair, v).GetY());

			if (type == EType::Sensor || type == EType::CollisionDisabled || type == EType::SphereOnly)
			{
				// The hair should fall through the floor
				CHECK(min_y < cFloorTop - 0.05f);
			}
			else
			{
				// The hair should rest on the floor
				CHECK_APPROX_EQUAL(min_y, cFloorTop + m.mHairRadius.mMin);
			}
		}
	}

	static void sTestHairRenderStrands(ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue, const HairShaders &inShaders)
	{
		// Two parallel strands, one of which is simulated
		Material m = sCreateMaterial();
		m.mSimulationStrandsFraction = 0.5f;
		Array<SVertex> vertices;
		Array<SStrand> strands;
		sAddStrand(vertices, strands, Vec3(0, 0, 0), Vec3(0.05f, 0, 0), 10);
		sAddStrand(vertices, strands, Vec3(0, 0, 0.02f), Vec3(0.05f, 0, 0), 10);
		Ref<HairSettings> settings = sCreateSettings(vertices, strands, { m });
		CHECK(settings->mSimStrands.size() == 1);
		settings->InitCompute(inComputeSystem);
		uint num_render_vertices = uint(settings->mRenderVertices.size());

		PhysicsTestContext c;
		Hair hair(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair.Init(inComputeSystem);
		CHECK(hair.GetRenderPositionsCB() != nullptr);

		// Create a 2nd hair that uses an externally provided render positions buffer
		Hair hair_override(settings, RVec3::sZero(), Quat::sIdentity(), Layers::MOVING);
		hair_override.OverrideRenderPositionsCB([](ComputeBuffer *inBuffer, Float3 *outPositions, uint inNumVertices) {
			const Float3 *positions = inBuffer->Map<Float3>(ComputeBuffer::EMode::Read);
			for (uint i = 0; i < inNumVertices; ++i)
				outPositions[i] = positions[i];
			inBuffer->Unmap();
		});
		Ref<ComputeBuffer> render_positions_cb = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_render_vertices, sizeof(Float3)).Get();
		hair_override.SetRenderPositionsCB(render_positions_cb);
		hair_override.Init(inComputeSystem);
		CHECK(hair_override.GetRenderPositionsCB() == render_positions_cb.GetPtr());

		auto check_render_positions = [&settings, num_render_vertices](Hair &ioHair, Hair &ioHairOverride, bool inModeledPose) {
			ioHair.LockReadBackBuffers();
			ioHairOverride.LockReadBackBuffers();
			const Float3 *render_positions = ioHair.GetRenderPositions();
			for (uint v = 0; v < num_render_vertices; ++v)
			{
				const HairSettings::RVertex &rv = settings->mRenderVertices[v];
				Vec3 render_pos(render_positions[v]);
				if (inModeledPose)
				{
					// In the modeled pose, the render positions match the input positions
					CHECK_APPROX_EQUAL(render_pos, Vec3(rv.mPosition), 1.0e-5f);
				}
				else
				{
					// Render vertices should stay at the same distance from the simulated vertex they're attached to
					uint32 sim_vtx = rv.mInfluences[0].mVertexIndex;
					float initial_distance = (Vec3(rv.mPosition) - Vec3(settings->mSimVertices[sim_vtx].mPosition)).Length();
					CHECK_APPROX_EQUAL((render_pos - sGetPosition(ioHair, sim_vtx)).Length(), initial_distance, 1.0e-4f);
				}
				CHECK(ioHairOverride.GetRenderPositions()[v] == render_positions[v]);
			}
			ioHairOverride.UnlockReadBackBuffers();
			ioHair.UnlockReadBackBuffers();
		};

		sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 0.0f);
		sUpdate(hair_override, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 0.0f);
		check_render_positions(hair, hair_override, true);

		for (int i = 0; i < 60; ++i)
		{
			sUpdate(hair, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
			sUpdate(hair_override, inComputeSystem, inComputeQueue, inShaders, *c.GetSystem(), 1.0f / 60.0f);
		}
		CHECK(sGetPosition(hair, settings->mSimStrands[0].mEndVtx - 1).GetY() < -0.1f);
		check_render_positions(hair, hair_override, false);
	}

	// Runs all hair tests that need a compute system
	static void RunTests(ComputeSystem *inComputeSystem)
	{
		ComputeQueueResult queue = inComputeSystem->CreateComputeQueue();
		CHECK(!queue.HasError());
		if (queue.HasError())
			return;

		Ref<HairShaders> shaders = new HairShaders;
		shaders->Init(inComputeSystem);
		bool all_valid = shaders->AllValid();
		CHECK(all_valid);
		if (!all_valid)
			return;

		sTestHairSettingsObjectStreamRestoreSimulatesIdentically(inComputeSystem, queue.Get(), *shaders);
		sTestHairSettingsBinaryStateRestoreSimulatesIdentically(inComputeSystem, queue.Get(), *shaders);
		sTestHairFallsUnderGravity(inComputeSystem, queue.Get(), *shaders);
		sTestHairGlobalPoseOnly(inComputeSystem, queue.Get(), *shaders);
		sTestHairWorldTransformInfluence(inComputeSystem, queue.Get(), *shaders);
		sTestHairMaxDeltaTime(inComputeSystem, queue.Get(), *shaders);
		sTestHairGrid(inComputeSystem, queue.Get(), *shaders);
		sTestHairCollision(inComputeSystem, queue.Get(), *shaders);
		sTestHairRenderStrands(inComputeSystem, queue.Get(), *shaders);
	}

#ifdef JPH_USE_CPU_COMPUTE
	TEST_CASE("TestHairCPU")
	{
		RegisterHair();

		ComputeSystemResult compute_system = CreateComputeSystemCPU();
		CHECK(!compute_system.HasError());
		if (!compute_system.HasError())
		{
			HairRegisterShaders(StaticCast<ComputeSystemCPU>(compute_system.Get()));
			RunTests(compute_system.Get());
		}
	}
#endif // JPH_USE_CPU_COMPUTE

	TEST_CASE("TestHairGPU")
	{
		RegisterHair();

		// The GPU is optional, skip if it's not available
		ComputeSystemResult compute_system = CreateComputeSystem();
		if (!compute_system.HasError())
		{
			SetComputeShaderLoader(compute_system.Get());
			RunTests(compute_system.Get());
		}
	}
}

#endif // defined(JPH_USE_DX12) || defined(JPH_USE_VK) || defined(JPH_USE_MTL) || defined(JPH_USE_CPU_COMPUTE)

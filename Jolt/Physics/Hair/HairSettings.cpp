// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Hair/HairSettings.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Geometry/ClosestPoint.h>
#include <Jolt/TriangleSplitter/TriangleSplitterBinning.h>
#include <Jolt/AABBTree/AABBTreeBuilder.h>
#include <Jolt/Core/QuickSort.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings)
{
	JPH_ADD_ATTRIBUTE(HairSettings, mSimVertices)
	JPH_ADD_ATTRIBUTE(HairSettings, mSimStrands)
	JPH_ADD_ATTRIBUTE(HairSettings, mRenderVertices)
	JPH_ADD_ATTRIBUTE(HairSettings, mRenderStrands)
	JPH_ADD_ATTRIBUTE(HairSettings, mScalpVertices)
	JPH_ADD_ATTRIBUTE(HairSettings, mScalpTriangles)
	JPH_ADD_ATTRIBUTE(HairSettings, mScalpInverseBindPose)
	JPH_ADD_ATTRIBUTE(HairSettings, mScalpSkinWeights)
	JPH_ADD_ATTRIBUTE(HairSettings, mScalpNumSkinWeightsPerVertex)
	JPH_ADD_ATTRIBUTE(HairSettings, mNumIterationsPerSecond)
	JPH_ADD_ATTRIBUTE(HairSettings, mMaxDeltaTime)
	JPH_ADD_ATTRIBUTE(HairSettings, mGridSize)
	JPH_ADD_ATTRIBUTE(HairSettings, mSimulationBoundsPadding)
	JPH_ADD_ATTRIBUTE(HairSettings, mInitialGravity)
	JPH_ADD_ATTRIBUTE(HairSettings, mMaterials)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::SkinWeight)
{
	JPH_ADD_ATTRIBUTE(HairSettings::SkinWeight, mJointIdx)
	JPH_ADD_ATTRIBUTE(HairSettings::SkinWeight, mWeight)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::SkinPoint)
{
	JPH_ADD_ATTRIBUTE(HairSettings::SkinPoint, mTriangleIndex)
	JPH_ADD_ATTRIBUTE(HairSettings::SkinPoint, mU)
	JPH_ADD_ATTRIBUTE(HairSettings::SkinPoint, mV)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::SVertexInfluence)
{
	JPH_ADD_ATTRIBUTE(HairSettings::SVertexInfluence, mVertexIndex)
	JPH_ADD_ATTRIBUTE(HairSettings::SVertexInfluence, mRelativePosition)
	JPH_ADD_ATTRIBUTE(HairSettings::SVertexInfluence, mWeight)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::RVertex)
{
	JPH_ADD_ATTRIBUTE(HairSettings::RVertex, mPosition)
	JPH_ADD_ATTRIBUTE(HairSettings::RVertex, mInfluences)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::SVertex)
{
	JPH_ADD_ATTRIBUTE(HairSettings::SVertex, mPosition)
	JPH_ADD_ATTRIBUTE(HairSettings::SVertex, mInvMass)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::RStrand)
{
	JPH_ADD_ATTRIBUTE(HairSettings::RStrand, mStartVtx)
	JPH_ADD_ATTRIBUTE(HairSettings::RStrand, mEndVtx)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::SStrand)
{
	JPH_ADD_BASE_CLASS(HairSettings::SStrand, HairSettings::RStrand)

		JPH_ADD_ATTRIBUTE(HairSettings::SStrand, mMaterialIndex)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::Gradient)
{
	JPH_ADD_ATTRIBUTE(HairSettings::Gradient, mMin)
	JPH_ADD_ATTRIBUTE(HairSettings::Gradient, mMax)
	JPH_ADD_ATTRIBUTE(HairSettings::Gradient, mMinFraction)
	JPH_ADD_ATTRIBUTE(HairSettings::Gradient, mMaxFraction)
}

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(HairSettings::Material)
{
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mEnableCollision)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mEnableLRA)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mLinearDamping)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mAngularDamping)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mMaxLinearVelocity)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mMaxAngularVelocity)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mGravityFactor)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mFriction)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mBendCompliance)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mBendComplianceMultiplier)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mStretchCompliance)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mInertiaMultiplier)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mHairRadius)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mWorldTransformInfluence)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mGridVelocityFactor)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mGridDensityForceFactor)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mGlobalPose)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mSkinGlobalPose)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mSimulationStrandsFraction)
	JPH_ADD_ATTRIBUTE(HairSettings::Material, mGravityPreloadFactor)
}

void HairSettings::Gradient::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mMin);
	inStream.Write(mMax);
	inStream.Write(mMinFraction);
	inStream.Write(mMaxFraction);
}

void HairSettings::Gradient::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mMin);
	inStream.Read(mMax);
	inStream.Read(mMinFraction);
	inStream.Read(mMaxFraction);
}

void HairSettings::InitRenderAndSimulationStrands(const Array<SVertex> &inVertices, const Array<SStrand> &inStrands)
{
	// Copy original strands to render strands
	mRenderVertices.resize(inVertices.size());
	for (uint i = 0, n = uint(inVertices.size()); i < n; ++i)
		mRenderVertices[i].mPosition = inVertices[i].mPosition;
	mRenderStrands.resize(inStrands.size());
	for (uint i = 0, n = uint(inStrands.size()); i < n; ++i)
		mRenderStrands[i] = RStrand(inStrands[i].mStartVtx, inStrands[i].mEndVtx);

	// Create buffer that holds indices to the strands
	Array<uint> indices_shuffle;
	indices_shuffle.resize(inStrands.size());
	for (uint i = 0, n = uint(inStrands.size()); i < n; ++i)
		indices_shuffle[i] = i;

	// Order on material index
	QuickSort(indices_shuffle.begin(), indices_shuffle.end(), [&inStrands](uint inLHS, uint inRHS) {
		return inStrands[inLHS].mMaterialIndex < inStrands[inRHS].mMaterialIndex;
	});

	// Loop over all materials
	Array<uint>::iterator begin_material = indices_shuffle.begin();
	while (begin_material < indices_shuffle.end())
	{
		uint32 material_index = inStrands[*begin_material].mMaterialIndex;

		// Find end of this material
		Array<uint>::iterator end_material = begin_material;
		do
			++end_material;
		while (end_material < indices_shuffle.end() && inStrands[*end_material].mMaterialIndex == material_index);

		// Select X% random strands to simulate
		std::mt19937 random;
		std::shuffle(begin_material, end_material, random);
		size_t num_simulated = max<size_t>(size_t(ceil(double(mMaterials[material_index].mSimulationStrandsFraction) * double(end_material - begin_material))), 1);
		Array<uint>::iterator end_simulation = begin_material + num_simulated;
		QuickSort(begin_material, end_simulation, std::less<uint>()); // Sort simulated strands back to original order
		for (Array<uint>::const_iterator idx = begin_material; idx < end_simulation; ++idx)
		{
			// Add simulation strand
			const HairSettings::SStrand &sim_strand = inStrands[*idx];
			mSimStrands.push_back(HairSettings::SStrand((uint32)mSimVertices.size(), (uint32)mSimVertices.size() + sim_strand.VertexCount(), sim_strand.mMaterialIndex));

			for (uint32 v = sim_strand.mStartVtx; v < sim_strand.mEndVtx; ++v)
			{
				// Link render vertex to simulation vertex
				mRenderVertices[v].mInfluences[0].mVertexIndex = uint32(mSimVertices.size());

				// Add simulation vertex
				mSimVertices.push_back(inVertices[v]);
			}
		}

		// Get influences for remaining strands
		for (Array<uint>::const_iterator idx = end_simulation; idx < end_material; ++idx)
		{
			const HairSettings::SStrand &render_strand = inStrands[*idx];

			// Find closest simulation strand
			float closest_d_sq = FLT_MAX;
			uint closest_strand_idx = 0;
			for (const HairSettings::SStrand &sim_strand : mSimStrands)
				if (sim_strand.mMaterialIndex == render_strand.mMaterialIndex)
				{
					// Get the first 2 vertices of the simulation strand
					uint32 v_max = sim_strand.mEndVtx - 1;
					uint32 v = sim_strand.mStartVtx, v_next = min(v + 1, v_max);
					Vec3 v_pos(mSimVertices[v].mPosition), v_next_pos(mSimVertices[v_next].mPosition);

					// Track total error when selecting this sim strand as parent for the render strand
					float d_sq_total = 0.0f;

					// Loop over the render strand
					for (uint32 rv = render_strand.mStartVtx; rv < render_strand.mEndVtx; ++rv)
					{
						Vec3 rv_pos(mRenderVertices[rv].mPosition);

						// Find closest simulated vertex (note that we assume that the strands do not loop back
						// on themselves so that an earlier vertex in the strand could be the closest)
						float d_sq = (rv_pos - v_pos).LengthSq();
						float d_sq_next = (rv_pos - v_next_pos).LengthSq();
						while (d_sq_next < d_sq)
						{
							// Get the next vertex of the simulation strand
							v = v_next;
							v_next = min(v + 1, v_max);
							v_pos = v_next_pos;
							v_next_pos = Vec3(mSimVertices[v_next].mPosition);

							// Update distance to render vertex
							d_sq = d_sq_next;
							d_sq_next = (rv_pos - v_next_pos).LengthSq();
						}

						// Accumulate total error
						d_sq_total += d_sq;

						// No point in continuing the search if our result is worse already
						if (d_sq_total > closest_d_sq)
							break;
					}

					// If this is the smallest error, accept
					if (d_sq_total < closest_d_sq)
					{
						closest_d_sq = d_sq_total;
						closest_strand_idx = uint(&sim_strand - mSimStrands.data());
					}
				}
			const HairSettings::SStrand &closest_strand = mSimStrands[closest_strand_idx];

			// Link render vertices to simulation vertices
			for (uint32 v = render_strand.mStartVtx; v < render_strand.mEndVtx; ++v)
			{
				HairSettings::RVertex &rv = mRenderVertices[v];

				// Find closest simulated vertex
				closest_d_sq = FLT_MAX;
				for (uint32 cv = closest_strand.mStartVtx; cv < closest_strand.mEndVtx; ++cv)
				{
					float d_sq = (Vec3(mSimVertices[cv].mPosition) - Vec3(rv.mPosition)).LengthSq();
					if (d_sq < closest_d_sq)
					{
						closest_d_sq = d_sq;
						rv.mInfluences[0].mVertexIndex = cv;
					}
				}
			}
		}

		// Next material
		begin_material = end_material;
	}
}

void HairSettings::sResample(Array<SVertex> &ioVertices, Array<SStrand> &ioStrands, uint32 inNumVerticesPerStrand)
{
	Array<SVertex> vertices;
	ioVertices.swap(vertices);
	Array<SStrand> strands;
	ioStrands.swap(strands);

	for (const SStrand &strand : strands)
	{
		// Determine output strand
		SStrand out_strand;
		out_strand.mStartVtx = (uint32)ioVertices.size();
		out_strand.mEndVtx = out_strand.mStartVtx + inNumVerticesPerStrand;
		out_strand.mMaterialIndex = strand.mMaterialIndex;
		ioStrands.push_back(out_strand);

		// Measure length of the strand
		float length = strand.MeasureLength(vertices);

		// Add the first vertex of the strand
		ioVertices.push_back(vertices[strand.mStartVtx]);

		// Resample the strand
		float cur_length = 0.0f;
		const SVertex *v0 = &vertices[strand.mStartVtx];
		const SVertex *v1 = &vertices[strand.mStartVtx + 1];
		float segment_length = (Vec3(v1->mPosition) - Vec3(v0->mPosition)).Length();
		for (uint32 resampled_point = 1; resampled_point < inNumVerticesPerStrand - 1; ++resampled_point)
		{
			float desired_len = resampled_point * length / (inNumVerticesPerStrand - 1);

			while (cur_length + segment_length < desired_len)
			{
				cur_length += segment_length;
				++v0;
				++v1;
				JPH_ASSERT(uint32(v1 - vertices.data()) < strand.mEndVtx);
				segment_length = (Vec3(v1->mPosition) - Vec3(v0->mPosition)).Length();
			}

			SVertex out_v = *v0;
			float fraction = (desired_len - cur_length) / segment_length;
			(Vec3(v0->mPosition) + (Vec3(v1->mPosition) - Vec3(v0->mPosition)) * fraction).StoreFloat3(&out_v.mPosition);
			out_v.mInvMass = v0->mInvMass + (v1->mInvMass - v0->mInvMass) * fraction < 0.5f? 0.0f : 1.0f;
			ioVertices.push_back(out_v);
		}

		// Add the last vertex of the strand
		ioVertices.push_back(vertices[strand.mEndVtx - 1]);

		JPH_ASSERT(uint32(ioVertices.size()) == out_strand.mEndVtx);
	}
}

static void sHairSettingsFindClosestTriangle(Vec3Arg inPoint, const AABBTreeBuilder &inBuilder, const AABBTreeBuilder::Node *inNode, Array<Float3> &inScalpVertices, float &ioClosestDistSq, HairSettings::SkinPoint &outSkinPoint)
{
	if (inNode->HasChildren())
	{
		// Get children
		const AABBTreeBuilder::Node *child0 = inNode->GetChild(0, inBuilder.GetNodes());
		const AABBTreeBuilder::Node *child1 = inNode->GetChild(1, inBuilder.GetNodes());

		// Order so that the first one is closest
		float dist_sq0 = child0 != nullptr? child0->mBounds.GetSqDistanceTo(inPoint) : FLT_MAX;
		float dist_sq1 = child1 != nullptr? child1->mBounds.GetSqDistanceTo(inPoint) : FLT_MAX;
		if (dist_sq1 < dist_sq0)
		{
			std::swap(child0, child1);
			std::swap(dist_sq0, dist_sq1);
		}

		// Visit in order of closeness
		if (dist_sq0 < ioClosestDistSq)
			sHairSettingsFindClosestTriangle(inPoint, inBuilder, child0, inScalpVertices, ioClosestDistSq, outSkinPoint);
		if (dist_sq1 < ioClosestDistSq)
			sHairSettingsFindClosestTriangle(inPoint, inBuilder, child1, inScalpVertices, ioClosestDistSq, outSkinPoint);
	}
	else
	{
		// Loop over the triangles
		for (const IndexedTriangle *t = inBuilder.GetTriangles().data() + inNode->mTrianglesBegin, *t_end = t + inNode->mNumTriangles; t < t_end; ++t)
		{
			Vec3 v0 = Vec3(inScalpVertices[t->mIdx[0]]) - inPoint;
			Vec3 v1 = Vec3(inScalpVertices[t->mIdx[1]]) - inPoint;
			Vec3 v2 = Vec3(inScalpVertices[t->mIdx[2]]) - inPoint;

			// Check if it is the closest triangle
			uint32 set;
			Vec3 closest_point = ClosestPoint::GetClosestPointOnTriangle(v0, v1, v2, set);
			float dist_sq = closest_point.LengthSq();
			if (dist_sq < ioClosestDistSq)
			{
				ioClosestDistSq = dist_sq;
				outSkinPoint.mTriangleIndex = t->mMaterialIndex;

				// Get barycentric coordinates of attachment point
				float w;
				ClosestPoint::GetBaryCentricCoordinates(v0, v1, v2, outSkinPoint.mU, outSkinPoint.mV, w);
			}
		}
	}
}

void HairSettings::Init(float &outMaxDistSqHairToScalp)
{
	outMaxDistSqHairToScalp = 0.0f;

	if (!mScalpTriangles.empty())
	{
		// Build a tree for all scalp triangles
		IndexedTriangleList triangles;
		triangles.reserve(mScalpTriangles.size());
		for (const IndexedTriangleNoMaterial &t : mScalpTriangles)
			triangles.push_back(IndexedTriangle(t.mIdx[0], t.mIdx[1], t.mIdx[2], uint32(&t - mScalpTriangles.data())));
		TriangleSplitterBinning splitter(mScalpVertices, triangles);
		AABBTreeBuilder builder(splitter, 8);
		AABBTreeBuilderStats builder_stats;
		const AABBTreeBuilder::Node *root = builder.Build(builder_stats);

		mSkinPoints.reserve(mSimStrands.size());
		for (const SStrand &strand : mSimStrands)
		{
			SkinPoint sp;
			sp.mTriangleIndex = 0;
			sp.mU = 0.0f;
			sp.mV = 0.0f;

			// Get root position
			Vec3 p = Vec3(mSimVertices[strand.mStartVtx].mPosition);

			// Find closest triangle on scalp
			float closest_dist_sq = FLT_MAX;
			sHairSettingsFindClosestTriangle(p, builder, root, mScalpVertices, closest_dist_sq, sp);
			outMaxDistSqHairToScalp = max(outMaxDistSqHairToScalp, closest_dist_sq);

			// Project root to the triangle as we will during simulation.
			// This ensures that we calculate the Bishop frame for the root correctly.
			const IndexedTriangleNoMaterial &t = mScalpTriangles[sp.mTriangleIndex];
			Vec3 v0 = Vec3(mScalpVertices[t.mIdx[0]]);
			Vec3 v1 = Vec3(mScalpVertices[t.mIdx[1]]);
			Vec3 v2 = Vec3(mScalpVertices[t.mIdx[2]]);
			p = sp.mU * v0 + sp.mV * v1 + (1.0f - sp.mU - sp.mV) * v2;
			p.StoreFloat3(&mSimVertices[strand.mStartVtx].mPosition);

			mSkinPoints.push_back(sp);
		}
	}

	Array<Vec3> r; // Outside loop to avoid reallocations
	Array<Vec3> x;
	Array<Vec3> k; // (bend_compliance, bend_compliance, stretch_compliance)
	Array<Vec3> g;
	Array<Quat> bishop;
	mMaxVerticesPerStrand = 0;
	for (const SStrand &strand : mSimStrands)
	{
		// Calculate max number of vertices per strand
		uint32 vertex_count = strand.VertexCount();
		mMaxVerticesPerStrand = max(mMaxVerticesPerStrand, vertex_count);

		// Calculate strand fraction for each vertex
		float total_length = strand.MeasureLength(mSimVertices);
		float cur_length = 0.0f;
		for (uint32 i = strand.mStartVtx; i < strand.mEndVtx - 1; ++i)
		{
			SVertex &v = mSimVertices[i];
			v.mStrandFraction = cur_length / total_length;
			cur_length += (Vec3(mSimVertices[i + 1].mPosition) - Vec3(v.mPosition)).Length();
		}
		mSimVertices[strand.mEndVtx - 1].mStrandFraction = 1.0f;

		// Particles
		// i=0     1       2
		// +------>+------>+
		//    x1      x2
		//
		// Let r_i be the edge between particle i - 1 and i in the rest pose
		// Let x_i be the edge between particle i - 1 and i in the deformed pose
		//
		// The force on particle i is:
		// f_i = k_i * (r_i - x_i) - k_{i+1} * (r_{i+1} - x_{i+1})
		// Where k_i = 1 / compliance_i
		//
		// We want to counter gravity, so:
		// f_i = -m_i * g
		//
		// Rearranging gives:
		// x_{i+1} * k_{i+1} - x_i * k_i = k_{i+1} * r_{i+1} - k_i * r_i + m_i * g
		//
		// Solving this with Gauss Seidel iteration:
		// x_i = (k_i * r_i - k_{i+1} * (r_{i+1} - x_{i+1}) - m_i * g) / k_i

		r.resize(vertex_count); // Rest edge
		x.resize(vertex_count); // Deformed edge
		k.resize(vertex_count); // Spring constant
		g.resize(vertex_count); // Gravity
		bishop.resize(vertex_count);

		// First element unused
		x[0] = r[0] = g[0] = k[0] = Vec3::sNaN();

		const HairSettings::Material &material = mMaterials[strand.mMaterialIndex];
		HairSettings::GradientSampler gravity_sampler(material.mGravityFactor);
		for (uint32 i = 1; i < vertex_count; ++i)
		{
			const SVertex &v1 = mSimVertices[strand.mStartVtx + i - 1];
			const SVertex &v2 = mSimVertices[strand.mStartVtx + i];
			x[i] = r[i] = Vec3(v2.mPosition) - Vec3(v1.mPosition);
			constexpr float cMinCompliance = 1.0e-10f;
			float bend_compliance = 1.0f / max(cMinCompliance, material.GetBendCompliance(v2.mStrandFraction));
			float stretch_compliance = 1.0f / max(cMinCompliance, material.mStretchCompliance);
			k[i] = Vec3(bend_compliance, bend_compliance, stretch_compliance);
			g[i] = v2.mInvMass > 0.0f? (material.mGravityPreloadFactor / v2.mInvMass) * mInitialGravity * gravity_sampler.Sample(v2.mStrandFraction) : Vec3::sZero();
		}

		// Solve for x
		if (material.mGravityPreloadFactor > 0.0f)
			for (int iteration = 0; iteration < 10; ++iteration)
			{
				// Don't modify the 1st vertex since it's fixed
				// Loop backwards so that we can use the latest value of x[i + 1]
				for (uint32 i = vertex_count - 1; i >= 1; --i)
				{
					// Calculate reference frame for this edge
					Vec3 frame_x = x[i].Normalized();
					Vec3 frame_y = frame_x.GetNormalizedPerpendicular();
					Vec3 frame_z = frame_x.Cross(frame_y);
					Mat44 frame(Vec4(frame_y, 0), Vec4(frame_z, 0), Vec4(frame_x, 0), Vec4(0, 0, 0, 1));

					// Gauss Seidel iteration
					// Note that we take all quantities to local space so that we can separate bend and stretch compliance and apply those as a simple vector multiplication
					Vec3 x_local = k[i] * frame.Multiply3x3Transposed(r[i]) - frame.Multiply3x3Transposed(g[i]);
					if (i < vertex_count - 1)
						x_local -= k[i + 1] * frame.Multiply3x3Transposed(r[i + 1] - x[i + 1]);
					x[i] = frame.Multiply3x3(x_local / k[i]);
				}
			}

		// Calculate the Bishop frame for the first rod in the strand
		{
			SVertex &v1 = mSimVertices[strand.mStartVtx];
			Vec3 tangent = x[1];
			v1.mLength = tangent.Length();
			JPH_ASSERT(v1.mLength > 0.0f, "Rods of zero length are not supported!");
			tangent /= v1.mLength;
			Vec3 normal = tangent.GetNormalizedPerpendicular();
			Vec3 binormal = tangent.Cross(normal);
			bishop[0] = Mat44(Vec4(normal, 0), Vec4(binormal, 0), Vec4(tangent, 0), Vec4(0, 0, 0, 1)).GetQuaternion().Normalized();
			bishop[0].StoreFloat4(&v1.mBishop);
		}

		// Calculate the Bishop frames for the rest of the rods in the strand
		for (uint32 i = 1; i < vertex_count - 1; ++i)
		{
			SVertex &v1 = mSimVertices[strand.mStartVtx + i];
			const SVertex &v2 = mSimVertices[strand.mStartVtx + i + 1];

			// Get the normal and tangent of the first rod's Bishop frame (that was already calculated)
			Mat44 r1_frame = Mat44::sRotation(bishop[i - 1]);
			Vec3 tangent1 = r1_frame.GetAxisZ();
			Vec3 normal1 = r1_frame.GetAxisX();

			// Calculate the Bishop frame for the 2nd rod
			Vec3 tangent2 = x[i + 1];
			v1.mLength = tangent2.Length();
			JPH_ASSERT(v1.mLength > 0.0f, "Rods of zero length are not supported!");
			tangent2 /= v1.mLength;
			Vec3 t1_cross_t2 = tangent1.Cross(tangent2);
			float sin_angle = t1_cross_t2.Length();
			Vec3 normal2 = normal1;
			if (sin_angle > 1.0e-6f)
			{
				// Rotate normal2
				t1_cross_t2 /= sin_angle;
				normal2 = Quat::sRotation(t1_cross_t2, ASin(sin_angle)) * normal2;

				// Ensure normal2 is perpendicular to tangent2
				normal2 -= normal2.Dot(tangent2) * tangent2;
				normal2 = normal2.Normalized();
			}
			Vec3 binormal2 = tangent2.Cross(normal2);
			bishop[i] = Mat44(Vec4(normal2, 0), Vec4(binormal2, 0), Vec4(tangent2, 0), Vec4(0, 0, 0, 1)).GetQuaternion().Normalized();

			// Calculate the delta, used in simulation
			(bishop[i - 1].Conjugated() * bishop[i]).Normalized().StoreFloat4(&v1.mOmega0);

			// Calculate the Bishop frame in the modeled pose for initializing the simulation
			Vec3 modeled_tangent2 = (Vec3(v2.mPosition) - Vec3(v1.mPosition)).Normalized();
			Quat modeled_bishop = Quat::sFromTo(tangent2, modeled_tangent2) * bishop[i];
			modeled_bishop.StoreFloat4(&v1.mBishop);
		}

		// Copy Bishop frame to the last vertex
		mSimVertices[strand.mEndVtx - 1].mBishop = mSimVertices[strand.mEndVtx - 2].mBishop;
	}

	// Finalize skin points by calculating how to go from triangle frame to Bishop frame
	for (SkinPoint &sp : mSkinPoints)
	{
		const IndexedTriangleNoMaterial &t = mScalpTriangles[sp.mTriangleIndex];
		Vec3 v0 = Vec3(mScalpVertices[t.mIdx[0]]);
		Vec3 v1 = Vec3(mScalpVertices[t.mIdx[1]]);
		Vec3 v2 = Vec3(mScalpVertices[t.mIdx[2]]);

		// Get tangent vector
		Vec3 tangent = (v1 - v0).Normalized();

		// Get normal of the triangle
		Vec3 normal = tangent.Cross(v2 - v0).Normalized();

		// Calculate basis for the triangle
		Vec3 binormal = tangent.Cross(normal);
		Quat triangle_basis = Mat44(Vec4(normal, 0), Vec4(binormal, 0), Vec4(tangent, 0), Vec4(0, 0, 0, 1)).GetQuaternion();

		// Calculate how to rotate from the triangle basis to the Bishop frame of the root
		Quat to_bishop = triangle_basis.Conjugated() * Quat(mSimVertices[mSimStrands[&sp - mSkinPoints.data()].mStartVtx].mBishop);
		sp.mToBishop = to_bishop.CompressUnitQuat();
	}

	// Calculate the grid size
	mSimulationBounds = {};
	for (const SVertex &v : mSimVertices)
		mSimulationBounds.Encapsulate(Vec3(v.mPosition));
	mSimulationBounds.ExpandBy(mSimulationBoundsPadding);

	// Prepare neutral density grid
	mNeutralDensity.resize(mGridSize.GetX() * mGridSize.GetY() * mGridSize.GetZ(), 0.0f);
	GridSampler sampler(this);
	for (const SVertex &v : mSimVertices)
		if (v.mInvMass > 0.0f)
		{
			sampler.Sample(Vec3(v.mPosition), [this, &v](uint32 inIndex, float inFraction) {
				mNeutralDensity[inIndex] += inFraction / v.mInvMass;
			});
		}

	// Calculate density scale for drawing the grid
	mDensityScale = 0.0f;
	for (float density : mNeutralDensity)
		mDensityScale = max(mDensityScale, density);
	if (mDensityScale > 0.0f)
		mDensityScale = 1.0f / mDensityScale;

	// Prepare render vertices
	for (RVertex &v : mRenderVertices)
	{
		Vec3 render_pos(v.mPosition);

		float total_weight = 0.0f;
		for (SVertexInfluence &inf : v.mInfluences)
			if (inf.mVertexIndex != cNoInfluence)
			{
				const SVertex &simulated_vertex = mSimVertices[inf.mVertexIndex];
				Vec3 simulated_pos(simulated_vertex.mPosition);
				Vec3 local_position = Quat(simulated_vertex.mBishop).InverseRotate(render_pos - simulated_pos);
				local_position.StoreFloat3(&inf.mRelativePosition);

				// Weigh according to inverse distance to the simulated vertex
				inf.mWeight = 1.0f / (local_position.Length() + 1.0e-6f);
				total_weight += inf.mWeight;
			}
			else
				inf.mWeight = 0.0f;

		// Normalize weights
		if (total_weight > 0.0f)
			for (SVertexInfluence &a : v.mInfluences)
				if (a.mVertexIndex != cNoInfluence)
					a.mWeight /= total_weight;

		// Order so that largest weight comes first
		QuickSort(std::begin(v.mInfluences), std::end(v.mInfluences), [](const SVertexInfluence &inLHS, const SVertexInfluence &inRHS) {
				return inLHS.mWeight > inRHS.mWeight;
			});
	}
}

void HairSettings::InitCompute(ComputeSystem *inComputeSystem)
{
	// Optional: We can attach the roots of the hairs to the scalp
	if (!mScalpTriangles.empty() && !mSkinPoints.empty())
	{
		mScalpTrianglesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mScalpTriangles.size() * 3, sizeof(uint32), mScalpTriangles.data()).Get();
		mSkinPointsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mSkinPoints.size(), sizeof(SkinPoint), mSkinPoints.data()).Get();

		// We can skin the scalp or the skinned vertices can be provided externally
		if (!mScalpVertices.empty() && !mScalpInverseBindPose.empty() && !mScalpSkinWeights.empty())
		{
			mScalpVerticesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mScalpVertices.size(), sizeof(Float3), mScalpVertices.data()).Get();
			mScalpSkinWeightsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mScalpSkinWeights.size(), sizeof(JPH_HairSkinWeight), mScalpSkinWeights.data()).Get();
		}
	}

	// Calculate the number of vertices for every strand
	Array<uint8> strand_vertex_counts;
	strand_vertex_counts.resize((mSimStrands.size() + sizeof(uint32) - 1) & ~(sizeof(uint32) - 1), 0); // Make size multiple of sizeof(uint32)
	for (size_t i = 0, n = mSimStrands.size(); i < n; ++i)
	{
		uint32 count = mSimStrands[i].VertexCount();
		JPH_ASSERT(count < 256);
		strand_vertex_counts[i] = (uint8)count;
	}

	// Calculate material index for every strand
	Array<uint8> strand_material_indices;
	strand_material_indices.resize((mSimStrands.size() + sizeof(uint32) - 1) & ~(sizeof(uint32) - 1), 0); // Make size multiple of sizeof(uint32)
	for (size_t i = 0, n = mSimStrands.size(); i < n; ++i)
	{
		uint32 material_index = mSimStrands[i].mMaterialIndex;
		JPH_ASSERT(material_index < 256);
		strand_material_indices[i] = (uint8)material_index;
	}

	// Create buffers that contain information about the rest pose of the hair
	// Rearrange vertices so that the first vertices of all strands are grouped together, then the second vertices, etc.
	uint num_vertices = uint(mMaxVerticesPerStrand * mSimStrands.size());
	Array<Float3> vertices_position;
	vertices_position.resize(num_vertices);
	Array<uint32> vertices_bishop;
	vertices_bishop.resize(num_vertices);
	Array<uint32> vertices_omega0;
	vertices_omega0.resize(num_vertices);
	Array<uint32> vertices_fixed;
	vertices_fixed.resize((num_vertices + 31) / 32, 0);
	Array<float> vertices_length;
	vertices_length.resize(num_vertices);
	Array<uint32> vertices_strand_fraction;
	vertices_strand_fraction.resize((num_vertices + 3) / 4, 0);
	for (size_t s = 0, ns = mSimStrands.size(); s < ns; ++s)
	{
		const SStrand &strand = mSimStrands[s];
		for (uint32 v = 0, nv = strand.VertexCount(); v < nv; ++v)
		{
			const SVertex &in_v = mSimVertices[strand.mStartVtx + v];
			size_t idx = v * mSimStrands.size() + s;

			vertices_position[idx] = in_v.mPosition;
			vertices_bishop[idx] = Vec4::sLoadFloat4(&in_v.mBishop).CompressUnitVector();
			vertices_omega0[idx] = Vec4::sLoadFloat4(&in_v.mOmega0).CompressUnitVector();
			vertices_length[idx] = in_v.mLength;
			if (in_v.mInvMass <= 0.0f)
				vertices_fixed[idx >> 5] |= uint32(1 << (idx & 31));
			vertices_strand_fraction[idx >> 2] |= uint32(in_v.mStrandFraction * 255.0f) << ((idx & 3) << 3);
		}
	}

	// Calculate a map from render vertex to strand index
	Array<uint32> simulation_vertex_to_strand_idx;
	simulation_vertex_to_strand_idx.resize(mSimVertices.size(), ~uint32(0));
	for (const SStrand &strand : mSimStrands)
		for (uint v = strand.mStartVtx; v < strand.mEndVtx; ++v)
				simulation_vertex_to_strand_idx[v] = uint32(&strand - mSimStrands.data());

	// Create buffer for simulated vertex influences
	Array<JPH_HairSVertexInfluence> svertex_influences;
	svertex_influences.resize(mRenderVertices.size() * cHairNumSVertexInfluences);
	for (size_t v = 0, n = mRenderVertices.size(); v < n; ++v)
		for (uint a = 0; a < cHairNumSVertexInfluences; ++a)
		{
			JPH_HairSVertexInfluence &inf = svertex_influences[v * cHairNumSVertexInfluences + a];
			inf = static_cast<const JPH_HairSVertexInfluence &>(mRenderVertices[v].mInfluences[a]);

			// Remap vertex index to reflect the transposing of the position buffer
			if (inf.mVertexIndex != cNoInfluence)
			{
				uint32 strand_idx = simulation_vertex_to_strand_idx[inf.mVertexIndex];
				uint32 start_vtx = mSimStrands[strand_idx].mStartVtx;
				inf.mVertexIndex = strand_idx + (inf.mVertexIndex - start_vtx) * uint32(mSimStrands.size());
			}
			else
			{
				// The shader doesn't check if weight is zero, it just takes the vertex. Make sure the index points to something.
				inf.mVertexIndex = 0;
			}
		}

	mVerticesPositionCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_position.size(), sizeof(Float3), vertices_position.data()).Get();
	mVerticesBishopCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_bishop.size(), sizeof(uint32), vertices_bishop.data()).Get();
	mVerticesOmega0CB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_omega0.size(), sizeof(uint32), vertices_omega0.data()).Get();
	mVerticesLengthCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_length.size(), sizeof(float), vertices_length.data()).Get();
	mVerticesFixedCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_fixed.size(), sizeof(uint32), vertices_fixed.data()).Get();
	mVerticesStrandFractionCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, vertices_strand_fraction.size(), sizeof(uint32), vertices_strand_fraction.data()).Get();
	mStrandVertexCountsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, strand_vertex_counts.size() / sizeof(uint32), sizeof(uint32), strand_vertex_counts.data()).Get();
	mStrandMaterialIndexCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, strand_material_indices.size() / sizeof(uint32), sizeof(uint32), strand_material_indices.data()).Get();
	mNeutralDensityCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mNeutralDensity.size(), sizeof(float), mNeutralDensity.data()).Get();
	mSVertexInfluencesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::Buffer, mRenderVertices.size() * cHairNumSVertexInfluences, sizeof(JPH_HairSVertexInfluence), svertex_influences.data()).Get();
}

void HairSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mSimVertices);
	inStream.Write(mSimStrands);
	inStream.Write(mRenderVertices);
	inStream.Write(mRenderStrands);
	inStream.Write(mScalpVertices);
	inStream.Write(mScalpTriangles);
	inStream.Write(mScalpInverseBindPose);
	inStream.Write(mScalpSkinWeights);
	inStream.Write(mScalpNumSkinWeightsPerVertex);
	inStream.Write(mNumIterationsPerSecond);
	inStream.Write(mMaxDeltaTime);
	inStream.Write(mGridSize);
	inStream.Write(mSimulationBoundsPadding);
	inStream.Write(mInitialGravity);
	inStream.Write(mMaterials, [](const Material &inElement, StreamOut &inS) {
		inS.Write(inElement.mEnableCollision);
		inS.Write(inElement.mEnableLRA);
		inS.Write(inElement.mLinearDamping);
		inS.Write(inElement.mAngularDamping);
		inS.Write(inElement.mMaxLinearVelocity);
		inS.Write(inElement.mMaxAngularVelocity);
		inElement.mGravityFactor.SaveBinaryState(inS);
		inS.Write(inElement.mFriction);
		inS.Write(inElement.mBendCompliance);
		inS.Write(inElement.mBendComplianceMultiplier);
		inS.Write(inElement.mStretchCompliance);
		inS.Write(inElement.mInertiaMultiplier);
		inElement.mHairRadius.SaveBinaryState(inS);
		inElement.mWorldTransformInfluence.SaveBinaryState(inS);
		inElement.mGridVelocityFactor.SaveBinaryState(inS);
		inS.Write(inElement.mGridDensityForceFactor);
		inElement.mGlobalPose.SaveBinaryState(inS);
		inElement.mSkinGlobalPose.SaveBinaryState(inS);
		inS.Write(inElement.mSimulationStrandsFraction);
		inS.Write(inElement.mGravityPreloadFactor);
	});
	inStream.Write(mSkinPoints);
	inStream.Write(mSimulationBounds);
	inStream.Write(mNeutralDensity);
	inStream.Write(mDensityScale);
	inStream.Write(mMaxVerticesPerStrand);
}

void HairSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mSimVertices);
	inStream.Read(mSimStrands);
	inStream.Read(mRenderVertices);
	inStream.Read(mRenderStrands);
	inStream.Read(mScalpVertices);
	inStream.Read(mScalpTriangles);
	inStream.Read(mScalpInverseBindPose);
	inStream.Read(mScalpSkinWeights);
	inStream.Read(mScalpNumSkinWeightsPerVertex);
	inStream.Read(mNumIterationsPerSecond);
	inStream.Read(mMaxDeltaTime);
	inStream.Read(mGridSize);
	inStream.Read(mSimulationBoundsPadding);
	inStream.Read(mInitialGravity);
	inStream.Read(mMaterials, [](StreamIn &inS, Material &outElement) {
		inS.Read(outElement.mEnableCollision);
		inS.Read(outElement.mEnableLRA);
		inS.Read(outElement.mLinearDamping);
		inS.Read(outElement.mAngularDamping);
		inS.Read(outElement.mMaxLinearVelocity);
		inS.Read(outElement.mMaxAngularVelocity);
		outElement.mGravityFactor.RestoreBinaryState(inS);
		inS.Read(outElement.mFriction);
		inS.Read(outElement.mBendCompliance);
		inS.Read(outElement.mBendComplianceMultiplier);
		inS.Read(outElement.mStretchCompliance);
		inS.Read(outElement.mInertiaMultiplier);
		outElement.mHairRadius.RestoreBinaryState(inS);
		outElement.mWorldTransformInfluence.RestoreBinaryState(inS);
		outElement.mGridVelocityFactor.RestoreBinaryState(inS);
		inS.Read(outElement.mGridDensityForceFactor);
		outElement.mGlobalPose.RestoreBinaryState(inS);
		outElement.mSkinGlobalPose.RestoreBinaryState(inS);
		inS.Read(outElement.mSimulationStrandsFraction);
		inS.Read(outElement.mGravityPreloadFactor);
	});
	inStream.Read(mSkinPoints);
	inStream.Read(mSimulationBounds);
	inStream.Read(mNeutralDensity);
	inStream.Read(mDensityScale);
	inStream.Read(mMaxVerticesPerStrand);
}

void HairSettings::PrepareForScalpSkinning(Mat44Arg inJointToHair, const Mat44 *inJointMatrices, Mat44 *outJointMatrices) const
{
	for (uint32 i = 0, n = (uint32)mScalpInverseBindPose.size(); i < n; ++i)
		outJointMatrices[i] = inJointToHair * inJointMatrices[i] * mScalpInverseBindPose[i];
}

void HairSettings::SkinScalpVertices(Mat44Arg inJointToHair, const Mat44 *inJointMatrices, Array<Vec3> &outVertices) const
{
	outVertices.resize(mScalpVertices.size());

	// Pre transform all joint matrices
	Array<Mat44> joint_matrices;
	joint_matrices.resize((uint32)mScalpInverseBindPose.size());
	PrepareForScalpSkinning(inJointToHair, inJointMatrices, joint_matrices.data());

	// Skin all vertices
	for (uint32 i = 0; i < (uint32)mScalpVertices.size(); ++i)
	{
		Vec3 &v = outVertices[i];
		v = Vec3::sZero();
		for (const SkinWeight *w = mScalpSkinWeights.data() + i * mScalpNumSkinWeightsPerVertex, *w_end = w + mScalpNumSkinWeightsPerVertex; w < w_end; ++w)
			if (w->mWeight > 0.0f)
				v += w->mWeight * joint_matrices[w->mJointIdx] * Vec3(mScalpVertices[i]);
	}
}

JPH_NAMESPACE_END

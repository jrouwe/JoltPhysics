// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/QuickSort.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>

JPH_NAMESPACE_BEGIN

// Removes internal edges from collision results. Can be used to filter out 'ghost collisions'.
// Based on: Contact generation for meshes - Pierre Terdiman (https://www.codercorner.com/MeshContacts.pdf)
class InternalEdgeRemovingCollector : public CollideShapeCollector
{
	inline static constexpr uint cMaxDelayedResults = 16;
	inline static constexpr uint cMaxVoidedFeatures = 128;

	/// Check if a vertex is voided
	inline bool				IsVoided(Vec3 inV) const
	{
		for (const Float3 &vf : mVoidedFeatures)
			if (inV.IsClose(Vec3::sLoadFloat3Unsafe(vf), 1.0e-8f))
				return true;
		return false;
	}

	/// Add all vertices of a face to the voided features
	inline void				VoidFeatures(const CollideShapeResult &inResult)
	{
		for (const Vec3 &v : inResult.mShape2Face)
			if (!IsVoided(v))
			{
				if (mVoidedFeatures.size() == cMaxVoidedFeatures)
					break;
				Float3 f;
				v.StoreFloat3(&f);
				mVoidedFeatures.push_back(f);
			}
	}

	/// Call the chained collector
	inline void				Chain(const CollideShapeResult &inResult)
	{
		mChainedCollector.AddHit(inResult);
	}

	/// Call the chained collector and void all features of inResult
	inline void				ChainAndVoid(const CollideShapeResult &inResult)
	{
		Chain(inResult);
		VoidFeatures(inResult);
	}

public:
	/// Constructor, configures a collector to be called with all the results that do not hit internal edges
							InternalEdgeRemovingCollector(CollideShapeCollector &inChainedCollector) :
		mChainedCollector(inChainedCollector)
	{
	}

	// See: CollideShapeCollector::AddHit
	virtual void			AddHit(const CollideShapeResult &inResult) override
	{
		// We only support welding when the shape is a triangle or has more vertices so that we can calculate a normal
		if (inResult.mShape2Face.size() < 3)
			return ChainAndVoid(inResult);

		// Get the triangle normal of shape 2 face
		Vec3 triangle_normal = (inResult.mShape2Face[1] - inResult.mShape2Face[0]).Cross(inResult.mShape2Face[2] - inResult.mShape2Face[0]);
		float triangle_normal_len = triangle_normal.Length();
		if (triangle_normal_len < 1e-6f)
			return ChainAndVoid(inResult);

		// If the triangle normal matches the contact normal within 1 degree, we can process the contact immediately
		Vec3 contact_normal = -inResult.mPenetrationAxis;
		float contact_normal_len = inResult.mPenetrationAxis.Length();
		if (triangle_normal.Dot(contact_normal) > 0.999848f * contact_normal_len * triangle_normal_len) // cos(1 degree)
			return ChainAndVoid(inResult);

		// Delayed processing
		if (mDelayedResults.size() == cMaxDelayedResults)
			return ChainAndVoid(inResult);
		mDelayedResults.push_back(inResult);
	}

	/// After all hits have been added, call this function to process the delayed results
	void					Flush()
	{
		// Sort on biggest penetration depth first
		uint sorted_indices[cMaxDelayedResults];
		for (uint i = 0; i < uint(mDelayedResults.size()); ++i)
			sorted_indices[i] = i;
		QuickSort(sorted_indices, sorted_indices + mDelayedResults.size(), [this](uint inLHS, uint inRHS) { return mDelayedResults[inLHS].mPenetrationDepth > mDelayedResults[inRHS].mPenetrationDepth; });

		// Loop over all results
		for (uint i = 0; i < uint(mDelayedResults.size()); ++i)
		{
			// Check if this face contains any voided features
			const CollideShapeResult &r = mDelayedResults[sorted_indices[i]];
			for (Vec3 v : r.mShape2Face)
				if (IsVoided(v))
					goto skip;

			// No voided features, accept the contact
			Chain(r);
		skip:;

			// Void the features of this face
			VoidFeatures(r);
		}
	}

	/// Version of CollisionDispatch::sCollideShapeVsShape that removes internal edges
	static void				sCollideShapeVsShape(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector, const ShapeFilter &inShapeFilter = { })
	{
		InternalEdgeRemovingCollector wrapper(ioCollector);
		CollisionDispatch::sCollideShapeVsShape(inShape1, inShape2, inScale1, inScale2, inCenterOfMassTransform1, inCenterOfMassTransform2, inSubShapeIDCreator1, inSubShapeIDCreator2, inCollideShapeSettings, wrapper, inShapeFilter);
		wrapper.Flush();
	}

private:
	CollideShapeCollector &	mChainedCollector;
	StaticArray<Float3, cMaxVoidedFeatures> mVoidedFeatures; // Read with Vec3::sLoadFloat3Unsafe so must not be the last member
	StaticArray<CollideShapeResult, cMaxDelayedResults> mDelayedResults;
};

JPH_NAMESPACE_END

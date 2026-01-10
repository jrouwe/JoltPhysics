// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/IndexedTriangle.h>
#include <Jolt/ObjectStream/SerializableObject.h>
#include <Jolt/Compute/ComputeBuffer.h>
#include <Jolt/Compute/ComputeSystem.h>
#include <Jolt/Shaders/HairStructs.h>

JPH_NAMESPACE_BEGIN

class StreamOut;
class StreamIn;

/// This class defines the setup of a hair groom, it can be shared between multiple hair instances
class JPH_EXPORT HairSettings : public RefTarget<HairSettings>
{
	JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, HairSettings)

public:
	/// How much a vertex is influenced by a joint
	struct JPH_EXPORT SkinWeight : public JPH_HairSkinWeight
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SkinWeight)
	};

	/// Information about where a hair strand is attached to the scalp mesh
	struct JPH_EXPORT SkinPoint : public JPH_HairSkinPoint
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SkinPoint)
	};

	static constexpr uint32 cNoInfluence = ~uint32(0);

	/// Describes how a render vertex is influenced by a simulated vertex
	struct JPH_EXPORT SVertexInfluence : public JPH_HairSVertexInfluence
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SVertexInfluence)

		inline			SVertexInfluence()							{ mVertexIndex = cNoInfluence; mRelativePosition = JPH_float3(0, 0, 0); mWeight = 0.0f; }
	};

	/// A render vertex
	struct JPH_EXPORT RVertex
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, RVertex)

		Float3			mPosition { 0, 0, 0 };						///< Initial position of the vertex
		SVertexInfluence mInfluences[cHairNumSVertexInfluences];	///< Attach to X simulated vertices (computed during Init)
	};

	/// A simulated vertex in a hair strand
	struct JPH_EXPORT SVertex
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SVertex)

		/// Constructor
						SVertex() = default;
		explicit		SVertex(const Float3 &inPosition, float inInvMass = 1.0f) : mPosition(inPosition), mInvMass(inInvMass) { }

		Float3			mPosition { 0, 0, 0 };						///< Initial position of the vertex in its modeled pose
		float			mInvMass = 1.0f;							///< Inverse of the mass of the vertex
		float			mLength = 0.0f;								///< Initial distance of this vertex to the next of the unloaded strand, computed by Init
		float			mStrandFraction = 0.0f;						///< Fraction along the strand, 0 = start, 1 = end, computed by Init
		Float4			mBishop { 0, 0, 0, 1.0f };					///< Bishop frame of the strand in its modeled pose, computed by Init
		Float4			mOmega0 { 0, 0, 0, 1.0f };					///< Conjugate(Previous Bishop) * Bishop, defines the rotation difference between the previous rod and this one of the unloaded strand, computed by Init
	};

	/// A hair render strand
	struct JPH_EXPORT RStrand
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, RStrand)

		/// Constructor
						RStrand() = default;
						RStrand(uint32 inStartVtx, uint32 inEndVtx) : mStartVtx(inStartVtx), mEndVtx(inEndVtx) { }

		uint32			VertexCount() const							{ return mEndVtx - mStartVtx; }

		float			MeasureLength(const Array<SVertex> &inVertices) const
		{
			float length = 0.0f;
			for (uint32 v = mStartVtx; v < mEndVtx - 1; ++v)
				length += (Vec3(inVertices[v + 1].mPosition) - Vec3(inVertices[v].mPosition)).Length();
			return length;
		}

		uint32			mStartVtx;
		uint32			mEndVtx;
	};

	/// A hair simulation strand
	struct JPH_EXPORT SStrand : public RStrand
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, SStrand)

						SStrand() = default;
						SStrand(uint32 inStartVtx, uint32 inEndVtx, uint32 inMaterialIndex) : RStrand(inStartVtx, inEndVtx), mMaterialIndex(inMaterialIndex) { }

		uint32			mMaterialIndex = 0;							///< Index in mMaterials
	};

	/// Gradient along a hair strand of a value, e.g. compliance, friction, etc.
	class JPH_EXPORT Gradient
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Gradient)

	public:
						Gradient() = default;
						Gradient(float inMin, float inMax, float inMinFraction = 0.0f, float inMaxFraction = 1.0f) : mMin(inMin), mMax(inMax), mMinFraction(inMinFraction), mMaxFraction(inMaxFraction) { }

		/// We drive a value to its target with fixed time steps using:
		///
		/// x(t + fixed_dt) = target + (1 - k) * (x(t) - target)
		///
		/// For varying time steps we can rewrite this to:
		///
		/// x(t + dt) = target + (1 - k)^inTimeRatio * (x(t) - target)
		///
		/// Where inTimeRatio is defined as dt / fixed_dt.
		///
		/// This means k' = 1 - (1 - k)^inTimeRatio
		Gradient		MakeStepDependent(float inTimeRatio) const
		{
			auto make_dependent = [inTimeRatio](float inValue) {
					return 1.0f - std::pow(1.0f - inValue, inTimeRatio);
				};

			return Gradient(make_dependent(mMin), make_dependent(mMax), mMinFraction, mMaxFraction);
		}

		/// Saves the state of this object in binary form to inStream. Doesn't store the compute buffers.
		void			SaveBinaryState(StreamOut &inStream) const;

		/// Restore the state of this object from inStream.
		void			RestoreBinaryState(StreamIn &inStream);

		float			mMin = 0.0f;								///< Minimum value of the gradient
		float			mMax = 1.0f;								///< Maximum value of the gradient
		float			mMinFraction = 0.0f;						///< Fraction along the hair strand that corresponds to the minimum value
		float			mMaxFraction = 1.0f;						///< Fraction along the hair strand that corresponds to the maximum value
	};

	class GradientSampler
	{
	public:
						GradientSampler() = default;

		explicit		GradientSampler(const Gradient &inGradient) :
			mMultiplier((inGradient.mMax - inGradient.mMin) / (inGradient.mMaxFraction - inGradient.mMinFraction)),
			mOffset(inGradient.mMin - inGradient.mMinFraction * mMultiplier),
			mMin(min(inGradient.mMin, inGradient.mMax)),
			mMax(max(inGradient.mMin, inGradient.mMax))
		{
		}

		/// Sample the value along the strand
		inline float	Sample(float inFraction) const
		{
			return min(mMax, max(mMin, mOffset + inFraction * mMultiplier));
		}

		inline float	Sample(const SStrand &inStrand, uint32 inVertex) const
		{
			return Sample(float(inVertex - inStrand.mStartVtx) / float(inStrand.VertexCount() - 1));
		}

		/// Convert to Float4 to pass to shader
		inline Float4	ToFloat4() const
		{
			return Float4(mMultiplier, mOffset, mMin, mMax);
		}

	private:
		float			mMultiplier;
		float			mOffset;
		float			mMin;
		float			mMax;
	};

	/// The material determines the simulation parameters for a hair strand
	struct JPH_EXPORT Material
	{
		JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(JPH_EXPORT, Material)

		/// Returns if this material needs a density/velocity grid
		bool			NeedsGrid() const							{ return mGridVelocityFactor.mMin != 0.0f || mGridVelocityFactor.mMax != 0.0f || mGridDensityForceFactor != 0.0f; }

		/// If this material only needs running the global pose logic
		bool			GlobalPoseOnly() const						{ return !mEnableCollision && mGlobalPose.mMin == 1.0f && mGlobalPose.mMax == 1.0f; }

		/// Calculate the bend compliance at a fraction along the strand
		float			GetBendCompliance(float inStrandFraction) const
		{
			float fraction = inStrandFraction * 3.0f;
			uint idx = min(uint(fraction), 2u);
			fraction = fraction - float(idx);
			JPH_ASSERT(fraction >= 0.0f && fraction <= 1.0f);
			float multiplier = mBendComplianceMultiplier[idx] * (1.0f - fraction) + mBendComplianceMultiplier[idx + 1] * fraction;
			return multiplier * mBendCompliance;
		}

		bool			mEnableCollision = true;					///< Enable collision detection between hair strands and the environment.
		bool			mEnableLRA = true;							///< Enable Long Range Attachments to keep hair close to the modeled pose. This prevents excessive stretching when the head moves quickly.
		float			mLinearDamping = 2.0f;						///< Linear damping coefficient for the simulated rods.
		float			mAngularDamping = 2.0f;						///< Angular damping coefficient for the simulated rods.
		float			mMaxLinearVelocity = 10.0f;					///< Maximum linear velocity of a vertex.
		float			mMaxAngularVelocity = 50.0f;				///< Maximum angular velocity of a vertex.
		Gradient		mGravityFactor { 0.1f, 1.0f, 0.2f, 0.8f };	///< How much gravity affects the hair along its length, 0 = no gravity, 1 = full gravity. Can be used to reduce the effect of gravity.
		float			mFriction = 0.2f;							///< Collision friction coefficient. Usually in the range [0, 1]. 0 = no friction.
		float			mBendCompliance = 1.0e-7f;					///< Compliance for bend constraints: 1 / stiffness.
		Float4			mBendComplianceMultiplier = { 1.0f, 100.0f, 100.0f, 1.0f }; ///< Multiplier for bend compliance at 0%, 33%, 66% and 100% of the strand length.
		float			mStretchCompliance = 1.0e-8f;				///< Compliance for stretch constraints: 1 / stiffness.
		float			mInertiaMultiplier = 10.0f;					///< Multiplier applied to the mass of a rod to calculate its inertia.
		Gradient		mHairRadius = { 0.001f, 0.001f };			///< Radius of the hair strand along its length, used for collision detection.
		Gradient		mWorldTransformInfluence { 0.0f, 1.0f };	///< How much rotating the head influences the hair, 0 = not at all, the hair will move with the head as if it had no inertia. 1 = hair stays in place as the head moves and is correctly simulated. This can be used to reduce the effect of turning the head towards the root of strands.
		Gradient		mGridVelocityFactor { 0.05f, 0.01f };		///< Every iteration this fraction of the grid velocity will be applied to the vertex velocity. Defined at cDefaultIterationsPerSecond, if this changes, the value will internally be adjusted to result in the same behavior.
		float			mGridDensityForceFactor = 0.0f;				///< This factor will try to push the density of the hair towards the neutral density defined in the density grid. Note that can result in artifacts so defaults to 0.
		Gradient		mGlobalPose { 0.01f, 0, 0.0f, 0.3f };		///< Every iteration this fraction of the neutral pose will be applied to the vertex position. Defined at cDefaultIterationsPerSecond, if this changes, the value will internally be adjusted to result in the same behavior.
		Gradient		mSkinGlobalPose { 1.0f, 0.0f, 0.0f, 0.1f }; ///< How much the global pose follows the skin of the scalp. 0 is not following, 1 is fully following.
		float			mSimulationStrandsFraction = 0.1f;			///< Used by InitRenderAndSimulationStrands only. Indicates the fraction of strands that should be simulated.
		float			mGravityPreloadFactor = 0.0f;				///< Note: Not fully functional yet! This controls how much of the gravity we will remove from the modeled pose when initializing. A value of 1 fully removes gravity and should result in no sagging when the simulation starts. A value of 0 doesn't remove gravity.
	};

	/// Split the supplied render strands into render and simulation strands and calculate connections between them.
	/// When this function returns mSimVertices, mSimStrands, mRenderVertices and mRenderStrands are overwritten.
	/// @param inVertices Vertices for the strands.
	/// @param inStrands The strands that this instance should have.
	void				InitRenderAndSimulationStrands(const Array<SVertex> &inVertices, const Array<SStrand> &inStrands);

	/// Resample the hairs to a new fixed number of vertices per strand. Must be called prior to Init if desired.
	static void			sResample(Array<SVertex> &ioVertices, Array<SStrand> &ioStrands, uint32 inNumVerticesPerStrand);

	/// Initialize the structure, calculating simulation bounds and vertex properties
	/// @param outMaxDistSqHairToScalp Maximum distance^2 the root vertex of a hair is from the scalp, can be used to check if the hair matched the scalp correctly
	void				Init(float &outMaxDistSqHairToScalp);

	/// Must be called after Init to setup the compute buffers
	void				InitCompute(ComputeSystem *inComputeSystem);

	/// Sample the neutral density at a grid position
	float				GetNeutralDensity(uint32 inX, uint32 inY, uint32 inZ) const
	{
		JPH_ASSERT(inX < mGridSize.GetX() && inY < mGridSize.GetY() && inZ < mGridSize.GetZ());
		return mNeutralDensity[inX + inY * mGridSize.GetX() + inZ * mGridSize.GetX() * mGridSize.GetY()];
	}

	/// Get the number of vertices in the vertex buffers padded to a multiple of mMaxVerticesPerStrand.
	inline uint32		GetNumVerticesPadded() const
	{
		return uint32(mSimStrands.size()) * mMaxVerticesPerStrand;
	}

	/// @brief Calculates the pose used for skinning the scalp
	/// @param inJointToHair Transform to bring the model space joint matrices to the hair local space
	/// @param inJointMatrices Model space joint matrices of the joints in the face
	/// @param outJointMatrices Joint matrices combined with the inverse bind pose
	void				PrepareForScalpSkinning(Mat44Arg inJointToHair, const Mat44 *inJointMatrices, Mat44 *outJointMatrices) const;

	/// Skin the scalp mesh to the given joint matrices and output the skinned scalp vertices
	/// @param inJointToHair Transform to bring the model space joint matrices to the hair local space
	/// @param inJointMatrices Model space joint matrices of the joints in the face
	/// @param outVertices Returns skinned vertices
	void				SkinScalpVertices(Mat44Arg inJointToHair, const Mat44 *inJointMatrices, Array<Vec3> &outVertices) const;

	/// Saves the state of this object in binary form to inStream. Doesn't store the compute buffers.
	void				SaveBinaryState(StreamOut &inStream) const;

	/// Restore the state of this object from inStream.
	void				RestoreBinaryState(StreamIn &inStream);

	class GridSampler
	{
	public:
		inline explicit	GridSampler(const HairSettings *inSettings) :
			mGridSizeMin2(inSettings->mGridSize - UVec4::sReplicate(2)),
			mGridSizeMin1((inSettings->mGridSize - UVec4::sReplicate(1)).ToFloat()),
			mGridStride(1, inSettings->mGridSize.GetX(), inSettings->mGridSize.GetX() * inSettings->mGridSize.GetY(), 0),
			mOffset(inSettings->mSimulationBounds.mMin),
			mScale(Vec3(inSettings->mGridSize.ToFloat()) / inSettings->mSimulationBounds.GetSize())
		{
		}

		/// Convert a position in hair space to a grid index and fraction
		inline void		PositionToIndexAndFraction(Vec3Arg inPosition, UVec4 &outIndex, Vec3 &outFraction) const
		{
			// Get position in grid space
			Vec3 grid_pos = Vec3::sMin(Vec3::sMax(inPosition - mOffset, Vec3::sZero()) * mScale, mGridSizeMin1);
			outIndex = UVec4::sMin(Vec4(grid_pos).ToInt(), mGridSizeMin2);
			outFraction = grid_pos - Vec3(outIndex.ToFloat());
		}

		template <typename F>
		inline void		Sample(UVec4Arg inIndex, Vec3Arg inFraction, const F &inFunc) const
		{
			Vec3 fraction[] = { Vec3::sReplicate(1.0f) - inFraction, inFraction };

			// Sample the grid
			for (uint32 z = 0; z < 2; ++z)
				for (uint32 y = 0; y < 2; ++y)
					for (uint32 x = 0; x < 2; ++x)
					{
						uint32 index = mGridStride.Dot(inIndex + UVec4(x, y, z, 0));
						float combined_fraction = fraction[x].GetX() * fraction[y].GetY() * fraction[z].GetZ();
						inFunc(index, combined_fraction);
					}
		}

		template <typename F>
		inline void		Sample(Vec3Arg inPosition, const F &inFunc) const
		{
			UVec4 index;
			Vec3 fraction;
			PositionToIndexAndFraction(inPosition, index, fraction);
			Sample(index, fraction, inFunc);
		}

		UVec4			mGridSizeMin2;
		Vec3			mGridSizeMin1;
		UVec4			mGridStride;
		Vec3			mOffset;
		Vec3			mScale;
	};

	static constexpr uint32 cDefaultIterationsPerSecond = 360;

	Array<SVertex>		mSimVertices;								///< Simulated vertices. Used by mSimStrands.
	Array<SStrand>		mSimStrands;								///< Defines the start and end of each simulated strand.

	Array<RVertex>		mRenderVertices;							///< Rendered vertices. Used by mRenderStrands.
	Array<RStrand>		mRenderStrands;								///< Defines the start and end of each rendered strand.

	Array<Float3>		mScalpVertices;								///< Vertices of the scalp mesh, used to attach hairs. Note that the hair vertices mSimVertices must be in the same space as these vertices.
	Array<IndexedTriangleNoMaterial> mScalpTriangles;				///< Triangles of the scalp mesh.
	Array<Mat44>		mScalpInverseBindPose;						///< Inverse bind pose of the scalp mesh, joints are in model space
	Array<SkinWeight>	mScalpSkinWeights;							///< Skin weights of the scalp mesh, for each vertex we have mScalpNumSkinWeightsPerVertex entries
	uint				mScalpNumSkinWeightsPerVertex = 0;			///< Number of skin weights per vertex

	uint32				mNumIterationsPerSecond = cDefaultIterationsPerSecond;
	float				mMaxDeltaTime = 1.0f / 30.0f;				///< Maximum delta time for the simulation step (to avoid running an excessively long step, note that this will effectively slow down time)
	UVec4				mGridSize { 32, 32, 32, 0 };				///< Number of grid cells used to simulate the hair. W unused.
	Vec3				mSimulationBoundsPadding = Vec3::sReplicate(0.1f); ///< Padding around the simulation bounds to ensure that the grid is large enough and that we detect collisions with the hairs. This is added on all sides after calculating the bounds in the neutral pose.
	Vec3				mInitialGravity { 0, -9.81f, 0 };			///< Initial gravity in local space of the hair, used to calculate the unloaded rest pose
	Array<Material>		mMaterials;									///< Materials used by the hair strands

	// Values computed by Init
	Array<SkinPoint>	mSkinPoints;								///< For each simulated vertex, where it is attached to the scalp mesh
	AABox				mSimulationBounds { Vec3::sZero(), 1.0f };	///< Bounds that the simulation is supposed to fit in
	Array<float>		mNeutralDensity;							///< Neutral density grid used to apply forces to keep the hair in place
	float				mDensityScale = 0.0f;						///< Highest density value in the neutral density grid, used to scale the density for rendering
	uint32				mMaxVerticesPerStrand = 0;					///< Maximum number of vertices per strand, used for padding the compute buffers

	// Compute data
	Ref<ComputeBuffer>	mScalpVerticesCB;
	Ref<ComputeBuffer>	mScalpTrianglesCB;
	Ref<ComputeBuffer>	mScalpSkinWeightsCB;
	Ref<ComputeBuffer>	mSkinPointsCB;
	Ref<ComputeBuffer>	mVerticesFixedCB;
	Ref<ComputeBuffer>	mVerticesPositionCB;
	Ref<ComputeBuffer>	mVerticesBishopCB;
	Ref<ComputeBuffer>	mVerticesOmega0CB;
	Ref<ComputeBuffer>	mVerticesLengthCB;
	Ref<ComputeBuffer>	mVerticesStrandFractionCB;
	Ref<ComputeBuffer>	mStrandVertexCountsCB;
	Ref<ComputeBuffer>	mStrandMaterialIndexCB;
	Ref<ComputeBuffer>	mNeutralDensityCB;
	Ref<ComputeBuffer>	mSVertexInfluencesCB;
};

JPH_NAMESPACE_END

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Hair/Hair.h>
#include <Jolt/Physics/Hair/HairShaders.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaleHelpers.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/Profiler.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif

JPH_NAMESPACE_BEGIN

Hair::Hair(const HairSettings *inSettings, RVec3Arg inPosition, QuatArg inRotation, ObjectLayer inLayer) :
	mSettings(inSettings),
	mPrevPosition(inPosition),
	mPosition(inPosition),
	mPrevRotation(inRotation),
	mRotation(inRotation),
	mLayer(inLayer)
{
}

Hair::~Hair()
{
	// Delete debug data
	if (mPositions != nullptr)
		delete [] mPositions;
	if (mRotations != nullptr)
		delete [] mRotations;
	if (mVelocities != nullptr)
		delete [] mVelocities;
	if (mRenderPositionsOverridden)
		delete [] mRenderPositions;
}

void Hair::Init(ComputeSystem *inComputeSystem)
{
	// Create compute buffers
	size_t num_vertices_padded = mSettings->GetNumVerticesPadded();
	size_t grid_size = mSettings->mNeutralDensity.size();
	size_t num_render_vertices = mSettings->mRenderVertices.size();

	if (!mSettings->mScalpInverseBindPose.empty() && !mSettings->mScalpVertices.empty())
	{
		mScalpJointMatricesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, mSettings->mScalpInverseBindPose.size() * sizeof(Mat44), sizeof(Mat44)).Get();
		mScalpVerticesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, mSettings->mScalpVertices.size(), sizeof(Float3)).Get();
		mScalpTrianglesCB = mSettings->mScalpTrianglesCB;
	}

	if (mScalpVerticesCB != nullptr)
	{
		mGlobalPoseTransformsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, mSettings->mSimStrands.size(), sizeof(JPH_HairGlobalPoseTransform)).Get();
	}
	else
	{
		// No vertices provided externally and none in settings, use identity transforms
		JPH_HairGlobalPoseTransform identity;
		identity.mPosition = JPH_float3(0, 0, 0);
		identity.mRotation = JPH_float4(0, 0, 0, 1);
		Array<JPH_HairGlobalPoseTransform> identity_array(mSettings->mSimStrands.size(), identity);
		mGlobalPoseTransformsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, mSettings->mSimStrands.size(), sizeof(JPH_HairGlobalPoseTransform), identity_array.data()).Get();
	}

	mCollisionPlanesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_vertices_padded, sizeof(JPH_HairCollisionPlane)).Get();
	mMaterialsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, mSettings->mMaterials.size(), sizeof(JPH_HairMaterial)).Get();
	mPreviousPositionsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_vertices_padded, sizeof(JPH_HairPosition)).Get();
	mPositionsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_vertices_padded, sizeof(JPH_HairPosition)).Get();
	mVelocitiesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_vertices_padded, sizeof(JPH_HairVelocity)).Get();
	mConstantsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::ConstantBuffer, 1, sizeof(JPH_HairUpdateContext)).Get();
	mVelocityAndDensityCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, grid_size, sizeof(Float4)).Get();
	if (!mRenderPositionsOverridden)
		mRenderPositionsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, num_render_vertices, sizeof(Float3)).Get();
}

void Hair::InitializeContext(UpdateContext &outCtx, float inDeltaTime, const PhysicsSystem &inSystem)
{
	float clamped_delta_time = min(inDeltaTime, mSettings->mMaxDeltaTime);
	outCtx.mNumIterations = (uint)std::round(clamped_delta_time * mSettings->mNumIterationsPerSecond);
	outCtx.mDeltaTime = outCtx.mNumIterations > 0? clamped_delta_time / outCtx.mNumIterations : 0.0f;
	outCtx.mTimeRatio = outCtx.mDeltaTime * float(HairSettings::cDefaultIterationsPerSecond);
	outCtx.mHalfDeltaTime = 0.5f * outCtx.mDeltaTime;
	outCtx.mInvDeltaTimeSq = outCtx.mDeltaTime > 0.0f? 1.0f / Square(outCtx.mDeltaTime) : 1.0e12f;
	outCtx.mTwoDivDeltaTime = outCtx.mDeltaTime > 0.0f? 2.0f / outCtx.mDeltaTime : 1.0e12f;
	outCtx.mSubStepGravity = (mRotation.Conjugated() * inSystem.GetGravity()) * outCtx.mDeltaTime;

	// Calculate delta transform from previous to current position and rotation
	outCtx.mHasTransformChanged = mPosition != mPrevPosition || mRotation != mPrevRotation;
	RMat44 prev_com = RMat44::sRotationTranslation(mPrevRotation, mPrevPosition);
	outCtx.mDeltaTransform = (GetWorldTransform().InversedRotationTranslation() * prev_com).ToMat44();
	outCtx.mDeltaTransformQuat = outCtx.mDeltaTransform.GetQuaternion();
	mPrevPosition = mPosition;
	mPrevRotation = mRotation;

	// Check if we need collision detection / grid
	outCtx.mNeedsCollision = false;
	outCtx.mNeedsGrid = false;
	outCtx.mGlobalPoseOnly = true;
	for (const HairSettings::Material &material : mSettings->mMaterials)
	{
		outCtx.mNeedsCollision |= material.mEnableCollision;
		outCtx.mNeedsGrid |= material.NeedsGrid();
		outCtx.mGlobalPoseOnly &= material.GlobalPoseOnly();
	}

	if (outCtx.mNeedsCollision)
	{
		struct Collector : public CollideShapeBodyCollector
		{
										Collector(const PhysicsSystem &inSystem, RMat44Arg inTransform, const AABox &inLocalBounds, Array<LeafShape> &ioHits) :
											mSystem(inSystem),
											mTransform(inTransform),
											mInverseTransform(inTransform.InversedRotationTranslation()),
											mLocalBounds(inLocalBounds),
											mHits(ioHits)
			{
			}

			virtual void				AddHit(const BodyID &inResult) override
			{
				BodyLockRead lock(mSystem.GetBodyLockInterface(), inResult);
				if (lock.Succeeded())
				{
					const Body &body = lock.GetBody();
					if (body.IsRigidBody()
						&& !body.IsSensor())
					{
						// Calculate transform of this body relative to the hair instance
						Mat44 com = (mInverseTransform * body.GetCenterOfMassTransform()).ToMat44();

						// Collect leaf shapes
						struct LeafShapeCollector : public TransformedShapeCollector
						{
												LeafShapeCollector(RMat44Arg inHeadTransform, const Body &inBody, Array<LeafShape> &ioHits) : mHeadTransform(inHeadTransform), mBody(inBody), mHits(ioHits) { }

							virtual void		AddHit(const TransformedShape &inResult) override
							{
								mHits.emplace_back(Mat44::sRotationTranslation(inResult.mShapeRotation, Vec3(inResult.mShapePositionCOM)),
									inResult.GetShapeScale(),
									mHeadTransform.Multiply3x3Transposed(mBody.GetPointVelocity(mHeadTransform * inResult.mShapePositionCOM)), // Calculate velocity of shape at its center of mass position
									mHeadTransform.Multiply3x3Transposed(mBody.GetAngularVelocity()),
									inResult.mShape);
							}

							RMat44				mHeadTransform;
							const Body &		mBody;
							Array<LeafShape> &	mHits;
						};
						LeafShapeCollector collector(mTransform, body, mHits);
						body.GetShape()->CollectTransformedShapes(mLocalBounds, com.GetTranslation(), com.GetQuaternion(), Vec3::sOne(), SubShapeIDCreator(), collector, { });
					}
				}
			}

		private:
			const PhysicsSystem &		mSystem;
			RMat44						mTransform;
			RMat44						mInverseTransform;
			AABox						mLocalBounds;
			Array<LeafShape> &			mHits;
		};

		// Calculate world space bounding box
		RMat44 transform = GetWorldTransform();
		AABox world_bounds = mSettings->mSimulationBounds.Transformed(transform);

		// Collect shapes that intersect with the bounding box
		Collector collector(inSystem, transform, mSettings->mSimulationBounds, outCtx.mShapes);
		DefaultBroadPhaseLayerFilter broadphase_layer_filter = inSystem.GetDefaultBroadPhaseLayerFilter(mLayer);
		DefaultObjectLayerFilter object_layer_filter = inSystem.GetDefaultLayerFilter(mLayer);
		inSystem.GetBroadPhaseQuery().CollideAABox(world_bounds, collector, broadphase_layer_filter, object_layer_filter);

		// If no shapes were found, we don't need collision
		if (outCtx.mShapes.empty())
			outCtx.mNeedsCollision = false;
	}
}

void Hair::Update(float inDeltaTime, Mat44Arg inJointToHair, const Mat44 *inJointMatrices, const PhysicsSystem &inSystem, const HairShaders &inShaders, ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue)
{
	UpdateContext ctx;
	InitializeContext(ctx, inDeltaTime, inSystem);

	if (inJointMatrices != nullptr && mScalpJointMatricesCB != nullptr)
	{
		JPH_PROFILE("Prepare for Skinning");

		Mat44 *joints = mScalpJointMatricesCB->Map<Mat44>(ComputeBuffer::EMode::Write);
		mSettings->PrepareForScalpSkinning(inJointToHair, inJointMatrices, joints);
		mScalpJointMatricesCB->Unmap();
	}

	if (ctx.mNeedsCollision)
	{
		JPH_PROFILE("Create Collision Shapes");

		// First determine buffer sizes
		uint num_shapes = 0;
		uint num_faces = 0;
		uint num_vertices = 0;
		uint num_header = 0;
		uint num_indices = 0;
		uint max_vertices_per_face = 0;
		uint max_points = 0;
		for (const LeafShape &shape : ctx.mShapes)
			if (shape.mShape->GetSubType() == EShapeSubType::ConvexHull)
			{
				const ConvexHullShape *ch = static_cast<const ConvexHullShape *>(shape.mShape.GetPtr());
				++num_shapes;
				++num_header; // Write number of vertices
				uint np = ch->GetNumPoints();
				max_points = max(max_points, np);
				num_vertices += np;
				uint nf = ch->GetNumFaces();
				num_faces += nf;
				for (uint f = 0; f < nf; ++f)
				{
					num_header += 2; // Write indices start + end
					uint num_vertices_in_face = ch->GetNumVerticesInFace(f);
					num_indices += num_vertices_in_face;
					max_vertices_per_face = max(max_vertices_per_face, num_vertices_in_face);
				}
			}
		++num_header; // Terminator
		num_indices += num_header;

		// Now allocate buffers
		if (mCollisionShapesCB == nullptr || mCollisionShapesCB->GetSize() < num_shapes)
		{
			mCollisionShapesCB = nullptr;
			mCollisionShapesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, num_shapes, sizeof(JPH_HairCollisionShape)).Get();
		}
		if (mShapePlanesCB == nullptr || mShapePlanesCB->GetSize() < num_faces)
		{
			mShapePlanesCB = nullptr;
			mShapePlanesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, max(num_faces, 1u), sizeof(Float4)).Get();
		}
		if (mShapeVerticesCB == nullptr || mShapeVerticesCB->GetSize() < num_vertices)
		{
			mShapeVerticesCB = nullptr;
			mShapeVerticesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, max(num_vertices, 1u), sizeof(Float3)).Get();
		}
		if (mShapeIndicesCB == nullptr || mShapeIndicesCB->GetSize() < num_indices)
		{
			mShapeIndicesCB = nullptr;
			mShapeIndicesCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::UploadBuffer, num_indices, sizeof(uint32)).Get();
		}

		JPH_HairCollisionShape *collision_shapes = mCollisionShapesCB->Map<JPH_HairCollisionShape>(ComputeBuffer::EMode::Write);
		Float4 *shape_planes = mShapePlanesCB->Map<Float4>(ComputeBuffer::EMode::Write);
		Float3 *shape_vertices = mShapeVerticesCB->Map<Float3>(ComputeBuffer::EMode::Write);
		uint32 *shape_indices = mShapeIndicesCB->Map<uint32>(ComputeBuffer::EMode::Write);
		uint *face_indices = (uint *)JPH_STACK_ALLOC(max_vertices_per_face * sizeof(uint));
		Vec3 *points = (Vec3 *)JPH_STACK_ALLOC(max_points * sizeof(Vec3));

		// Convert the hulls to compute buffers
		Float4 *sp = shape_planes;
		Float3 *sv = shape_vertices;
		uint32 *sh = shape_indices;
		JPH_HairCollisionShape *cs = collision_shapes;
		uint32 *si = shape_indices + num_header;
		for (const LeafShape &shape : ctx.mShapes)
			if (shape.mShape->GetSubType() == EShapeSubType::ConvexHull)
			{
				const ConvexHullShape *ch = static_cast<const ConvexHullShape *>(shape.mShape.GetPtr());

				// Store collision shape
				shape.mTransform.GetTranslation().StoreFloat3(&cs->mCenterOfMass);
				shape.mLinearVelocity.StoreFloat3(&cs->mLinearVelocity);
				shape.mAngularVelocity.StoreFloat3(&cs->mAngularVelocity);
				++cs;

				// Store points transformed to hair space
				Mat44 shape_transform = shape.mTransform.PreScaled(shape.mScale);
				uint first_vertex_index = uint(sv - shape_vertices);
				for (uint p = 0, np = ch->GetNumPoints(); p < np; ++p)
				{
					Vec3 v = shape_transform * ch->GetPoint(p);
					points[p] = v; // Store points in a temporary buffer so we avoid reading from GPU memory
					v.StoreFloat3(sv);
					++sv;
				}

				// Store number of faces
				uint nf = ch->GetNumFaces();
				*sh = nf;
				++sh;

				// Store the indices
				if (ScaleHelpers::IsInsideOut(shape.mScale))
				{
					// Reverse winding order
					for (uint f = 0; f < nf; ++f)
					{
						// Store indices
						uint nv = ch->GetFaceVertices(f, max_vertices_per_face, face_indices);
						uint32 indices_start = uint32(si - shape_indices);
						*sh = indices_start;
						++sh;
						*sh = indices_start + nv;
						++sh;
						for (int v = int(nv) - 1; v >= 0; --v, ++si)
							*si = face_indices[v] + first_vertex_index;

						// Calculate plane (avoids reading from GPU memory)
						Plane::sFromPointsCCW(points[face_indices[2]], points[face_indices[1]], points[face_indices[0]]).StoreFloat4(sp);
						++sp;
					}
				}
				else
				{
					// Keep winding order
					for (uint f = 0; f < nf; ++f)
					{
						// Store indices
						uint nv = ch->GetFaceVertices(f, max_vertices_per_face, face_indices);
						uint32 indices_start = uint32(si - shape_indices);
						*sh++ = indices_start;
						*sh++ = indices_start + nv;
						for (uint v = 0; v < nv; ++v)
							*si++ = face_indices[v] + first_vertex_index;

						// Calculate plane (avoids reading from GPU memory)
						Plane::sFromPointsCCW(points[face_indices[0]], points[face_indices[1]], points[face_indices[2]]).StoreFloat4(sp);
						++sp;
					}
				}
			}
		*sh = 0; // Terminator
		++sh;
		JPH_ASSERT(uint(cs - collision_shapes) == num_shapes);
		JPH_ASSERT(uint(sp - shape_planes) == num_faces);
		JPH_ASSERT(uint(sv - shape_vertices) == num_vertices);
		JPH_ASSERT(uint(sh - shape_indices) == num_header);
		JPH_ASSERT(uint(si - shape_indices) == num_indices);

		// Unmap buffers
		mCollisionShapesCB->Unmap();
		mShapePlanesCB->Unmap();
		mShapeVerticesCB->Unmap();
		mShapeIndicesCB->Unmap();
	}

	{
		JPH_PROFILE("Set materials");

		JPH_HairMaterial *materials = mMaterialsCB->Map<JPH_HairMaterial>(ComputeBuffer::EMode::Write);
		for (size_t i = 0, n = mSettings->mMaterials.size(); i < n; ++i)
		{
			const HairSettings::Material &m_in = mSettings->mMaterials[i];
			JPH_HairMaterial &m_out = materials[i];

			GradientSampler world_transform_influence(m_in.mWorldTransformInfluence);
			m_out.mWorldTransformInfluence = world_transform_influence.ToFloat4();
			GradientSampler global_pose(ctx.mGlobalPoseOnly? m_in.mGlobalPose : m_in.mGlobalPose.MakeStepDependent(ctx.mTimeRatio));
			m_out.mGlobalPose = global_pose.ToFloat4();
			GradientSampler global_pose_skin_to_root(m_in.mSkinGlobalPose);
			m_out.mSkinGlobalPose = global_pose_skin_to_root.ToFloat4();
			GradientSampler gravity_factor(m_in.mGravityFactor);
			m_out.mGravityFactor = gravity_factor.ToFloat4();
			GradientSampler hair_radius(m_in.mHairRadius);
			m_out.mHairRadius = hair_radius.ToFloat4();
			m_out.mBendComplianceMultiplier = m_in.mBendComplianceMultiplier;
			GradientSampler grid_velocity_factor(m_in.mGridVelocityFactor.MakeStepDependent(ctx.mTimeRatio));
			m_out.mGridVelocityFactor = grid_velocity_factor.ToFloat4();
			m_out.mEnableCollision = ctx.mNeedsCollision && m_in.mEnableCollision? 1 : 0;
			m_out.mEnableLRA = m_in.mEnableLRA? 1 : 0;
			m_out.mEnableGrid = m_in.mGridVelocityFactor.mMin != 0.0f || m_in.mGridVelocityFactor.mMax != 0.0f || m_in.mGridDensityForceFactor != 0.0f;
			m_out.mFriction = m_in.mFriction;
			m_out.mExpLinearDampingDeltaTime = std::exp(-m_in.mLinearDamping * ctx.mDeltaTime);
			m_out.mExpAngularDampingDeltaTime = std::exp(-m_in.mAngularDamping * ctx.mDeltaTime);
			m_out.mBendComplianceInvDeltaTimeSq = m_in.mBendCompliance * ctx.mInvDeltaTimeSq;
			m_out.mStretchComplianceInvDeltaTimeSq = m_in.mStretchCompliance * ctx.mInvDeltaTimeSq;
			m_out.mGridDensityForceFactor = m_in.mGridDensityForceFactor;
			m_out.mInertiaMultiplier = m_in.mInertiaMultiplier;
			m_out.mMaxLinearVelocitySq = Square(m_in.mMaxLinearVelocity);
			m_out.mMaxAngularVelocitySq = Square(m_in.mMaxAngularVelocity);
		}
		mMaterialsCB->Unmap();
	}

	{
		JPH_PROFILE("Set constants");

		JPH_HairUpdateContext *cdata = mConstantsCB->Map<JPH_HairUpdateContext>(ComputeBuffer::EMode::Write);
		cdata->cNumStrands = uint32(mSettings->mSimStrands.size());
		cdata->cNumVertices = mSettings->GetNumVerticesPadded();
		cdata->cNumGridPoints = (uint32)mSettings->mNeutralDensity.size();
		cdata->cNumRenderVertices = (uint)mSettings->mRenderVertices.size();
		HairSettings::GridSampler grid_sampler(mSettings);
		memcpy(&cdata->cGridSizeMin2, &grid_sampler.mGridSizeMin2, 3 * sizeof(float));
		cdata->cTwoDivDeltaTime = ctx.mTwoDivDeltaTime;
		grid_sampler.mGridSizeMin1.StoreFloat3(&cdata->cGridSizeMin1);
		cdata->cDeltaTime = ctx.mDeltaTime;
		grid_sampler.mOffset.StoreFloat3(&cdata->cGridOffset);
		cdata->cHalfDeltaTime = ctx.mHalfDeltaTime;
		grid_sampler.mScale.StoreFloat3(&cdata->cGridScale);
		cdata->cInvDeltaTimeSq = ctx.mInvDeltaTimeSq;
		ctx.mSubStepGravity.StoreFloat3(&cdata->cSubStepGravity);
		cdata->cNumSkinVertices = (uint)mSettings->mScalpVertices.size();
		memcpy(&cdata->cGridStride, &grid_sampler.mGridStride, 3 * sizeof(uint32));
		cdata->cNumSkinWeightsPerVertex = mSettings->mScalpNumSkinWeightsPerVertex;
		for (int i = 0; i < 4; ++i)
			ctx.mDeltaTransform.GetColumn4(i).StoreFloat4(&cdata->cDeltaTransform[i]);
		for (int i = 0; i < 4; ++i)
			mScalpToHead.GetColumn4(i).StoreFloat4(&cdata->cScalpToHead[i]);
		ctx.mDeltaTransformQuat.StoreFloat4(&cdata->cDeltaTransformQuat);
		mConstantsCB->Unmap();
	}

	{
		JPH_PROFILE("Set iteration constants");

		// Ensure that we have the right number of constant buffers allocated
		uint old_size = uint(mIterationConstantsCB.size());
		if (old_size < ctx.mNumIterations)
		{
			mIterationConstantsCB.resize(ctx.mNumIterations);
			for (uint i = old_size; i < ctx.mNumIterations; ++i)
				mIterationConstantsCB[i] = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::ConstantBuffer, 1, sizeof(JPH_HairIterationContext)).Get();
		}

		// Fill in the constant buffers
		JPH_HairIterationContext iteration_data;
		for (uint i = 0; i < ctx.mNumIterations; ++i)
		{
			iteration_data.cAccumulatedDeltaTime = ctx.mDeltaTime * (i + 1);
			iteration_data.cIterationFraction = 1.0f / float(ctx.mNumIterations - i);

			JPH_HairIterationContext *idata = mIterationConstantsCB[i]->Map<JPH_HairIterationContext>(ComputeBuffer::EMode::Write);
			*idata = iteration_data;
			mIterationConstantsCB[i]->Unmap();
		}
	}

	{
		JPH_PROFILE("Queue Compute");

		uint dispatch_per_vertex = (mSettings->GetNumVerticesPadded() + cHairPerVertexBatch - 1) / cHairPerVertexBatch;
		uint dispatch_per_vertex_skip_first_vertex = (mSettings->GetNumVerticesPadded() - (uint)mSettings->mSimStrands.size() + cHairPerVertexBatch - 1) / cHairPerVertexBatch; // Skip the first vertex of each strand
		uint dispatch_per_grid_cell = uint((mSettings->mNeutralDensity.size() + cHairPerGridCellBatch - 1) / cHairPerGridCellBatch);
		uint dispatch_per_strand = uint((mSettings->mSimStrands.size() + cHairPerStrandBatch - 1) / cHairPerStrandBatch);
		uint dispatch_per_render_vertex = uint((mSettings->mRenderVertices.size() + cHairPerRenderVertexBatch - 1) / cHairPerRenderVertexBatch);

		bool was_teleported = mTeleported;
		mTeleported = false;
		if (was_teleported)
		{
			// Initialize positions and velocities
			inComputeQueue->SetShader(inShaders.mTeleportCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
			inComputeQueue->SetBuffer("gInitialBishops", mSettings->mVerticesBishopCB);
			inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
			inComputeQueue->SetRWBuffer("gVelocities", mVelocitiesCB);
			inComputeQueue->Dispatch(dispatch_per_vertex);
		}
		else if (!ctx.mGlobalPoseOnly && ctx.mHasTransformChanged)
		{
			// Apply delta transform
			inComputeQueue->SetShader(inShaders.mApplyDeltaTransformCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
			inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
			inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
			inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
			inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
			inComputeQueue->SetRWBuffer("gVelocities", mVelocitiesCB);
			inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);
		}

		if (mScalpJointMatricesCB != nullptr)
		{
			// Skin the scalp mesh
			inComputeQueue->SetShader(inShaders.mSkinVerticesCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gScalpVertices", mSettings->mScalpVerticesCB);
			inComputeQueue->SetBuffer("gScalpSkinWeights", mSettings->mScalpSkinWeightsCB);
			inComputeQueue->SetBuffer("gScalpJointMatrices", mScalpJointMatricesCB);
			inComputeQueue->SetRWBuffer("gScalpVerticesOut", mScalpVerticesCB);
			inComputeQueue->Dispatch(uint((mSettings->mScalpVertices.size() + cHairPerVertexBatch - 1) / cHairPerVertexBatch));
		}

		if (mScalpVerticesCB != nullptr)
		{
			// Determine if we directly write to the position / transform buffers or if we need to interpolate
			bool needs_interpolate = !ctx.mGlobalPoseOnly && !was_teleported;

			// Create target buffers if they don't exist yet
			if (mTargetPositionsCB == nullptr && needs_interpolate)
			{
				mTargetPositionsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, mSettings->mSimStrands.size(), sizeof(JPH_HairPosition)).Get();
				mTargetGlobalPoseTransformsCB = inComputeSystem->CreateComputeBuffer(ComputeBuffer::EType::RWBuffer, mSettings->mSimStrands.size(), sizeof(JPH_HairGlobalPoseTransform)).Get();
			}

			// Skin the strand roots to the scalp mesh
			inComputeQueue->SetShader(inShaders.mSkinRootsCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gSkinPoints", mSettings->mSkinPointsCB);
			inComputeQueue->SetBuffer("gScalpVertices", mScalpVerticesCB);
			inComputeQueue->SetBuffer("gScalpTriangles", mScalpTrianglesCB);
			inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
			inComputeQueue->SetBuffer("gInitialBishops", mSettings->mVerticesBishopCB);
			inComputeQueue->SetRWBuffer("gPositions", needs_interpolate? mTargetPositionsCB : mPositionsCB);
			inComputeQueue->SetRWBuffer("gGlobalPoseTransforms", needs_interpolate? mTargetGlobalPoseTransformsCB : mGlobalPoseTransformsCB);
			inComputeQueue->Dispatch(dispatch_per_strand);
		}

		if (ctx.mGlobalPoseOnly)
		{
			// Only run global pose logic
			inComputeQueue->SetShader(inShaders.mApplyGlobalPoseCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
			inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
			inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
			inComputeQueue->SetBuffer("gInitialBishops", mSettings->mVerticesBishopCB);
			inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
			inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
			inComputeQueue->SetBuffer("gGlobalPoseTransforms", mGlobalPoseTransformsCB);
			inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
			inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);
		}
		else if (ctx.mNumIterations > 0)
		{
			if (ctx.mNeedsCollision)
			{
				// Calculate collision planes
				inComputeQueue->SetShader(inShaders.mCalculateCollisionPlanesCS);
				inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
				inComputeQueue->SetBuffer("gPositions", mPositionsCB);
				inComputeQueue->SetBuffer("gShapePlanes", mShapePlanesCB);
				inComputeQueue->SetBuffer("gShapeVertices", mShapeVerticesCB);
				inComputeQueue->SetBuffer("gShapeIndices", mShapeIndicesCB);
				inComputeQueue->SetRWBuffer("gCollisionPlanes", mCollisionPlanesCB);
				inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);
			}

			if (ctx.mNeedsGrid)
			{
				// Clear the grid
				inComputeQueue->SetShader(inShaders.mGridClearCS);
				inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
				inComputeQueue->SetRWBuffer("gVelocityAndDensity", mVelocityAndDensityCB);
				inComputeQueue->Dispatch(dispatch_per_grid_cell);

				// Accumulate vertices into the grid
				inComputeQueue->SetShader(inShaders.mGridAccumulateCS);
				inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
				inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
				inComputeQueue->SetBuffer("gPositions", mPositionsCB);
				inComputeQueue->SetBuffer("gVelocities", mVelocitiesCB);
				inComputeQueue->SetRWBuffer("gVelocityAndDensity", mVelocityAndDensityCB);
				inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);

				// Normalize velocities in the grid
				inComputeQueue->SetShader(inShaders.mGridNormalizeCS);
				inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
				inComputeQueue->SetRWBuffer("gVelocityAndDensity", mVelocityAndDensityCB);
				inComputeQueue->Dispatch(dispatch_per_grid_cell);
			}

			// First integrate
			inComputeQueue->SetShader(inShaders.mIntegrateCS);
			inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
			inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
			inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
			inComputeQueue->SetBuffer("gNeutralDensity", mSettings->mNeutralDensityCB);
			inComputeQueue->SetBuffer("gVelocityAndDensity", mVelocityAndDensityCB);
			inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
			inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
			inComputeQueue->SetBuffer("gVelocities", mVelocitiesCB);
			inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
			inComputeQueue->SetRWBuffer("gPreviousPositions", mPreviousPositionsCB);
			inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);

			for (uint it = 0; it < ctx.mNumIterations; ++it)
			{
				if (mTargetPositionsCB != nullptr && !was_teleported)
				{
					// Update skinned roots for this iteration (interpolate them towards the target positions)
					inComputeQueue->SetShader(inShaders.mUpdateRootsCS);
					inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
					inComputeQueue->SetConstantBuffer("gIterationContext", mIterationConstantsCB[it]);
					inComputeQueue->SetBuffer("gTargetPositions", mTargetPositionsCB);
					inComputeQueue->SetBuffer("gTargetGlobalPoseTransforms", mTargetGlobalPoseTransformsCB);
					inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
					inComputeQueue->SetRWBuffer("gGlobalPoseTransforms", mGlobalPoseTransformsCB);
					inComputeQueue->Dispatch(dispatch_per_strand);
				}

				// Then update the constraints per strand
				inComputeQueue->SetShader(inShaders.mUpdateStrandsCS);
				inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
				inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
				inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
				inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
				inComputeQueue->SetBuffer("gOmega0s", mSettings->mVerticesOmega0CB);
				inComputeQueue->SetBuffer("gInitialLengths", mSettings->mVerticesLengthCB);
				inComputeQueue->SetBuffer("gStrandVertexCounts", mSettings->mStrandVertexCountsCB);
				inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
				inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
				inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
				inComputeQueue->Dispatch(dispatch_per_strand);

				if (it == ctx.mNumIterations - 1)
				{
					// Last iteration: only update velocities
					inComputeQueue->SetShader(inShaders.mUpdateVelocityCS);
					inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
					inComputeQueue->SetConstantBuffer("gIterationContext", mIterationConstantsCB[it]);
					inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
					inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
					inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
					inComputeQueue->SetBuffer("gInitialBishops", mSettings->mVerticesBishopCB);
					inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
					inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
					inComputeQueue->SetBuffer("gPreviousPositions", mPreviousPositionsCB);
					inComputeQueue->SetBuffer("gGlobalPoseTransforms", mGlobalPoseTransformsCB);
					inComputeQueue->SetBuffer("gCollisionShapes", mCollisionShapesCB);
					inComputeQueue->SetBuffer("gCollisionPlanes", mCollisionPlanesCB);
					inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
					inComputeQueue->SetRWBuffer("gVelocities", mVelocitiesCB);
					inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);
				}
				else
				{
					// Other iterations: update velocities then integrate again
					inComputeQueue->SetShader(inShaders.mUpdateVelocityIntegrateCS);
					inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
					inComputeQueue->SetConstantBuffer("gIterationContext", mIterationConstantsCB[it]);
					inComputeQueue->SetBuffer("gVerticesFixed", mSettings->mVerticesFixedCB);
					inComputeQueue->SetBuffer("gStrandFractions", mSettings->mVerticesStrandFractionCB);
					inComputeQueue->SetBuffer("gInitialPositions", mSettings->mVerticesPositionCB);
					inComputeQueue->SetBuffer("gInitialBishops", mSettings->mVerticesBishopCB);
					inComputeQueue->SetBuffer("gNeutralDensity", mSettings->mNeutralDensityCB);
					inComputeQueue->SetBuffer("gVelocityAndDensity", mVelocityAndDensityCB);
					inComputeQueue->SetBuffer("gStrandMaterialIndex", mSettings->mStrandMaterialIndexCB);
					inComputeQueue->SetBuffer("gMaterials", mMaterialsCB);
					inComputeQueue->SetBuffer("gGlobalPoseTransforms", mGlobalPoseTransformsCB);
					inComputeQueue->SetBuffer("gCollisionShapes", mCollisionShapesCB);
					inComputeQueue->SetBuffer("gCollisionPlanes", mCollisionPlanesCB);
					inComputeQueue->SetRWBuffer("gPreviousPositions", mPreviousPositionsCB);
					inComputeQueue->SetRWBuffer("gPositions", mPositionsCB);
					inComputeQueue->Dispatch(dispatch_per_vertex_skip_first_vertex);
				}
			}
		}

		// Remap simulation positions to render positions
		inComputeQueue->SetShader(inShaders.mCalculateRenderPositionsCS);
		inComputeQueue->SetConstantBuffer("gContext", mConstantsCB);
		inComputeQueue->SetBuffer("gSVertexInfluences", mSettings->mSVertexInfluencesCB);
		inComputeQueue->SetBuffer("gPositions", mPositionsCB);
		inComputeQueue->SetRWBuffer("gRenderPositions", mRenderPositionsCB);
		inComputeQueue->Dispatch(dispatch_per_render_vertex);
	}
}

void Hair::ReadBackGPUState(ComputeQueue *inComputeQueue)
{
	if (mPositionsReadBackCB == nullptr)
	{
		// Create read back buffers
		if (mScalpVerticesCB != nullptr)
			mScalpVerticesReadBackCB = mScalpVerticesCB->CreateReadBackBuffer().Get();
		mPositionsReadBackCB = mPositionsCB->CreateReadBackBuffer().Get();
		mVelocitiesReadBackCB = mVelocitiesCB->CreateReadBackBuffer().Get();
		mVelocityAndDensityReadBackCB = mVelocityAndDensityCB->CreateReadBackBuffer().Get();
		mRenderPositionsReadBackCB = mRenderPositionsCB->CreateReadBackBuffer().Get();
	}

	{
		JPH_PROFILE("Transfer data from GPU");

		// Read back the skinned vertices
		if (mScalpVerticesCB != nullptr)
			inComputeQueue->ScheduleReadback(mScalpVerticesReadBackCB, mScalpVerticesCB);

		// Read back the vertices
		inComputeQueue->ScheduleReadback(mPositionsReadBackCB, mPositionsCB);
		inComputeQueue->ScheduleReadback(mVelocitiesReadBackCB, mVelocitiesCB);
		inComputeQueue->ScheduleReadback(mRenderPositionsReadBackCB, mRenderPositionsCB);

		// Read back the velocity and density
		inComputeQueue->ScheduleReadback(mVelocityAndDensityReadBackCB, mVelocityAndDensityCB);

		// Wait for the compute queue to finish
		inComputeQueue->ExecuteAndWait();
	}

	{
		JPH_PROFILE("Reorder hair data");

		// Reorder position and velocity data
		const JPH_HairPosition *positions = mPositionsReadBackCB->Map<JPH_HairPosition>(ComputeBuffer::EMode::Read);
		const JPH_HairVelocity *velocities = mVelocitiesReadBackCB->Map<JPH_HairVelocity>(ComputeBuffer::EMode::Read);
		size_t num_vertices = mSettings->mSimVertices.size();
		if (mPositions == nullptr)
			mPositions = new Float3 [num_vertices];
		if (mRotations == nullptr)
			mRotations = new Quat [num_vertices];
		if (mVelocities == nullptr)
			mVelocities = new JPH_HairVelocity [num_vertices];
		uint32 num_strands = (uint32)mSettings->mSimStrands.size();
		for (uint32 s = 0; s < num_strands; ++s)
		{
			const HairSettings::SStrand &strand = mSettings->mSimStrands[s];
			for (uint32 v = 0; v < strand.VertexCount(); ++v)
			{
				uint32 in_index = s + v * num_strands;
				uint32 out_index = strand.mStartVtx + v;
				mPositions[out_index] = Float3(positions[in_index].mPosition);
				mRotations[out_index] = Quat(positions[in_index].mRotation);
				mVelocities[out_index] = velocities[in_index];
			}
		}
		mPositionsReadBackCB->Unmap();
		mVelocitiesReadBackCB->Unmap();
	}
}

void Hair::LockReadBackBuffers()
{
	if (mScalpVerticesReadBackCB != nullptr)
		mScalpVertices = mScalpVerticesReadBackCB->Map<Float3>(ComputeBuffer::EMode::Read);
	mVelocityAndDensity = mVelocityAndDensityReadBackCB->Map<Float4>(ComputeBuffer::EMode::Read);
	if (mRenderPositionsOverridden)
	{
		uint num_render_vertices = (uint)mSettings->mRenderVertices.size();
		if (mRenderPositions == nullptr)
			mRenderPositions = new Float3 [num_render_vertices];
		mRenderPositionsToFloat3(mRenderPositionsReadBackCB, const_cast<Float3 *>(mRenderPositions), num_render_vertices);
	}
	else
		mRenderPositions = mRenderPositionsReadBackCB->Map<Float3>(ComputeBuffer::EMode::Read);
}

void Hair::UnlockReadBackBuffers()
{
	if (mScalpVerticesReadBackCB != nullptr)
		mScalpVerticesReadBackCB->Unmap();
	mVelocityAndDensityReadBackCB->Unmap();
	if (!mRenderPositionsOverridden)
		mRenderPositionsReadBackCB->Unmap();
}

#ifdef JPH_DEBUG_RENDERER

void Hair::Draw(const DrawSettings &inSettings, DebugRenderer *inRenderer)
{
	LockReadBackBuffers();

	const Float3 *positions = GetPositions();
	const Float3 *render_positions = GetRenderPositions();
	const Quat *rotations = GetRotations();
	StridedPtr<const Float3> velocities = GetVelocities();
	StridedPtr<const Float3> angular_velocities = GetAngularVelocities();
	const Float4 *grid_velocity_and_density = GetGridVelocityAndDensity();
	const Float3 *scalp_vertices = GetScalpVertices();

	float arrow_size = 0.01f * mSettings->mSimulationBounds.GetSize().ReduceMin();
	RMat44 com = GetWorldTransform();

	// Draw the render strands
	if (inSettings.mDrawRenderStrands)
	{
		JPH_PROFILE("Draw Render Strands");

		// Calculate a map of sim vertex index to strand index
		Array<uint> sim_vertex_to_strand;
		sim_vertex_to_strand.resize(mSettings->mSimVertices.size(), 0);
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
		{
			const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
			for (uint v = strand.mStartVtx; v < strand.mEndVtx; ++v)
				sim_vertex_to_strand[v] = i;
		}

		Hash<uint32> hasher;
		switch (inSettings.mRenderStrandColor)
		{
		case ERenderStrandColor::PerRenderStrand:
			{
				Color color = Color::sGreen;
				for (const HairSettings::RStrand &strand : mSettings->mRenderStrands)
				{
					uint32 strand_idx = sim_vertex_to_strand[mSettings->mRenderVertices[strand.mStartVtx].mInfluences[0].mVertexIndex];
					if (strand_idx >= inSettings.mSimulationStrandBegin && strand_idx < inSettings.mSimulationStrandEnd)
					{
						RVec3 x0 = com * Vec3(render_positions[strand.mStartVtx]);
						for (uint32 v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
						{
							RVec3 x1 = com * Vec3(render_positions[v]);
							inRenderer->DrawLine(x0, x1, color);
							x0 = x1;
						}
						color = Color(uint32(hasher(color.GetUInt32())) | 0xff000000);
					}
				}
			}
			break;

		case ERenderStrandColor::PerSimulatedStrand:
			for (const HairSettings::RStrand &strand : mSettings->mRenderStrands)
			{
				uint32 strand_idx = sim_vertex_to_strand[mSettings->mRenderVertices[strand.mStartVtx].mInfluences[0].mVertexIndex];
				if (strand_idx >= inSettings.mSimulationStrandBegin && strand_idx < inSettings.mSimulationStrandEnd)
				{
					Color color = Color(uint32(hasher(strand_idx)) | 0xff000000);
					RVec3 x0 = com * Vec3(render_positions[strand.mStartVtx]);
					for (uint32 v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
					{
						RVec3 x1 = com * Vec3(render_positions[v]);
						inRenderer->DrawLine(x0, x1, color);
						x0 = x1;
					}
				}
			}
			break;

		case ERenderStrandColor::GravityFactor:
		case ERenderStrandColor::WorldTransformInfluence:
		case ERenderStrandColor::GridVelocityFactor:
		case ERenderStrandColor::GlobalPose:
		case ERenderStrandColor::SkinGlobalPose:
			for (const HairSettings::RStrand &strand : mSettings->mRenderStrands)
			{
				uint32 strand_idx = sim_vertex_to_strand[mSettings->mRenderVertices[strand.mStartVtx].mInfluences[0].mVertexIndex];
				const HairSettings::Material &material = mSettings->mMaterials[mSettings->mSimStrands[strand_idx].mMaterialIndex];

				// Prepare sampler
				GradientSampler sampler;
				if (inSettings.mRenderStrandColor == ERenderStrandColor::GravityFactor)
					sampler = GradientSampler(material.mGravityFactor);
				else if (inSettings.mRenderStrandColor == ERenderStrandColor::WorldTransformInfluence)
					sampler = GradientSampler(material.mWorldTransformInfluence);
				else if (inSettings.mRenderStrandColor == ERenderStrandColor::GridVelocityFactor)
					sampler = GradientSampler(material.mGridVelocityFactor);
				else if (inSettings.mRenderStrandColor == ERenderStrandColor::GlobalPose)
					sampler = GradientSampler(material.mGlobalPose);
				else
					sampler = GradientSampler(material.mSkinGlobalPose);

				if (strand_idx >= inSettings.mSimulationStrandBegin && strand_idx < inSettings.mSimulationStrandEnd)
				{
					RVec3 x0 = com * Vec3(render_positions[strand.mStartVtx]);
					for (uint32 v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
					{
						RVec3 x1 = com * Vec3(render_positions[v]);
						uint32 simulated_vtx = mSettings->mRenderVertices[v].mInfluences[0].mVertexIndex;
						float factor = sampler.Sample(mSettings->mSimVertices[simulated_vtx].mStrandFraction);
						inRenderer->DrawLine(x0, x1, Color::sGreenRedGradient(factor));
						x0 = x1;
					}
				}
			}
			break;
		}
	}

	// Draw the rods
	if (inSettings.mDrawRods)
	{
		JPH_PROFILE("Draw Rods");

		Color color = Color::sRed;
		Hash<uint32> hasher;
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
				RVec3 x0 = com * Vec3(positions[strand.mStartVtx]);
				for (uint32 v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
				{
					RVec3 x1 = com * Vec3(positions[v]);
					inRenderer->DrawLine(x0, x1, color);
					x0 = x1;
				}
				color = Color(uint32(hasher(color.GetUInt32())) | 0xff000000);
			}
	}

	// Draw the rods in their unloaded pose
	if (inSettings.mDrawUnloadedRods)
	{
		JPH_PROFILE("Draw Unloaded Rods");

		Color color = Color::sYellow;
		Hash<uint32> hasher;
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
				RVec3 x0 = com * Vec3(positions[strand.mStartVtx]);
				Quat rotation = mRotation * rotations[strand.mStartVtx];
				for (uint32 v = strand.mStartVtx + 1; v < strand.mEndVtx; ++v)
				{
					RVec3 x1 = x0 + rotation.RotateAxisZ() * mSettings->mSimVertices[v - 1].mLength;
					inRenderer->DrawLine(x0, x1, color);
					rotation = (rotation * Quat(mSettings->mSimVertices[v].mOmega0)).Normalized();
					x0 = x1;
				}
				color = Color(uint32(hasher(color.GetUInt32())) | 0xff000000);
			}
	}

	// Draw vertex velocities
	if (inSettings.mDrawVertexVelocity)
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
				for (uint32 v = strand.mStartVtx; v < strand.mEndVtx; ++v)
				{
					Vec3 velocity(velocities[v]);
					if (velocity.LengthSq() > 1.0e-6f)
					{
						Vec3 pos = Vec3(positions[v]);
						inRenderer->DrawArrow(com * pos, com * (pos + velocity), Color::sGreen, arrow_size);
					}
				}
			}

	// Draw angular velocities
	if (inSettings.mDrawAngularVelocity)
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
				for (uint32 v = strand.mStartVtx; v < strand.mEndVtx; ++v)
				{
					Vec3 angular_velocity(angular_velocities[v]);
					if (angular_velocity.LengthSq() > 1.0e-6f)
					{
						Vec3 pos = Vec3(positions[v]);
						inRenderer->DrawArrow(com * pos, com * (pos + 0.1f * angular_velocity), Color::sOrange, arrow_size);
					}
				}
			}

	// Draw rod orientations
	if (inSettings.mDrawOrientations)
		for (uint i = 0, n = (uint)mSettings->mSimStrands.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SStrand &strand = mSettings->mSimStrands[i];
				for (uint32 v = strand.mStartVtx; v < strand.mEndVtx; ++v)
					inRenderer->DrawCoordinateSystem(com * Mat44::sRotationTranslation(rotations[v], Vec3(positions[v])), arrow_size);
			}

	// Draw grid bounds
	if (inSettings.mDrawNeutralDensity || inSettings.mDrawGridDensity || inSettings.mDrawGridVelocity)
		inRenderer->DrawWireBox(com, mSettings->mSimulationBounds, Color::sGrey);

	// Draw neutral density
	if (inSettings.mDrawNeutralDensity)
	{
		Vec3 offset = mSettings->mSimulationBounds.mMin;
		Vec3 scale = mSettings->mSimulationBounds.GetSize() / Vec3(mSettings->mGridSize.ToFloat());
		float marker_size = 0.5f * scale.ReduceMin();
		for (uint32 z = 0; z < mSettings->mGridSize.GetX(); ++z)
			for (uint32 y = 0; y < mSettings->mGridSize.GetY(); ++y)
				for (uint32 x = 0; x < mSettings->mGridSize.GetZ(); ++x)
				{
					float density = mSettings->GetNeutralDensity(x, y, z);
					JPH_ASSERT(density >= 0.0f);
					if (density > 0.0f)
					{
						Vec3 pos = offset + Vec3(UVec4(x, y, z, 0).ToFloat()) * scale;
						inRenderer->DrawMarker(com * pos, Color::sGreenRedGradient(density * mSettings->mDensityScale), marker_size);
					}
				}
	}

	// Draw current density
	if (inSettings.mDrawGridDensity || inSettings.mDrawGridVelocity)
	{
		Vec3 offset = mSettings->mSimulationBounds.mMin;
		Vec3 scale = mSettings->mSimulationBounds.GetSize() / Vec3(mSettings->mGridSize.ToFloat());
		float marker_size = 0.5f * scale.ReduceMin();
		for (uint32 z = 0; z < mSettings->mGridSize.GetX(); ++z)
			for (uint32 y = 0; y < mSettings->mGridSize.GetY(); ++y)
				for (uint32 x = 0; x < mSettings->mGridSize.GetZ(); ++x)
				{
					const Float4 &velocity_and_density = grid_velocity_and_density[x + y * mSettings->mGridSize.GetX() + z * mSettings->mGridSize.GetX() * mSettings->mGridSize.GetY()];
					float density = velocity_and_density.w;
					Vec3 velocity = Vec3::sLoadFloat3Unsafe((const Float3 &)velocity_and_density);
					if (density > 0.0f)
					{
						RVec3 pos = com * (offset + Vec3(UVec4(x, y, z, 0).ToFloat()) * scale);
						if (inSettings.mDrawGridDensity)
							inRenderer->DrawMarker(pos, Color::sGreenRedGradient(density * mSettings->mDensityScale), marker_size);
						if (inSettings.mDrawGridVelocity && velocity.LengthSq() > 1.0e-6f)
							inRenderer->DrawArrow(pos, pos + com.Multiply3x3(velocity), Color::sYellow, arrow_size);
					}
				}
	}

	if (inSettings.mDrawSkinPoints)
		for (uint i = 0, n = (uint)mSettings->mSkinPoints.size(); i < n; ++i)
			if (i >= inSettings.mSimulationStrandBegin && i < inSettings.mSimulationStrandEnd)
			{
				const HairSettings::SkinPoint &sp = mSettings->mSkinPoints[i];
				const IndexedTriangleNoMaterial &tri = mSettings->mScalpTriangles[sp.mTriangleIndex];
				RVec3 v0 = com * Vec3(scalp_vertices[tri.mIdx[0]]);
				RVec3 v1 = com * Vec3(scalp_vertices[tri.mIdx[1]]);
				RVec3 v2 = com * Vec3(scalp_vertices[tri.mIdx[2]]);
				inRenderer->DrawWireTriangle(v0, v1, v2, Color::sYellow);

				RVec3 point = Real(sp.mU) * v0 + Real(sp.mV) * v1 + Real(1.0f - sp.mU - sp.mV) * v2;
				Vec3 tangent = Vec3(v1 - v0).Normalized();
				Vec3 normal = tangent.Cross(Vec3(v2 - v0)).Normalized();
				Vec3 binormal = tangent.Cross(normal);
				RMat44 basis(Vec4(normal, 0), Vec4(binormal, 0), Vec4(tangent, 0), point);
				inRenderer->DrawCoordinateSystem(basis, 0.01f);
			}

	// Draw initial gravity
	if (inSettings.mDrawInitialGravity)
		inRenderer->DrawArrow(com.GetTranslation(), com * mSettings->mInitialGravity, Color::sBlue, 0.05f * mSettings->mInitialGravity.Length());

	UnlockReadBackBuffers();
}

#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_END

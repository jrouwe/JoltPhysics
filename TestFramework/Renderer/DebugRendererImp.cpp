// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DebugRendererImp.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderPrimitive.h>
#include <Renderer/Texture.h>
#include <Renderer/Font.h>

#ifndef JPH_DEBUG_RENDERER
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.cpp>
	#undef JPH_DEBUG_RENDERER
#endif // !JPH_DEBUG_RENDERER

DebugRendererImp::DebugRendererImp(Renderer *inRenderer, const Font *inFont) :
	mRenderer(inRenderer),
	mFont(inFont)
{
	// Create input layout for lines
	const D3D12_INPUT_ELEMENT_DESC line_vertex_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Lines
	ComPtr<ID3DBlob> vtx_line = mRenderer->CreateVertexShader("Assets/Shaders/LineVertexShader.hlsl");
	ComPtr<ID3DBlob> pix_line = mRenderer->CreatePixelShader("Assets/Shaders/LinePixelShader.hlsl");
	mLineState = mRenderer->CreatePipelineState(vtx_line.Get(), line_vertex_desc, ARRAYSIZE(line_vertex_desc), pix_line.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);

	// Create input layout for triangles
	const D3D12_INPUT_ELEMENT_DESC triangles_vertex_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_INV_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_INV_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_INV_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_INV_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 128, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
	};

	// Triangles
	ComPtr<ID3DBlob> vtx_triangle = mRenderer->CreateVertexShader("Assets/Shaders/TriangleVertexShader.hlsl");
	ComPtr<ID3DBlob> pix_triangle  = mRenderer->CreatePixelShader("Assets/Shaders/TrianglePixelShader.hlsl");
	mTriangleStateBF = mRenderer->CreatePipelineState(vtx_triangle.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_triangle.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);
	mTriangleStateFF = mRenderer->CreatePipelineState(vtx_triangle.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_triangle.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::FrontFace);
	mTriangleStateWire = mRenderer->CreatePipelineState(vtx_triangle.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_triangle.Get(), D3D12_FILL_MODE_WIREFRAME, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);

	// Shadow pass
	ComPtr<ID3DBlob> vtx_shadow = mRenderer->CreateVertexShader("Assets/Shaders/TriangleDepthVertexShader.hlsl");
	ComPtr<ID3DBlob> pix_shadow = mRenderer->CreatePixelShader("Assets/Shaders/TriangleDepthPixelShader.hlsl");
	mShadowStateBF = mRenderer->CreatePipelineState(vtx_shadow.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_shadow.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);
	mShadowStateFF = mRenderer->CreatePipelineState(vtx_shadow.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_shadow.Get(), D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::FrontFace);
	mShadowStateWire = mRenderer->CreatePipelineState(vtx_shadow.Get(), triangles_vertex_desc, ARRAYSIZE(triangles_vertex_desc), pix_shadow.Get(), D3D12_FILL_MODE_WIREFRAME, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, PipelineState::EDepthTest::On, PipelineState::EBlendMode::AlphaBlend, PipelineState::ECullMode::Backface);

	// Create depth only texture (no color buffer, as seen from light)
	mDepthTexture = mRenderer->CreateRenderTarget(4096, 4096);

	// Create instances buffer
	for (uint n = 0; n < Renderer::cFrameCount; ++n)
		mInstancesBuffer[n] = new RenderInstances(mRenderer);

	// Create empty batch
	Vertex empty_vertex { Float3(0, 0, 0), Float3(1, 0, 0), Float2(0, 0), Color::sWhite };
	uint32 empty_indices[] = { 0, 0, 0 };
	mEmptyBatch = CreateTriangleBatch(&empty_vertex, 1, empty_indices, 3);

	// Initialize base class
	DebugRenderer::Initialize();
}

void DebugRendererImp::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor)
{
	RVec3 offset = mRenderer->GetBaseOffset();

	Line line;
	Vec3(inFrom - offset).StoreFloat3(&line.mFrom);
	line.mFromColor = inColor;
	Vec3(inTo - offset).StoreFloat3(&line.mTo);
	line.mToColor = inColor;

	lock_guard lock(mLinesLock);
	mLines.push_back(line);
}

DebugRenderer::Batch DebugRendererImp::CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount)
{
	if (inTriangles == nullptr || inTriangleCount == 0)
		return mEmptyBatch;

	BatchImpl *primitive = new BatchImpl(mRenderer, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	primitive->CreateVertexBuffer(3 * inTriangleCount, sizeof(Vertex), inTriangles);

	return primitive;
}

DebugRenderer::Batch DebugRendererImp::CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount)
{
	if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
		return mEmptyBatch;

	BatchImpl *primitive = new BatchImpl(mRenderer, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	primitive->CreateVertexBuffer(inVertexCount, sizeof(Vertex), inVertices);
	primitive->CreateIndexBuffer(inIndexCount, inIndices);

	return primitive;
}

void DebugRendererImp::DrawGeometry(RMat44Arg inModelMatrix, const AABox &inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	lock_guard lock(mPrimitivesLock);

	RVec3 offset = mRenderer->GetBaseOffset();

	Mat44 model_matrix = inModelMatrix.PostTranslated(-offset).ToMat44();
	AABox world_space_bounds = inWorldSpaceBounds;
	world_space_bounds.Translate(Vec3(-offset));

	// Our pixel shader uses alpha only to turn on/off shadows
	Color color = inCastShadow == ECastShadow::On? Color(inModelColor, 255) : Color(inModelColor, 0);

	if (inDrawMode == EDrawMode::Wireframe)
	{
		mWireframePrimitives[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
		++mNumInstances;
	}
	else
	{
		if (inCullMode != ECullMode::CullFrontFace)
		{
			mPrimitives[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
			++mNumInstances;
		}

		if (inCullMode != ECullMode::CullBackFace)
		{
			mPrimitivesBackFacing[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
			++mNumInstances;
		}
	}
}

void DebugRendererImp::FinalizePrimitive()
{
	JPH_PROFILE_FUNCTION();

	if (mLockedPrimitive != nullptr)
	{
		BatchImpl *primitive = static_cast<BatchImpl *>(mLockedPrimitive.GetPtr());

		// Unlock the primitive
		primitive->UnlockVertexBuffer();

		// Set number of indices to draw
		primitive->SetNumVtxToDraw(int(mLockedVertices - mLockedVerticesStart));

		// Add to draw list
		mTempPrimitives[new Geometry(mLockedPrimitive, mLockedPrimitiveBounds)].mInstances.push_back({ Mat44::sIdentity(), Mat44::sIdentity(), Color::sWhite, mLockedPrimitiveBounds, 1.0f });
		++mNumInstances;

		// Clear pointers
		mLockedPrimitive = nullptr;
		mLockedVerticesStart = nullptr;
		mLockedVertices = nullptr;
		mLockedVerticesEnd = nullptr;
		mLockedPrimitiveBounds = AABox();
	}
}

void DebugRendererImp::EnsurePrimitiveSpace(int inVtxSize)
{
	const int cVertexBufferSize = 10240;

	if (mLockedPrimitive == nullptr
		|| mLockedVerticesEnd - mLockedVertices < inVtxSize)
	{
		FinalizePrimitive();

		// Create new
		BatchImpl *primitive = new BatchImpl(mRenderer, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		primitive->CreateVertexBuffer(cVertexBufferSize, sizeof(Vertex));
		mLockedPrimitive = primitive;

		// Lock buffers
		mLockedVerticesStart = mLockedVertices = (Vertex *)primitive->LockVertexBuffer();
		mLockedVerticesEnd = mLockedVertices + cVertexBufferSize;
	}
}

void DebugRendererImp::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow)
{
	RVec3 offset = mRenderer->GetBaseOffset();

	Vec3 v1(inV1 - offset);
	Vec3 v2(inV2 - offset);
	Vec3 v3(inV3 - offset);

	lock_guard lock(mPrimitivesLock);

	EnsurePrimitiveSpace(3);

	// Set alpha to zero if we don't want to cast shadows to notify the pixel shader
	Color color(inColor, inCastShadow == ECastShadow::Off? 0 : 0xff);

	// Construct triangle
	new ((Triangle *)mLockedVertices) Triangle(v1, v2, v3, color);
	mLockedVertices += 3;

	// Update bounding box
	mLockedPrimitiveBounds.Encapsulate(v1);
	mLockedPrimitiveBounds.Encapsulate(v2);
	mLockedPrimitiveBounds.Encapsulate(v3);
}

void DebugRendererImp::DrawInstances(const Geometry *inGeometry, const Array<int> &inStartIdx)
{
	RenderInstances *instances_buffer = mInstancesBuffer[mRenderer->GetCurrentFrameIndex()];

	if (!inStartIdx.empty())
	{
		// Get LODs
		const Array<LOD> &geometry_lods = inGeometry->mLODs;

		// Write instances for all LODS
		int next_start_idx = inStartIdx.front();
		for (size_t lod = 0; lod < geometry_lods.size(); ++lod)
		{
			int start_idx = next_start_idx;
			next_start_idx = inStartIdx[lod + 1];
			int num_instances = next_start_idx - start_idx;
			instances_buffer->Draw(static_cast<BatchImpl *>(geometry_lods[lod].mTriangleBatch.GetPtr()), start_idx, num_instances);
		}
	}
}

void DebugRendererImp::DrawText3D(RVec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight)
{
	RVec3 offset = mRenderer->GetBaseOffset();

	Vec3 pos(inPosition - offset);

	lock_guard lock(mTextsLock);
	mTexts.emplace_back(pos, inString, inColor, inHeight);
}

void DebugRendererImp::DrawLines()
{
	JPH_PROFILE_FUNCTION();

	lock_guard lock(mLinesLock);

	// Draw the lines
	if (!mLines.empty())
	{
		RenderPrimitive primitive(mRenderer, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		primitive.CreateVertexBuffer((int)mLines.size() * 2, sizeof(Line) / 2);
		void *data = primitive.LockVertexBuffer();
		memcpy(data, &mLines[0], mLines.size() * sizeof(Line));
		primitive.UnlockVertexBuffer();
		mLineState->Activate();
		primitive.Draw();
	}
}

void DebugRendererImp::DrawTriangles()
{
	JPH_PROFILE_FUNCTION();

	lock_guard lock(mPrimitivesLock);

	// Finish the last primitive
	FinalizePrimitive();

	// Render to shadow map texture first
	mRenderer->SetRenderTarget(mDepthTexture);

	// Clear the shadow map texture to max depth
	mDepthTexture->ClearRenderTarget();

	// Get the camera and light frustum for culling
	Vec3 camera_pos(mRenderer->GetCameraState().mPos - mRenderer->GetBaseOffset());
	const Frustum &camera_frustum = mRenderer->GetCameraFrustum();
	const Frustum &light_frustum = mRenderer->GetLightFrustum();

	// Resize instances buffer and copy all visible instance data into it
	if (mNumInstances > 0)
	{
		// Create instances buffer
		RenderInstances *instances_buffer = mInstancesBuffer[mRenderer->GetCurrentFrameIndex()];
		instances_buffer->CreateBuffer(2 * mNumInstances, sizeof(Instance));
		Instance *dst_instance = reinterpret_cast<Instance *>(instances_buffer->Lock());

		// Next write index
		int dst_index = 0;

		// This keeps track of which instances use which lod, first array: 0 = light pass, 1 = geometry pass
		Array<Array<int>> lod_indices[2];

		for (InstanceMap *primitive_map : { &mPrimitives, &mTempPrimitives, &mPrimitivesBackFacing, &mWireframePrimitives })
			for (InstanceMap::value_type &v : *primitive_map)
			{
				// Get LODs
				const Array<LOD> &geometry_lods = v.first->mLODs;
				size_t num_lods = geometry_lods.size();
				JPH_ASSERT(num_lods > 0);

				// Ensure that our lod index array is big enough (to avoid reallocating memory too often)
				if (lod_indices[0].size() < num_lods)
					lod_indices[0].resize(num_lods);
				if (lod_indices[1].size() < num_lods)
					lod_indices[1].resize(num_lods);

				// Iterate over all instances
				const Array<InstanceWithLODInfo> &instances = v.second.mInstances;
				for (size_t i = 0; i < instances.size(); ++i)
				{
					const InstanceWithLODInfo &src_instance = instances[i];

					// Check if it overlaps with the light or camera frustum
					bool light_overlaps = light_frustum.Overlaps(src_instance.mWorldSpaceBounds);
					bool camera_overlaps = camera_frustum.Overlaps(src_instance.mWorldSpaceBounds);
					if (light_overlaps || camera_overlaps)
					{
						// Figure out which LOD to use
						const LOD &lod = v.first->GetLOD(camera_pos, src_instance.mWorldSpaceBounds, src_instance.mLODScaleSq);
						size_t lod_index = &lod - geometry_lods.data();

						// Store which index goes in which LOD
						if (light_overlaps)
							lod_indices[0][lod_index].push_back((int)i);
						if (camera_overlaps)
							lod_indices[1][lod_index].push_back((int)i);
					}
				}

				// Loop over both passes: 0 = light, 1 = geometry
				Array<int> *start_idx[] = { &v.second.mLightStartIdx, &v.second.mGeometryStartIdx };
				for (int type = 0; type < 2; ++type)
				{
					// Reserve space for instance indices
					Array<int> &type_start_idx = *start_idx[type];
					type_start_idx.resize(num_lods + 1);

					// Write out geometry pass instances
					for (size_t lod = 0; lod < num_lods; ++lod)
					{
						// Write start index for this LOD
						type_start_idx[lod] = dst_index;

						// Copy instances
						Array<int> &this_lod_indices = lod_indices[type][lod];
						for (int i : this_lod_indices)
						{
							const Instance &src_instance = instances[i];
							dst_instance[dst_index++] = src_instance;
						}

						// Prepare for next iteration (will preserve memory)
						this_lod_indices.clear();
					}

					// Write out end of last LOD
					type_start_idx.back() = dst_index;
				}
			}

		instances_buffer->Unlock();
	}

	if (!mPrimitives.empty() || !mTempPrimitives.empty())
	{
		// Front face culling, we want to render the back side of the geometry for casting shadows
		mShadowStateFF->Activate();

		// Draw all primitives as seen from the light
		if (mNumInstances > 0)
			for (InstanceMap::value_type &v : mPrimitives)
				DrawInstances(v.first, v.second.mLightStartIdx);
		for (InstanceMap::value_type &v : mTempPrimitives)
			DrawInstances(v.first, v.second.mLightStartIdx);
	}

	if (!mPrimitivesBackFacing.empty())
	{
		// Back face culling, we want to render the front side of back facing geometry
		mShadowStateBF->Activate();

		// Draw all primitives as seen from the light
		for (InstanceMap::value_type &v : mPrimitivesBackFacing)
			DrawInstances(v.first, v.second.mLightStartIdx);
	}

	if (!mWireframePrimitives.empty())
	{
		// Switch to wireframe mode
		mShadowStateWire->Activate();

		// Draw all wireframe primitives as seen from the light
		for (InstanceMap::value_type &v : mWireframePrimitives)
			DrawInstances(v.first, v.second.mLightStartIdx);
	}

	// Switch to the main render target
	mRenderer->SetRenderTarget(nullptr);

	// Bind the shadow map texture
	mDepthTexture->Bind(2);

	if (!mPrimitives.empty() || !mTempPrimitives.empty())
	{
		// Bind the normal shader, back face culling
		mTriangleStateBF->Activate();

		// Draw all primitives
		if (mNumInstances > 0)
			for (InstanceMap::value_type &v : mPrimitives)
				DrawInstances(v.first, v.second.mGeometryStartIdx);
		for (InstanceMap::value_type &v : mTempPrimitives)
			DrawInstances(v.first, v.second.mGeometryStartIdx);
	}

	if (!mPrimitivesBackFacing.empty())
	{
		// Front face culling, the next batch needs to render inside out
		mTriangleStateFF->Activate();

		// Draw all back primitives
		for (InstanceMap::value_type &v : mPrimitivesBackFacing)
			DrawInstances(v.first, v.second.mGeometryStartIdx);
	}

	if (!mWireframePrimitives.empty())
	{
		// Wire frame mode
		mTriangleStateWire->Activate();

		// Draw all wireframe primitives
		for (InstanceMap::value_type &v : mWireframePrimitives)
			DrawInstances(v.first, v.second.mGeometryStartIdx);
	}
}

void DebugRendererImp::DrawTexts()
{
	lock_guard lock(mTextsLock);

	JPH_PROFILE_FUNCTION();

	const CameraState &camera_state = mRenderer->GetCameraState();

	for (const Text &t : mTexts)
	{
		Vec3 forward = camera_state.mForward;
		Vec3 right = forward.Cross(camera_state.mUp).Normalized();
		Vec3 up = right.Cross(forward).Normalized();
		Mat44 transform(Vec4(right, 0), Vec4(up, 0), Vec4(forward, 0), Vec4(t.mPosition, 1));

		mFont->DrawText3D(transform * Mat44::sScale(t.mHeight), t.mText, t.mColor);
	}
}

void DebugRendererImp::Draw()
{
	DrawLines();
	DrawTriangles();
	DrawTexts();
}

void DebugRendererImp::ClearLines()
{
	lock_guard lock(mLinesLock);
	mLines.clear();
}

void DebugRendererImp::ClearMap(InstanceMap &ioInstances)
{
	Array<GeometryRef> to_delete;

	for (InstanceMap::value_type &kv : ioInstances)
	{
		if (kv.second.mInstances.empty())
			to_delete.push_back(kv.first);
		else
			kv.second.mInstances.clear();
	}

	for (GeometryRef &b : to_delete)
		ioInstances.erase(b);
}

void DebugRendererImp::ClearTriangles()
{
	lock_guard lock(mPrimitivesLock);

	// Close any primitive that's being built
	FinalizePrimitive();

	// Move primitives to draw back to the free list
	ClearMap(mWireframePrimitives);
	ClearMap(mPrimitives);
	mTempPrimitives.clear(); // These are created by FinalizePrimitive() and need to be cleared every frame
	ClearMap(mPrimitivesBackFacing);
	mNumInstances = 0;
}

void DebugRendererImp::ClearTexts()
{
	lock_guard lock(mTextsLock);
	mTexts.clear();
}

void DebugRendererImp::Clear()
{
	ClearLines();
	ClearTriangles();
	ClearTexts();
	NextFrame();
}

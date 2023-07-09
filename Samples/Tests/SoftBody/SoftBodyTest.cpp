// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodyTest.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodyTest) 
{ 
	JPH_ADD_BASE_CLASS(SoftBodyTest, Test) 
}

class SoftBodySettings
{
public:
	void				CalculateRestLengths()
	{
		for (Edge &e : mEdges)
			e.mRestLength = (Vec3(mVertices[e.mVertex[1]].mPosition) - Vec3(mVertices[e.mVertex[0]].mPosition)).Length();
	}

	struct Vertex
	{
		Float3			mPosition;
		float			mInvMass;
	};

	struct Edge
	{
		uint32			mVertex[2];
		float			mRestLength = 1.0f;
		float			mCompliance = 0.0f;
	};

	Array<Vertex>		mVertices;
	Array<Edge>			mEdges;
};

class SoftBody
{
public:
						SoftBody(const SoftBodySettings &inSettings);

	void				Update(float inDeltaTime, Vec3Arg inGravity, float inLinearDamping, uint inNumIterations);
	void				Draw(DebugRenderer *inRenderer) const;

	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		float			mInvMass;
	};

	using Edge = SoftBodySettings::Edge;

	Array<Vertex>		mVertices;
	Array<Edge>			mEdges;
};

SoftBody::SoftBody(const SoftBodySettings &inSettings)
{
	mVertices.resize(inSettings.mVertices.size());
	for (Array<Vertex>::size_type v = 0; v < mVertices.size(); ++v)
	{
		const SoftBodySettings::Vertex &in_vertex = inSettings.mVertices[v];
		Vertex &out_vertex = mVertices[v];
		out_vertex.mPreviousPosition = out_vertex.mPosition = Vec3(in_vertex.mPosition);
		out_vertex.mVelocity = Vec3::sZero();
		out_vertex.mInvMass = in_vertex.mInvMass;
	}

	mEdges = inSettings.mEdges;
}

void SoftBody::Update(float inDeltaTime, Vec3Arg inGravity, float inLinearDamping, uint inNumIterations)
{
	float dt = inDeltaTime / inNumIterations;
	float inv_dt_sq = 1.0f / Square(dt);

	for (uint iteration = 0; iteration < inNumIterations; ++iteration)
	{
		// Integrate
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
			{
				// Gravity
				v.mVelocity += inGravity * dt;

				// Damping
				v.mVelocity *= max(0.0f, 1.0f - inLinearDamping * dt);

				// Integrate
				v.mPreviousPosition = v.mPosition;
				v.mPosition += v.mVelocity * dt;
			}

		// Satisfy constraints
		for (const Edge &e : mEdges)
		{
			Vertex &v0 = mVertices[e.mVertex[0]];
			Vertex &v1 = mVertices[e.mVertex[1]];

			// Calculate current length
			Vec3 delta = v1.mPosition - v0.mPosition;
			float length = delta.Length();

			// Apply correction
			Vec3 correction = delta * (length - e.mRestLength) / (length * (v0.mInvMass + v1.mInvMass + e.mCompliance * inv_dt_sq));
			v0.mPosition += v0.mInvMass * correction;
			v1.mPosition -= v1.mInvMass * correction;
		}

		// Update velocity
		for (Vertex &v : mVertices)
			if (v.mInvMass > 0.0f)
				v.mVelocity = (v.mPosition - v.mPreviousPosition) / dt;
	}
}

void SoftBody::Draw(DebugRenderer *inRenderer) const
{
	for (const Vertex &v : mVertices)
		inRenderer->DrawMarker(v.mPosition, Color::sRed, 0.05f);

	for (const Edge &e : mEdges)
		inRenderer->DrawLine(mVertices[e.mVertex[0]].mPosition, mVertices[e.mVertex[1]].mPosition, Color::sWhite);
}

static SoftBody *sSoftBody;

SoftBodyTest::~SoftBodyTest()
{
	delete sSoftBody;
	sSoftBody = nullptr;
}

void SoftBodyTest::Initialize()
{
	// Floor
	CreateFloor();

	const uint cGridSize = 20;
	const float cGridSpacing = 1.0f;

	// Create settings
	SoftBodySettings settings;
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Vertex v;
			v.mPosition = Float3(x * cGridSpacing, 10.0f, y * cGridSpacing);
			v.mInvMass = 1.0f;
			settings.mVertices.push_back(v);
		}

	// Fixate corners
	settings.mVertices[0].mInvMass = 0.0f;
	settings.mVertices[cGridSize - 1].mInvMass = 0.0f;
	settings.mVertices[(cGridSize - 1) * cGridSize].mInvMass = 0.0f;
	settings.mVertices[(cGridSize - 1) * cGridSize + cGridSize - 1].mInvMass = 0.0f;

	// Create edges
	for (uint y = 0; y < cGridSize; ++y)
		for (uint x = 0; x < cGridSize; ++x)
		{
			SoftBodySettings::Edge e;
			e.mCompliance = 0.0001f;
			e.mVertex[0] = x + y * cGridSize;
			if (x < cGridSize - 1)
			{
				e.mVertex[1] = x + 1 + y * cGridSize;
				settings.mEdges.push_back(e);
			}
			if (y < cGridSize - 1)
			{
				e.mVertex[1] = x + (y + 1) * cGridSize;
				settings.mEdges.push_back(e);
			}
			if (x < cGridSize - 1 && y < cGridSize - 1)
			{
				e.mVertex[1] = x + 1 + (y + 1) * cGridSize;
				settings.mEdges.push_back(e);

				e.mVertex[0] = x + 1 + y * cGridSize;
				e.mVertex[1] = x + (y + 1) * cGridSize;
				settings.mEdges.push_back(e);
			}
		}
	settings.CalculateRestLengths();

	// Create soft body
	sSoftBody = new SoftBody(settings);
}

void SoftBodyTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	sSoftBody->Update(1.0f / 60.0f, Vec3(0.0f, -9.8f, 0.0f), 0.05f, 5);

	sSoftBody->Draw(DebugRenderer::sInstance);
}

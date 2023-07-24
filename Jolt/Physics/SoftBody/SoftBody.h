// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/Plane.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/SoftBody/SoftBodyParticleSettings.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif // JPH_DEBUG_RENDERER

/// This class contains the runtime information of a soft body. Soft bodies are implemented using XPBD, a particle and springs based approach.
class JPH_EXPORT SoftBody : public Body
{
public:
	/// Update the soft body
	void				Update(float inDeltaTime, PhysicsSystem &inSystem);

#ifdef JPH_DEBUG_RENDERER
	/// Draw the state of a soft body
	void				DrawVertices(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const;
	void				DrawEdgeConstraints(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const;
	void				DrawVolumeConstraints(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const;
	void				DrawPredictedBounds(DebugRenderer *inRenderer, Mat44Arg inCenterOfMassTransform) const;
#endif // JPH_DEBUG_RENDERER

	/// Run time information for a single particle
	struct Vertex
	{
		Vec3			mPreviousPosition;
		Vec3 			mPosition;
		Vec3 			mVelocity;
		Plane			mCollisionPlane;					///< Nearest collision plane
		int				mCollidingShapeIndex;				///< Index in the colliding shapes list of the body we may collide with
		float			mInvMass;
		float			mProjectedDistance;
	};

	using Edge = SoftBodyParticleSettings::Edge;
	using Face = SoftBodyParticleSettings::Face;
	using Volume = SoftBodyParticleSettings::Volume;

	RefConst<SoftBodyParticleSettings> mSettings;
	Array<Vertex>		mVertices;							///< Current state of all vertices in the simulation
	AABox				mLocalPredictedBounds;				///< Predicted bounding box for all vertices using extrapolation of velocity by last step delta time (relative to mPosition)
	uint32				mNumIterations;						///< Number of solver iterations
	float				mPressure;							///< n * R * T, amount of substance * ideal gass constant * absolute temperature, see https://en.wikipedia.org/wiki/Pressure
	bool				mUpdatePosition;					///< Update the position of the body while simulating (set to false for something that is attached to the static world)
};

JPH_NAMESPACE_END

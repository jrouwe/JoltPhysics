// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Hair/HairSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Core/StridedPtr.h>
#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;
#ifdef JPH_DEBUG_RENDERER
class DebugRenderer;
#endif
class HairShaders;

/// Hair simulation instance
///
/// Note that this system is currently still in development, it is missing important features like:
///
/// - Level of detail
/// - Wind forces
/// - Advection step for the grid velocity field
/// - Support for collision detection against shapes other than ConvexHullShape
/// - The Gradient class is very limited and will be replaced by a texture lookup
/// - Gravity preload factor is not fully functioning yet
/// - It is wasteful of memory (e.g. stores everything both on CPU and GPU)
/// - Only supports a single neutral pose to drive towards
/// - It could use further optimizations
class JPH_EXPORT Hair : public NonCopyable
{
public:
	/// Constructor / destructor
										Hair(const HairSettings *inSettings, RVec3Arg inPosition, QuatArg inRotation, ObjectLayer inLayer);
										~Hair();

	/// Initialize
	void								Init(ComputeSystem *inComputeSystem);

	/// Position and rotation of the hair in world space
	void								SetPosition(RVec3Arg inPosition)				{ mPosition = inPosition; }
	void								SetRotation(QuatArg inRotation)					{ mRotation = inRotation; }
	RMat44								GetWorldTransform() const						{ return RMat44::sRotationTranslation(mRotation, mPosition); }

	/// Access to the hair settings object which contains the configuration of the hair
	const HairSettings *				GetHairSettings() const							{ return mSettings; }

	/// The hair will be initialized in its default pose with zero velocity at the new position and rotation during the next update
	void								OnTeleported()									{ mTeleported = true; }

	/// Ability to externally provide the scalp vertices buffer. This allows skipping skinning the scalp during the simulation update. You may need to override JPH_SHADER_BIND_SCALP_VERTICES in HairSkinRootsBindings.h to match the format of the provided buffer.
	void								SetScalpVerticesCB(ComputeBuffer *inBuffer)		{ mScalpVerticesCB = inBuffer; }

	/// Ability to externally provide the scalp triangle indices buffer. This allows skipping skinning the scalp in during the simulation update. You may need to override JPH_SHADER_BIND_SCALP_TRIANGLES in HairSkinRootsBindings.h to match the format of the provided buffer.
	void								SetScalpTrianglesCB(ComputeBuffer *inBuffer)	{ mScalpTrianglesCB = inBuffer; }

	/// When skipping skinning, this allow specifying a transform that transforms the scalp mesh into head space.
	void								SetScalpToHead(Mat44Arg inMat)					{ mScalpToHead = inMat; }

	/// Function that converts the render positions buffer to Float3 vertices for debugging purposes. It maps an application defined format to Float3. Third parameter is the number of vertices.
	using RenderPositionsToFloat3 = std::function<void(ComputeBuffer *, Float3 *, uint)>;

	/// Enable externally set render vertices buffer (with potentially different vertex layout). Note that this also requires replacing the HairCalculateRenderPositions shader.
	void								OverrideRenderPositionsCB(const RenderPositionsToFloat3 &inRenderPositionsToFloat3) { JPH_ASSERT(mRenderPositionsCB == nullptr, "Must be called before Init"); mRenderPositionsOverridden = true; mRenderPositionsToFloat3 = inRenderPositionsToFloat3; }

	/// Allow setting the render vertices buffer externally in case it has special requirements for the calling application. You may need to override JPH_SHADER_BIND_RENDER_POSITIONS in HairCalculateRenderPositionsBindings.h to match the format of the provided buffer.
	void								SetRenderPositionsCB(ComputeBuffer *inBuffer)	{ JPH_ASSERT(mRenderPositionsOverridden, "Must call OverrideRenderPositionsCB first"); mRenderPositionsCB = inBuffer; }

	/// Step the hair simulation forward in time
	/// @param inDeltaTime Time step
	/// @param inJointToHair Transform that transforms from joint space to hair local space (as defined by GetWorldTransform)
	/// @param inJointMatrices Array of joint matrices in world space, length needs to match HairSettings::mScalpInverseBindPose.size()
	/// @param inSystem Physics system used for collision detection
	/// @param inShaders Preloaded hair compute shaders
	/// @param inComputeSystem Compute system to use
	/// @param inComputeQueue Compute queue to use
	void								Update(float inDeltaTime, Mat44Arg inJointToHair, const Mat44 *inJointMatrices, const PhysicsSystem &inSystem, const HairShaders &inShaders, ComputeSystem *inComputeSystem, ComputeQueue *inComputeQueue);

	/// Access to the resulting simulation data
	ComputeBuffer *						GetScalpVerticesCB() const						{ return mScalpVerticesCB; }		///< Skinned scalp vertices
	ComputeBuffer *						GetScalpTrianglesCB() const						{ return mScalpTrianglesCB; }		///< Skinned scalp triangle indices
	ComputeBuffer *						GetPositionsCB() const							{ return mPositionsCB; }			///< Note transposed for better memory access
	ComputeBuffer *						GetVelocitiesCB() const							{ return mVelocitiesCB; }			///< Note transposed for better memory access
	ComputeBuffer *						GetVelocityAndDensityCB() const					{ return mVelocityAndDensityCB; }	///< Velocity grid
	ComputeBuffer *						GetRenderPositionsCB() const					{ return mRenderPositionsCB; }		///< Render positions of the hair strands (see HairSettings::mRenderStrands to see where each strand starts and ends)

	/// Read back the GPU state so that the functions below can be used. For debugging purposes only, this is slow!
	void								ReadBackGPUState(ComputeQueue *inComputeQueue);

	/// Lock/unlock the data buffers so that the functions below return valid values.
	void								LockReadBackBuffers();
	void								UnlockReadBackBuffers();

	/// Access to the resulting simulation data (only valid when ReadBackGPUState has been called and the buffers have been locked)
	const Float3 *						GetScalpVertices() const						{ return mScalpVertices; }
	const Float3 *						GetPositions() const							{ return mPositions; }
	const Quat *						GetRotations() const							{ return mRotations; }
	StridedPtr<const Float3>			GetVelocities() const							{ return { (const Float3 *)&mVelocities->mVelocity, sizeof(JPH_HairVelocity) }; }
	StridedPtr<const Float3>			GetAngularVelocities() const					{ return { (const Float3 *)&mVelocities->mAngularVelocity, sizeof(JPH_HairVelocity) }; }
	const Float4 *						GetGridVelocityAndDensity() const				{ return mVelocityAndDensity; }
	const Float3 *						GetRenderPositions() const						{ return mRenderPositions; }

#ifdef JPH_DEBUG_RENDERER
	enum class ERenderStrandColor
	{
		PerRenderStrand,
		PerSimulatedStrand,
		GravityFactor,
		WorldTransformInfluence,
		GridVelocityFactor,
		GlobalPose,
		SkinGlobalPose,
	};

	struct DrawSettings
	{
		/// This specifies the range of simulation strands to draw, when drawing render strands we only draw the strands that belong to these simulation strands.
		uint							mSimulationStrandBegin = 0;
		uint							mSimulationStrandEnd = UINT_MAX;

		bool							mDrawRods = true;								///< Draws the simulated rods
		bool							mDrawUnloadedRods = false;						///< Draw rods in their unloaded pose. This pose is obtained by removing gravity influence from the modeled pose.
		bool							mDrawVertexVelocity = false;					///< Draws the velocity at each simulated vertex as an arrow
		bool							mDrawAngularVelocity = false;					///< Draws the angular velocity at each simulated vertex as an arrow
		bool							mDrawOrientations = false;						///< Draws a coordinate space for each simulated vertex
		bool							mDrawNeutralDensity = false;					///< Draws grid density of the hair in its neutral pose
		bool							mDrawGridDensity = false;						///< Draws the current grid density of the hair
		bool							mDrawGridVelocity = false;						///< Draws the velocity of each grid cell as an arrow
		bool							mDrawSkinPoints = false;						///< Draws the skinning points on the scalp
		bool 							mDrawRenderStrands = false;						///< Draws the render strands (slow, for debugging purposes!)
		bool 							mDrawInitialGravity = true;						///< Draws the configured initial gravity vector used to calculate the unloaded vertex positions
		ERenderStrandColor				mRenderStrandColor = ERenderStrandColor::PerSimulatedStrand; ///< Color for each strand
	};

	/// Debug functionality to draw the hair and its simulation properties
	void								Draw(const DrawSettings &inSettings, DebugRenderer *inRenderer);
#endif // JPH_DEBUG_RENDERER

protected:
	using Gradient = HairSettings::Gradient;
	using GradientSampler = HairSettings::GradientSampler;

	// Information about a colliding shape. Is always a leaf shape, compound shapes are expanded.
	struct LeafShape
	{
										LeafShape() = default;
										LeafShape(Mat44Arg inTransform, Vec3Arg inScale, Vec3Arg inLinearVelocity, Vec3Arg inAngularVelocity, const Shape *inShape) : mTransform(inTransform), mScale(inScale), mLinearVelocity(inLinearVelocity), mAngularVelocity(inAngularVelocity), mShape(inShape) { }

		Mat44							mTransform;
		Vec3							mScale;
		Vec3							mLinearVelocity;
		Vec3							mAngularVelocity;
		RefConst<Shape>					mShape;
	};

	// Internal context used during a simulation step
	struct UpdateContext
	{
		Mat44							mDeltaTransform;								// Transforms positions from the old hair transform to the new
		Quat							mDeltaTransformQuat;							// Rotation part of mDeltaTransform
		uint							mNumIterations;									// Number of iterations to run the solver for
		bool							mNeedsCollision;								// If collision detection should be performed
		bool							mNeedsGrid;										// If the grid should be calculated
		bool							mGlobalPoseOnly;								// If no simulation is needed and only the global pose needs to be applied
		bool							mHasTransformChanged;							// If the world transform has changed
		float							mDeltaTime;										// Delta time for a sub step
		float							mHalfDeltaTime;									// 0.5 * mDeltaTime
		float							mInvDeltaTimeSq;								// 1 / mDeltaTime^2
		float							mTwoDivDeltaTime;								// 2 / mDeltaTime
		float							mTimeRatio;										// Ratio between sub step delta time and default sub step delta time
		Vec3							mSubStepGravity;								// Gravity to apply in a sub step
		Array<LeafShape>				mShapes;										// List of colliding shapes
	};

	// Calculate the UpdateContext parameters
	void								InitializeContext(UpdateContext &outCtx, float inDeltaTime, const PhysicsSystem &inSystem);

	RefConst<HairSettings>				mSettings;										// Shared hair settings, must be kept alive during the lifetime of this hair instance

	RVec3								mPrevPosition;									// Position at the start of the last time step
	RVec3								mPosition;										// Current position in world space
	Quat								mPrevRotation;									// Rotation at the start of the last time step
	Quat								mRotation;										// Current rotation in world space
	bool								mTeleported = true;								// If the hair got teleported and should be set to the default pose
	ObjectLayer							mLayer;											// Layer for the hair to collide with

	Mat44								mScalpToHead = Mat44::sIdentity();				// When skipping skinning, this allow specifying a transform that transforms the scalp mesh into head space

	bool								mRenderPositionsOverridden = false;				// Indicates that the render positions buffer is provided externally
	RenderPositionsToFloat3				mRenderPositionsToFloat3;						// Function that transforms the render positions buffer to Float3 vertices for debugging purposes

	Ref<ComputeBuffer>					mScalpJointMatricesCB;
	Ref<ComputeBuffer>					mScalpVerticesCB;
	Ref<ComputeBuffer>					mScalpTrianglesCB;
	Ref<ComputeBuffer>					mTargetPositionsCB;								// Target root positions determined by skinning (where we're interpolating to, eventually written to mPositionsCB)
	Ref<ComputeBuffer>					mTargetGlobalPoseTransformsCB;					// Target global pose transforms determined by skinning (where we're interpolating to, eventually written to mGlobalPoseTransformsCB)
	Ref<ComputeBuffer>					mGlobalPoseTransformsCB;						// Current global pose transforms used for skinning the hairs
	Ref<ComputeBuffer>					mShapePlanesCB;
	Ref<ComputeBuffer>					mShapeVerticesCB;
	Ref<ComputeBuffer>					mShapeIndicesCB;
	Ref<ComputeBuffer>					mCollisionPlanesCB;
	Ref<ComputeBuffer>					mCollisionShapesCB;
	Ref<ComputeBuffer>					mMaterialsCB;
	Ref<ComputeBuffer>					mPreviousPositionsCB;
	Ref<ComputeBuffer>					mPositionsCB;
	Ref<ComputeBuffer>					mVelocitiesCB;
	Ref<ComputeBuffer>					mVelocityAndDensityCB;
	Ref<ComputeBuffer>					mConstantsCB;
	Array<Ref<ComputeBuffer>>			mIterationConstantsCB;
	Ref<ComputeBuffer>					mRenderPositionsCB;

	// Only valid after ReadBackGPUState has been called
	Ref<ComputeBuffer>					mScalpVerticesReadBackCB;
	Ref<ComputeBuffer>					mPositionsReadBackCB;
	Ref<ComputeBuffer>					mVelocitiesReadBackCB;
	Ref<ComputeBuffer>					mVelocityAndDensityReadBackCB;
	Ref<ComputeBuffer>					mRenderPositionsReadBackCB;
	const Float3 *						mScalpVertices = nullptr;
	Float3 *							mPositions = nullptr;
	Quat *								mRotations = nullptr;
	JPH_HairVelocity *					mVelocities = nullptr;
	const Float4 *						mVelocityAndDensity = nullptr;
	const Float3 *						mRenderPositions = nullptr;
};

JPH_NAMESPACE_END

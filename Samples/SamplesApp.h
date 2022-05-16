// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Application/Application.h>
#include <UI/UIManager.h>
#include <Application/DebugUI.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Skeleton/SkeletonPose.h>
#include <Tests/Test.h>
#include <Utils/ContactListenerImpl.h>
#include <Renderer/DebugRendererImp.h>
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Layers.h>

namespace JPH {
	class JobSystem;
	class TempAllocator;
};

// Application class that runs the samples
class SamplesApp : public Application
{
public:
	// Constructor / destructor
							SamplesApp();
	virtual					~SamplesApp() override;
		
	// Render the frame.
	virtual bool			RenderFrame(float inDeltaTime) override;

	// Override to specify the initial camera state (local to GetCameraPivot)
	virtual void			GetInitialCamera(CameraState &ioState) const override;

	// Override to specify a camera pivot point and orientation (world space)
	virtual Mat44			GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	// Get scale factor for this world, used to boost camera speed and to scale detail of the shadows
	virtual float			GetWorldScale() const override;

private:
	// Start running a new test
	void					StartTest(const RTTI *inRTTI);

	// Run all tests one by one
	void					RunAllTests();

	// Run the next test. Returns false when the application should exit.
	bool					NextTest();

	// Check if we've got to start the next test. Returns false when the application should exit.
	bool					CheckNextTest();

	// Create a snapshot of the physics system and save it to disc
	void					TakeSnapshot();

	// Create a snapshot of the physics system, save it to disc and immediately reload it
	void					TakeAndReloadSnapshot();

	// Probing the collision world
	RefConst<Shape>			CreateProbeShape();
	bool					CastProbe(float inProbeLength, float &outFraction, Vec3 &outPosition, BodyID &outID);

	// Shooting an object
	RefConst<Shape>			CreateShootObjectShape();
	void					ShootObject();

	// Debug functionality: firing a ball, mouse dragging
	void					UpdateDebug();

	// Draw the state of the physics system
	void					DrawPhysics();

	// Update the physics system with a fixed delta time
	void					StepPhysics(JobSystem *inJobSystem);

	// Save state of simulation
	void					SaveState(StateRecorderImpl &inStream);

	// Restore state of simulation
	void					RestoreState(StateRecorderImpl &inStream);

	// Compare current physics state with inExpectedState
	void					ValidateState(StateRecorderImpl &inExpectedState);

	// Global settings
	int						mMaxConcurrentJobs = thread::hardware_concurrency();		// How many jobs to run in parallel
	float					mUpdateFrequency = 60.0f;									// Physics update frequency
	int						mCollisionSteps = 1;										// How many collision detection steps per physics update
	int						mIntegrationSubSteps = 1;									// How many integration steps per physics update
	TempAllocator *			mTempAllocator = nullptr;									// Allocator for temporary allocations
	JobSystem *				mJobSystem = nullptr;										// The job system that runs physics jobs
	JobSystem *				mJobSystemValidating = nullptr;								// The job system to use when validating determinism
	BPLayerInterfaceImpl	mBroadPhaseLayerInterface;									// The broadphase layer interface that maps object layers to broadphase layers
	PhysicsSystem *			mPhysicsSystem = nullptr;									// The physics system that simulates the world
	ContactListenerImpl *	mContactListener = nullptr;									// Contact listener implementation
	PhysicsSettings			mPhysicsSettings;											// Main physics simulation settings

	// Drawing settings
#ifdef JPH_DEBUG_RENDERER
	bool					mDrawGetTriangles = false;									// Draw all shapes using Shape::GetTrianglesStart/Next
	bool					mDrawConstraints = false;									// If the constraints should be drawn
	bool					mDrawConstraintLimits = false;								// If the constraint limits should be drawn
	bool					mDrawConstraintReferenceFrame = false;						// If the constraint reference frames should be drawn
	BodyManager::DrawSettings mBodyDrawSettings;										// Settings for how to draw bodies from the body manager
	SkeletonPose::DrawSettings mPoseDrawSettings;										// Settings for drawing skeletal poses
#endif // JPH_DEBUG_RENDERER

	// Drawing using GetTriangles interface
	using ShapeToGeometryMap = unordered_map<RefConst<Shape>, DebugRenderer::GeometryRef>;
	ShapeToGeometryMap		mShapeToGeometry;

	// The test to run
	const RTTI *			mTestClass = nullptr;										// RTTI information for the test we're currently running
	Test *					mTest = nullptr;											// The test we're currently running
	UITextButton *			mTestSettingsButton = nullptr;								// Button that activates the menu that the test uses to configure additional settings

	// Automatic cycling through tests
	vector<const RTTI *>	mTestsToRun;												// The list of tests that are still waiting to be run
	float					mTestTimeLeft = -1.0f;										// How many seconds the test is still supposed to run
	bool					mExitAfterRunningTests = false;								// When true, the application will quit when mTestsToRun becomes empty
	UITextButton *			mNextTestButton = nullptr;									// Button that activates the next test when we're running all tests

	// Test settings
	bool					mInstallContactListener = false;							// When true, the contact listener is installed the next time the test is reset

	// State recording and determinism checks
	bool					mRecordState = false;										// When true, the state of the physics system is recorded in mPlaybackFrames every physics update
	bool					mCheckDeterminism = false;									// When true, the physics state is rolled back after every update and run again to verify that the state is the same
	vector<StateRecorderImpl> mPlaybackFrames;											// A list of recorded world states, one per physics simulation step
	enum class EPlaybackMode
	{
		Rewind,
		StepBack,
		Stop,
		StepForward,
		FastForward,
		Play
	};
	EPlaybackMode			mPlaybackMode = EPlaybackMode::Play;						// Current playback state. Indicates if we're playing or scrubbing back/forward.
	int						mCurrentPlaybackFrame = -1;									// Current playback frame

	// Which mode the probe is operating in.
	enum class EProbeMode
	{
		Pick,
		Ray,
		RayCollector,
		CollidePoint,
		CollideShape,
		CastShape,
		TransformedShape,
		GetTriangles,
		BroadPhaseRay,
		BroadPhaseBox,
		BroadPhaseSphere,
		BroadPhasePoint,
		BroadPhaseOrientedBox,
		BroadPhaseCastBox,
	};

	// Which probe shape to use.
	enum class EProbeShape
	{
		Sphere,
		Box,
		ConvexHull,
		Capsule,
		TaperedCapsule,
		Cylinder,
		Triangle,
		StaticCompound,
		StaticCompound2,
		MutableCompound,
	};

	// Probe settings
	EProbeMode				mProbeMode = EProbeMode::Pick;								// Mouse probe mode. Determines what happens under the crosshair.
	EProbeShape				mProbeShape = EProbeShape::Sphere;							// Shape to use for the mouse probe.
	bool					mScaleShape = false;										// If the shape is scaled or not. When true mShapeScale is taken into account.
	Vec3					mShapeScale = Vec3::sReplicate(1.0f);						// Scale in local space for the probe shape.
	EBackFaceMode			mBackFaceMode = EBackFaceMode::CollideWithBackFaces;		// How to handle back facing triangles when doing a collision probe check.
	EActiveEdgeMode			mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;	// How to handle active edges when doing a collision probe check.
	ECollectFacesMode		mCollectFacesMode = ECollectFacesMode::NoFaces;				// If we should collect colliding faces
	float					mMaxSeparationDistance = 0.0f;								// Max separation distance for collide shape test
	bool					mTreatConvexAsSolid = true;									// For ray casts if the shape should be treated as solid or if the ray should only collide with the surface
	bool					mReturnDeepestPoint = true;									// For shape casts, when true this will return the deepest point
	bool					mUseShrunkenShapeAndConvexRadius = false;					// Shrink then expand the shape by the convex radius
	int						mMaxHits = 10;												// The maximum number of hits to request for a collision probe.

	// Which object to shoot
	enum class EShootObjectShape
	{
		Sphere,
		ConvexHull,
		ThinBar,
	};

	// Shoot object settings
	EShootObjectShape		mShootObjectShape = EShootObjectShape::Sphere;				// Type of object to shoot
	float					mShootObjectVelocity = 20.0f;								// Speed at which objects are ejected
	EMotionQuality			mShootObjectMotionQuality = EMotionQuality::Discrete;		// Motion quality for the object that we're shooting
	float					mShootObjectFriction = 0.2f;								// Friction for the object that is shot
	float					mShootObjectRestitution = 0.0f;								// Restitution for the object that is shot
	bool					mShootObjectScaleShape = false;								// If the shape should be scaled
	Vec3					mShootObjectShapeScale = Vec3::sReplicate(1.0f);			// Scale of the object to shoot

	// Mouse dragging
	Body *					mDragAnchor = nullptr;										// A anchor point for the distance constraint. Corresponds to the current crosshair position.
	BodyID					mDragBody;													// The body ID of the body that the user is currently dragging.
	Ref<Constraint>			mDragConstraint;											// The distance constraint that connects the body to be dragged and the anchor point.
	float					mDragFraction;												// Fraction along cDragRayLength (see cpp) where the hit occurred. This will be combined with the crosshair position to get a 3d anchor point.

	// Timing
	uint					mStepNumber = 0;											// Which step number we're accumulating
	uint64					mTotalTime = 0;												// How many tick we spent
};

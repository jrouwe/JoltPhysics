# Breaking API Changes

This document lists all breaking API changes by date and by release tag. Note that not all API changes are listed here, trivial changes (that cause a compile error and require an obvious fix) are not listed.

Changes that make some state saved through SaveBinaryState from a prior version of the library unreadable by the new version is marked as *SBS*. See 'Saving Shapes' in [Architecture and API documentation](https://jrouwe.github.io/JoltPhysics/) for further information.

## Changes between v2.0.1 and latest

* 20221231 - ObjectLayerPairFilter and ObjectVsBroadPhaseLayerFilter are now objects instead of function pointers. (4315ad53e354f094f753664fcf7a52870f6915e4)
* 20221208 - ContactListener::OnContactValidate is reporting collisions relative to inBaseOffset. Add this to the contact point if you want world space positions. (428611482825e369e60e0a5daf17c69a4d0f2a6f)
* 20221204 - Changes related to double precision support for positions (a2c1c22059fa031faf0208258e654bcff79a63e4)
	* In many places in the public API Vec3 has been replaced by RVec3 (a Vec3 of Real values which can either be double or float depending on if JPH_DOUBLE_PRECISION is defined). In the same way RMat44 replaces Mat44. When compiling in single precision mode (the default) you should not notice a change.
	* Shape::GetSubmergedVolume now takes a plane that's relative to inCenterOfMassTransform instead of one in world space
	* Many of the NarrowPhaseQuery and TransformedShape collision queries now have a 'base offset' that you need to specify. Read the Big Worlds section in [Architecture and API documentation](https://jrouwe.github.io/JoltPhysics/) for more info.
	* The NarrowPhaseQuery/TransformedShape CastRay / CastShape functions now take a RRayCast / RShapeCast struct as input. When compiling in single precision mode this is the same as a RayCast or ShapeCast so only the type name needs to be updated.
	* If you implement your own TempAllocator and want to compile in double precision, make sure you align to JPH_RVECTOR_ALIGNMENT bytes (instead of 16)
	* The SkeletonPose got a 'root offset' member, this means that the ragdoll will now make the joint transform of the first body zero and put that offset in the 'root offset'.
	* ContactManifold now stores the contacts relative to mBaseOffset, the arrays containing the contact points have been renamed from mWorldSpaceContactPointsOn1/2 to mRelativeContactPointsOn1/2 to reflect this.
	* The DebugRenderer::DrawLine function now takes RVec3Arg parameters instead of Float3 parameters.
	* The format of a recording recorded with DebugRendererRecorder has changed, this invalidates any prior recordings.
* 20221128 - MotionProperties::SetMotionQuality has been removed because changing it for an active body could cause crashes. Use BodyInterface::SetMotionQuality instead. (64802d163a7336e60916365ad9bce764cec4ca70)

## Changes between v1.1.0 and v2.0.0

* 20221027 - *SBS* (vehicles only) - Rewrote engine model for wheeled vehicle. Before engine inertia was only used when the clutch was pressed, now it is always used, so you may want to use a lower value. The way torque is distributed over the wheels has also changed and may require tweaking the vehicle parameters. (5ac751cee9afcc097fd4f884308f5e4dc9fdaeaf)
* 20220903 - *SBS* - Added overrides for number of position/velocity solver iterations. Only affects serialization. (38ec33942ead4968a83409bd13d868f60e6397c4)
* 20220826 - *SBS* - Removed FixedConstraintSettings and SliderConstraintSettings SetPoint functions. If you were calling this function replace it by setting mAutoDetectPoint = true. (d16a0b05bfeed42b1618e3774a9c953e6922d22b)
* 20220614 - It is now possible to override the memory allocator, register the default using RegisterDefaultAllocator(). This means that the public API now takes STL containers that use a custom memory allocator so use Array instead of vector, UnorderedMap instead of unordered_map etc. If you're using placement new, replace new (x) y with ::new (x) y. Define JPH_DISABLE_CUSTOM_ALLOCATOR to disable this new behavior (b68097f582148d6f66c18a6ff95c5ca9b40b48cc)
* 20220606 - *SBS* - The slider constraint now has frequency and damping for its limits (09d6d9d51c46fbd159bf98abfd43cc639f6c0403)
* 20220606 - *SBS* - The rack and pinion and gear constraints were added (09d6d9d51c46fbd159bf98abfd43cc639f6c0403)
* 20220517 - Note: Superseded by d16a0b05bfeed42b1618e3774a9c953e6922d22b. When constructing a FixedConstraint you now need to call FixedConstraintSettings::SetPoint to configure the point where the bodies attach (4f7c925c31f39eda1d8d68e4e72456b5def93d9b)
* 20220516 - Constraint::GetType was renamed to GetSubType, a new GetType function was introduced (3e2151a009e8f11ca724754b2bd25e14d2654fb6)
* 20220516 - *SBS* - Added possibility to save the current state of the physics world as a scene (3e2151a009e8f11ca724754b2bd25e14d2654fb6)
* 20220510 - Factory::sInstance must now be allocated by the application prior to calling RegisterTypes() and has changed to a pointer (3ca62973dae7cda7a9ceece698438a45b9ad1433)
* 20220503 - Unused function SerializableObject::OnLoaded was removed (388d47254a236c053a472e54c10b264765badc09)
* 20220502 - ContactConstraintManager::CombineFunction has additional parameters: the SubShapeIDs from both bodies (6b873563739dfd3d77263c2c50af2f3f418ec15b)
* 20220415 - Removed Body::GetDebugName / SetDebugName, keep this info in a lookaside table if you need it (6db4d3beac6760e55f65102db00f93dfbc56ac26)
* 20220406 - Renamed CollisionDispatch::sCastShapeVsShape to sCastShapeVsShapeLocalSpace (6ba21f50dcf17bd506080ec30759724a7f3097d8)
* 20220327 - Changed the default include path, #include <xxx> must be replaced by #include <Jolt/xxx> (06e9d17d385814cd24d3b77d689c0a29d854e194)
* 20220318 - Added support for SSE2. If you want to use later versions of SSE make sure you have JPH_USE_SSE4_1 and JPH_USE_SSE4_2 defined (28f363856a007d03f657e46e8f6d90ccd7c6487a)
* 20220303 - Note: Partially superseded by d16a0b05bfeed42b1618e3774a9c953e6922d22b. When constructing a SliderConstraint you now need to call SliderConstraintSettings::SetPoint to configure the point where the bodies attach. Also replace mSliderAxis = x with SetSliderAxis(x) (5a327ec182d0436d435c62d0bccb4e76c6324659)
* 20220228 - PointConstraint::mCommonPoint is now mPoint1 / mPoint2. Replace mCommonPoint = x with mPoint1 = mPoint2 = x (066dfb8940ba3e7dbf8ed47e9a1eeb194730e04b)
* 20220226 - ObjectToBroadPhaseLayer and BroadPhaseLayerToString changed to BroadPhaseLayerInterface, this makes mapping a broadphase layer to an object layer more flexible (36dd3f8c8c31ef1aeb7585b2b615c23bc8b76f13)
* 20220222 - Shape and body user data changed from void * / uint32 to uint64 (14e062ac96abd571c6eff5e40b1df4d8b2333f55)

## Changes between v1.0.0 and v1.1.0

* No breaking changes.

## Changes between v0.0.0 and v1.0.0

* 20220107 - PhysicsSettings::mBodyPairCacheCosMaxDeltaRotation was renamed to mBodyPairCacheCosMaxDeltaRotationDiv2
* 20211219 - *SBS* - Now storing 3 components for a Vec3 instead of 4 in SaveBinaryState (23c1b9d9029d74076c0549c8779b3b5ac2179ea3)
* 20211212 - Removed StatCollector (92a117e0f05a08de154e86d3cd0b354783aa5593)
* 20210711 - HeightFieldShapeSettings::mBlockSize is subdivided one more time at run-time, so this is effectively 2x the block size (2aa3b443bf71785616f3140c32e6a04c49516535)
* 20211106 - Mutex class now has its own implementation on Platform Blue, users must implement the JPH_PLATFORM_BLUE_MUTEX_* functions (a61dc67503a87ef0e190f7fb31d495ac51aa43de)
* 20211019 - ShapeCast::mShape no longer keeps a reference, the caller is responsible for keeping the reference now (e2bbdda9110b083b49ba323f8fd0d88c19847c2e)
* 20211004 - Removed RTTI from Shape class, use Shape::GetType / GetSubType now (6d5cafd53501c2c1e313f1b1f29d5161db074fd5)
* 20210930 - Changed RestoreMaterialState and RestoreSubShapeState to use pointers instead of vectors to allow loading shapes with fewer memory allocations (b8953791f35a91fcd12568c7dc4cc2f68f40fb3f)
* 20210918 - PhysicsSystem::Init takes an extra parameter to specify the amount of mutexes to use (ef371411af878023f062b9930db09f17411f01ba)
* 20210827 - BroadPhaseLayerPairFilter was changed to ObjectVsBroadPhaseLayerFilter to avoid testing too many layers during collision queries (33883574bbc6fe208a4b62054d00b582872da6f4)

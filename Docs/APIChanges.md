# Breaking API Changes

This document lists all breaking API changes by date and by release tag. Note that not all API changes are listed here, trivial changes (that cause a compile error and require an obvious fix) are not listed.

Changes that make some state saved through SaveBinaryState from a prior version of the library unreadable by the new version is marked as *SBS*. See 'Saving Shapes' in [Architecture and API documentation](https://jrouwe.github.io/JoltPhysics/) for further information.

## Changes between v3.0.1 and v4.0.0

* 20231003 - *SBS* - Bug fix in serialization of SoftBodySharedSettings breaks binary serialization format. (ccb250747eee4dedebfa02d950775478fb52f786)
* 20230914 - Removed GetProcessorTicksPerSecond as it was not correctly implemented for all platforms. (d44f4bad0872075d5cef2779742c89203d4f4488)
* 20230819 - *SBS* - RagdollSettings got the ability to have constraints that do not follow the skeleton. This changes the binary serialization format for this class. (08fc49d2d7abfa1a69e21971785d37724c748bb6)
* 20230807 - Renamed ContactSettings::mRelativeSurfaceVelocity to mRelativeLinearSurfaceVelocity. (76b809ddb1abf96641acc587fffa70101323d323)
* 20230807 - *SBS* - PhysicsScene is now able to load/save soft bodies. This changes the binary serialization format. (779ba3673beebdc4021842516f4ff6aa7c1e09b4)
* 20230805 - Body::SaveState and MotionProperties::SaveState now only save the state that can be changed by the simulation. Configuration properties like friction, restitution etc. must be saved by the user if desired. (7ff50429abd53f1914fd25a9e80ff47f22bc9f0e)
* 20230801 - *SBS* - Constraint priority was added to all constraints which changes the binary serialization format. (e341bb3e959460fbe196032095c1ab0346d7e746)
* 20230704 - *SBS* - A new flag was added to BodyCreationSettings that changes the binary serialization format. (2dd3a033a41e422eb470484029324cc9bbaf0825)
* 20230629 - Fix for engine RPM being much higher than wheel RPM when measured at clutch. Before we were ignoring bake and wheel torques in engine RPM calculation. Now they're much closer but this unfortunately means that the simulation of the vehicle has changed and mainly the engine torque and clutch strength need to be re-tweaked. (b40090766c545a68dccfac76cde8c6345ca626a6)
* 20230623 - The parameter inIntegrationSubSteps was removed from PhysicsSystem::Update because more and more features didn't support it. If you were using it multiply inCollisionSteps with the value of inIntegrationSubSteps to get roughly the same behavior. (8fcc7a78ec051b215bf13b037b9f975baa803b6f)
* 20230618 - *SBS* - A new flag was added to BodyCreationSettings that changes the binary serialization format. (107b70c7585909f0757a62c318261a18d670ff97)
* 20230610 - A bug was fixed that causes the vehicle suspension to be weaker when driving over low mass objects. This also changes suspension behavior a bit when driving over static objects. (44b82e395697ea553574df3cd806ffe264bfa5c4)
* 20230609 - *SBS* - The MotorcycleController lean controller is now a full PID controller. This changes binary serialization format. (70e7bb3e5808dabc17ee38fb823fbfa7e9140a91)
* 20230609 - *SBS* - VehicleConstraint uses the new SpringSettings class as a member which contains the mFrequency and mDamping members. This requires minor code changes. (0da97d8f3345f14c5b4b0ee3571c05832c556f98)
* 20230609 - *SBS* - DistanceConstraintSettings, SliderConstraintSettings and MotorSettings now use the new SpringSettings class as a member which contains the mFrequency and mDamping members. This requires minor code changes. (3cabc057c1267fde288c1ab2a23076702c71eb79)
* 20230520 - A bug was fixed in CharacterVirtual that makes mPenetrationRecoverySpeed behave according to the documentation (1 = fully resolve collision in 1 update). With the bug the recovery was too little. If you want the penetration recovery to work as before with the bug multiply it by 1 / delta_time. (8dd93317d66a9a72d3afeff4ecb17c257a7e9d91)
* 20230420 - To support compiling Jolt as a shared library, the RTTI macros were changed to be able to specify if a symbol should be exported or not. If you're using Jolt's RTTI system in your own project you need to change e.g. JPH_DECLARE_RTTI_VIRTUAL(XXX) to JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, XXX). (d2f1d97004d036c6f759203c42e264e401472037)

## Changes between v2.0.1 and v3.0.0

* 20230331 - *SBS* - Vehicle wheels now support specifying the steering axis and wheel forward and up axis separately. This breaks the serialization format and requires setting extra properties on the wheels. (4269d8bbc77b889552a842c2e8476ba7ffc6b9a1)
* 20230328 - Vehicle now supports suspension under an angle. The behavior of the suspension, even if it is under 90 degrees with the vehicle body, changed so this may require tweaking the spring constants. (172a99c718bded5faa169ac440517286684fa2f0)
* 20230316 - The signature of ShapeFilter changed and the ShouldCollide function is no longer called for triangles inside a mesh/heightfield shape (you can use CollisionCollector::AddHit to filter per triangle). The previous implementation didn't pass in enough context for the application to fully determine which sub shapes were colliding. See [#473](https://github.com/jrouwe/JoltPhysics/discussions/473) for more information. (bc4fa997f15f2953dc87ee5c1ba51ecf2077c287)
* 20230313 - VehicleCollisionTester::Collide parameter outSuspensionLength was returning suspension length + wheel radius, now it returns the suspension length. If you have your own implementation of VehicleCollisionTester you need to update your code. (fcd9cb0f1677709e30951f2748aefd5f72ffdae1)
* 20230212 - Sensors are now able to detect other Sensors, make sure you put sensors in an ObjectLayer that doesn't collide with other sensors if you want to preserve the old behavior. (a76f5891ee429ae4fcde659c19f1eb769f9d8a21)
* 20230205 - *SBS* - Added 'IsSensor' and 'UseManifoldReduction' to BodyCreationSettings::SaveBinaryState. (8f6f210f53fc71e43760e20aeb2eae28ea168f4b)
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

# Release Notes

For breaking API changes see [this document](https://github.com/jrouwe/JoltPhysics/blob/master/Docs/APIChanges.md).

## Unreleased Changes

### New functionality

* Added Cosserat rods to soft bodies. This is a stick constraint with an orientation that can be used to attach geometry. Can be used e.g. to simulate vegetation in a cheap way. See the new `SoftBodyCosseratRodConstraintTest` demo.
* Added ability to drive hinge constraints with `Ragdoll::DriveToPoseUsingMotors`. This also adds `HingeConstraint::SetTargetOrientationBS` which sets the target angle in body space.
* Added `JPH_USE_EXTERNAL_PROFILE` cmake option that allows overriding the behavior of the profile macros.
* Added `SoftBodyCreationSettings::mFacesDoubleSided` which treats the faces of the soft body as double sided. This can be used to make e.g. flags double sided.

### Bug Fixes

* WheelSettingsTV and WheelSettingsWV were not serializing their base class members.
* The remap tables in `SoftBodySharedSettings::OptimizationResults`` mapped from new to old index instead of from old to new as was documented. The maps now behave as documented.
* Fixed an issue where soft body bend constraints could be created with identical vertices. This led to an assert triggering.
* Fixed infinite recursion when colliding a `TriangleShape` vs a `TriangleShape`.
* 32-bit MinGW g++ doesn't call the correct overload for the new operator when a type is 16 bytes aligned. This could cause unaligned read access violations.
* Fixed compiling in double precision and fixed issues with floating point contraction that caused unit test failures on LoongArch architecture.
* Added an epsilon to the `CastRay` / `CastShape` early out condition to avoid dividing by a very small number and overflowing to INF. This can cause a float overflow exception.
* Fixed Samples requiring Vulkan extension `VK_EXT_device_address_binding_report` without checking if it is available.
* Fixed Vulkan warning in Samples: VkSemaphore is being signaled by VkQueue but it may still be in use by VkSwapchainKHR.

## v5.3.0

### New functionality

#### Samples

* The Samples and JoltViewer can run on Linux using Vulkan. Make sure to install the Vulkan SDK before compiling (see: `Build/ubuntu24_install_vulkan_sdk.sh`).
* The Samples and JoltViewer can run on macOS using Metal.

#### MeshShape

* `MeshShape`s of up to 110M triangles are possible now, but the actual maximum is very dependent on how the triangles in the mesh are connected.
* Optimized creation of `MeshShape`. Improves build speed by about 25% and reduces number of allocations by a factor of 1000. Allocations caused contention when building meshes from multiple threads.
* Added `MeshShapeSettings::mBuildQuality` which allows selecting between faster mesh creation or faster run time performance.

#### Character

* Added `OnContactPersisted`, `OnContactRemoved`, `OnCharacterContactPersisted` and `OnCharacterContactRemoved` functions on `CharacterContactListener` to better match the interface of `ContactListener`.
* Every `CharacterVirtual` now has a `CharacterID`. This ID can be used to identify the character after removal and is used to make the simulation deterministic in case a character collides with multiple other virtual characters.
* Added support for `CharacterVirtual` to override the inner rigid body ID. This can be used to make the simulation deterministic in e.g. client/server setups.

#### Collision Detection

* Added `PhysicsSystem::SetSimShapeFilter`. This allows filtering out collisions between sub shapes within a body and can for example be used to have a single body that contains a low detail simulation shape an a high detail collision query shape. An example of a body that's both a sensor and a rigid body can be found in `ContactListenerTest`.
* Added `PhysicsSystem::SetSimCollideBodyVsBody`. This allows overriding the collision detection between two bodies. It can be used to only store the 1st hit for sensor collisions. This makes sensors cheaper if you only need to know if there is an overlap or not. An example can be found in `SimCollideBodyVsBodyTest`.
* Added `ClosestHitPerBodyCollisionCollector` which will report the closest / deepest hit per body that the collision query collides with.
* Added overridable `CollisionCollector::OnBodyEnd` that is called after all hits for a body have been processed when collecting hits through `NarrowPhaseQuery`.

#### New Platforms

* Added support for RISC-V, LoongArch and PowerPC (Little Endian) CPUs.
* Added support for WASM64.

#### Various

* Removed the use of `std::unordered_map` and `std::unordered_set` and replaced them with our own implementation: `UnorderedMap` and `UnorderedSet`.
* Added `MotionProperties::ScaleToMass`. This lets you easily change the mass and inertia tensor of a body after creation.
* Split up `Body::ApplyBuoyancyImpulse` into `Body::GetSubmergedVolume` and `Body::ApplyBuoyancyImpulse`. This allows you to use the calculated submerged volume for other purposes.
* Added binary serialization to `SkeletalAnimation`.
* Added the ability to add a sub shape at a specified index in a `MutableCompoundShape` rather than at the end.
* Added `STLLocalAllocator` which is an allocator that can be used in e.g. the `Array` class. It keeps a fixed size buffer for N elements and only when it runs out of space falls back to the heap.
* Added the following constants on PhysicsSystem: `cMaxBodiesLimit`, `cMaxBodyPairsLimit` and `cMaxContactConstraintsLimit`. These constants are the max allowable values for `PhysicsSystem::Init`. Exceeding these will trigger an assert and the system will clamp the values. Note that on a 32 bit system, you'll run out of memory before you reach these values.

### Bug fixes

* Fixed bodies gaining more energy than intended due to restitution. E.g. A restitution of 1 could lead to bodies bouncing ever higher.
* `BodyInterface::AddForce` applied a force per soft body vertex rather than to the whole body, this resulted in a soft body accelerating much more compared to a rigid body of the same mass.
* `std::push_heap`/`pop_heap` behave differently on macOS vs Windows/Linux when elements compare equal, this made the cross platform deterministic build not deterministic in some cases.
* Removing a sub shape from a `MutableCompoundShape` would not update the bounding box if the last shape was removed, which can result in a small performance loss.
* An empty `MutableCompoundShape` now returns the same local bounding box as `EmptyShape` (a point at (0, 0, 0)). This prevents floating point overflow exceptions.
* VehicleConstraint would override `Body::SetAllowSleeping` every frame, making it impossible for client code to configure a vehicle that cannot go to sleep.
* Fixed `CharacterVirtual::Contact::mIsSensorB` not being persisted in `SaveState`.
* Fixed `CharacterVirtual::Contact::mHadContact` not being true for collisions with sensors. They will still be marked as `mWasDiscarded` to prevent any further interaction.
* Fixed `Character::SetShape` failing to switch when standing inside a sensor / `Character::PostSimulation` finding a sensor as ground collision.
* Fixed numerical inaccuracy in penetration depth calculation when `CollideShapeSettings::mMaxSeparationDistance` was set to a really high value (e.g. 1000).
* Bugfix in `Semaphore::Acquire` for non-windows platform. The count was updated before waiting, meaning that the counter would become -(number of waiting threads) and the semaphore would not wake up until at least the same amount of releases was done. In practice this meant that the main thread had to do the last (number of threads) jobs, slowing down the simulation a bit.
* Fixed a bug in `ManifoldBetweenTwoFaces` that led to incorrect `ContactManifold::mRelativeContactPointsOn2` when the contact normal and the face normal were not roughly parallel. Also it possibly led to jitter in the simulation in that case.
* Fixed `InternalEdgeRemovingCollector` not working when colliding with a very dense triangle grid because it ran out of internal space. Now falling back to memory allocations when this happens to avoid ghost collisions.
* Fixed running out of stack space when simulating a really high number of active rigid bodies.
* Moved the 'broad phase bit' to the highest bit in `BodyID` to avoid running out of `NodeID`s in `BroadPhaseQuadTree` when calling `PhysicsSystem::OptimizeBroadPhase` on a tree with a very high body count.
* `TempAllocatorImpl` uses 64 bit integers internally to allow for a higher max contact constraint count.
* When inserting lots of bodies without using batching, a broad phase tree of depth > 128 can be created. If the `PhysicsSystem` was destructed in this situation, a stack overflow would cause a crash.
* When calling `PhysicsSystem::Update` with a delta time of 0, contact remove callbacks were triggered by accident for all existing contacts.
* Fixed `HingeConstraint` not having limits if `LimitsMin` was set to `-JPH_PI` or `LimitsMax` was set to `JPH_PI`. It should only be turned off if both are.
* Fixed `CylinderShape::GetSupportingFace` returning the wrong face. When the height of a cylinder was small compared to its radius, it would sink more into the ground than needed during simulation.
* When there were no active bodies, the step listeners weren't called. This meant they couldn't wake up bodies. The step listeners are now only skipped if the physics system is updated with zero delta time.
* Fixed a race condition in soft body simulation that could break determinism.
* Added overloads for placement new in the `JPH_OVERRIDE_NEW_DELETE` macro, this means it is no longer needed to do `:: new (address) JPH::class_name(constructor_arguments)` but you can do `new (address) JPH::class_name(constructor_arguments)`.
* Fixed a GCC warning `-Wshadow=global`.

## v5.2.0

### New functionality

* Added PlaneShape. An infinite plane. Negative half space is considered solid.
* Added TaperedCylinderShape. A cylinder with different top and bottom radii.
* Added EmptyShape. A shape that collides with nothing and that can be used as a placeholder or for dummy bodies.
* Use MeshShapeSettings::mPerTriangleUserData at about 25% memory increase to get per triangle user data through MeshShape::GetTriangleUserData
* Added `Shape::GetLeafShape` to be able to get a leaf shape given a sub shape ID
* Added `HeightFieldShape::GetSubShapeCoordinates` to get the triangle coordinates of a particular sub shape ID
* Split back face mode between convex/triangles for ray casts. This allows you to e.g. have meshes that do give back face hits while convex shapes don't.
* SoftBodyManifold now returns sensor contacts separately. Before this change, there was a limit of a single colliding body per soft body vertex. If the closest body happened to be a sensor this effectively disabled the collision with the world and caused artifacts. We can now also detect multiple sensor contacts per soft body and they are returned through a new interface `SoftBodyManifold::GetSensorContactBodyID`.
* Added support for running Jolt with ThreadSanitizer.
* Added support for using ScaledShape inside CharacterVirtual.
* Added ability to save/restore a simulation in parts using `StateRecorder::SetIsLastPart`. Also added `StateRecorderFilter::ShouldRestoreContact` to allow selective restoring of contacts.
* Added `JPH_DEBUG_SYMBOL_FORMAT` cmake option. This allows switching from the default dwarf symbol format to e.g. the source-map format for emscripten, which speeds up compilation.

### Bug fixes

* Fixed an issue where enhanced internal edge removal would throw away valid contacts when a dynamic compound shape is colliding with another mesh / box shape.
* Fixed an issue where the bounding volume of a HeightFieldShape was not properly adjusted when calling SetHeights leading to missed collisions.
* Workaround for CMake error `CMake Error: No output files for WriteBuild!` when using the 'Ninja Multi-Config' generator.
* When a height field was created where SampleCount / BlockSize is not a power of 2 and a soft body touched the right or bottom border of the height field, the application would crash.
* Fixed a link error `ld: error: undefined symbol: pthread_create` on FreeBSD.
* Fixed missing files ConfigurationString.h and SoftBodyUpdateContext.h when running `cmake --install`.
* Fixed various missing header files when running `cmake --install` when `ENABLE_OBJECT_STREAM=OFF`.
* When using `cmake --install` to install a shared library on Windows, the dll is installed in the 'bin' folder now.
* Fixed cmake warning: `Policy CMP0177 is not set: install() DESTINATION paths are normalized.`
* Fixed `unresolved symbol '__emutls_v._ZN3JPH11PhysicsLock6sLocksE'` when compiling Jolt as a shared library with MinGW.
* Added workaround for issue where Firefox has problems with the `_mm_blendv_ps` intrinsic when compiling to WASM.

## v5.1.0

### New functionality

#### Soft Body

* Added support for applying a global force to a soft body through Body::AddForce.
* Implemented better algorithm to split soft body constraints into parallel groups. This makes the soft body simulation 10-20% faster and also enables multithreading LRA, bend, volume and skinned constraints.
* Added approximate ACos function which speeds up dihedral bend constraints by approx. 10%.
* Improved sorting of LRA soft body constraints to improve convergence.
* Added ability to draw soft body constraint evaluation order.

#### HeightField Shape

* Sped up deserialization of HeightFieldShape/MeshShape classes by optimizing reading a vector of data in StreamIn, by switching std::vector out for a custom Array class and by combining a number of allocations into one.
* Added HeightFieldShape::GetMinHeightValue/GetMaxHeightValue that can be used to know which range of heights are accepted by SetHeights.
* Allowing negative stride when getting/setting height field shape heights or materials. This improves performance if your data happens to be layed out the wrong way around.
* Added HeightFieldShapeSettings::mMaterialsCapacity which can enlarge the internal materials array capacity to avoid resizing when HeightFieldShape::SetMaterials is called with materials that weren't in use by the height field yet.
* Added Clone function to HeightFieldShape. This allows creating a copy before modifying the shape.

#### Character

* Added CharacterBaseSettings::mEnhancedInternalEdgeRemoval (default false) that allows smoother movement for both the Character and CharacterVirtual class.
* Added ability for a CharacterVirtual to collide with another CharacterVirtual by using the new CharacterVsCharacterCollision interface.
* Added the option to add an inner rigid body to a CharacterVirtual. This allows it to interact with sensors through the regular ContactListener and to be found by normal collision checks.

#### Vehicles

* Added ability to override the gravity vector per vehicle. This allows creating vehicles that can e.g. stick to the surface of a track and drive upside down. See VehicleConstraint::OverrideGravity.

#### Various

* Replaced std::vector with a custom Array class. std::vector initializes memory to zero which causes an undesired performance overhead.
* Added macro JPH_OBJECT_STREAM that controls if ObjectStream and serialized attributes are compiled into the library or not. This can reduce the size of the library if you're not using this feature.
* Added option to get a callback when a JobSystemThreadPool thread starts/stops. This allows you to e.g. set application specific thread locals.
* Added cmake option USE_ASSERTS to turn on asserts in builds other than the Debug build.
* Switch from using _DEBUG to NDEBUG to detect debug mode. NDEBUG is defined in the standard while _DEBUG is Visual Studio specific.
* The OVERRIDE_CXX_FLAGS cmake flag will now also work for MSVC and allow you to specify your own CMAKE_CXX_FLAGS_DEBUG/CMAKE_CXX_FLAGS_RELEASE flags
* BodyInterface::AddForce/Torque functions now take an optional EActivation parameter that makes it optional to activate the body. This can be used e.g. to not let the body wake up if you're applying custom gravity to a body.
* Activating bodies now resets the sleep timer when the body is already active. This prevents the body from going to sleep in the next frame and can avoid quick 1 frame naps.
* Added Clone function to MutableCompoundShape. This allows creating a copy before modifying the shape.
* QuadTree / FixedSizeFreeList: Reorder variable layout to reduce false sharing & thread syncs to reduce simulation time by approximately 5%.
* Generate a CMake config file when the project is installed. Allows for other projects to import Jolt using the find_package() functionality.
* Added USE_WASM_SIMD cmake option. This will enable SIMD on the emscripten WASM build.
* Emscripten WASM build can now be compiled cross platform deterministic and deliver the same results as Windows, Linux etc.
* Added Shape::MakeScaleValid function. This function will take a scale vector and check it against the scaling rules for the shape. If it is not valid, it will return a scale that is close to the provided scale which is valid.
* Added cmake options to toggle exception-handling and RTTI. CPP_EXCEPTIONS_ENABLED enables exceptions, CPP_RTTI_ENABLED enables RTTI. By default they're both off as Jolt doesn't use these features. In the PerformanceTest this speeds up the simulation by about 5% for MSVC, no difference was measured for clang.

### Bug fixes

* Fix for new warning in MSVC 17.10 in immintrin.h: '__check_isa_support': unreferenced inline function has been removed.
* Fix error in gcc 14. Using always_inline in debug mode causes error: "inlining failed in call to 'always_inline' 'XXX': function not considered for inlining"
* Fixed clang-18 warning "LLVMgold.so: error loading plugin ... cannot open shared object file: No such file or directory", due to https://github.com/llvm/llvm-project/issues/84271 it currently doesn't support LTO.
* Suppress GCC warning: 'XXX' may be used uninitialized in this function [-Werror=maybe-uninitialized].
* Fixed compile errors when compiling with GCC for the ARM platform.
* When calling CharacterVirtual::SetShape, a collision with a sensor would cause the function to abort as if the character was in collision.
* CharacterVirtual stick to floor / stair walk did not trigger a contact added callback on the CharacterContactListener.
* Fixed bug where the the skinned position of a soft body would update in the first sub-iteration, causing a large velocity spike and jittery behavior.
* Fixed bug where the velocity of soft body vertices would increase indefinitely when resting on the back stop of a skinned constraint.
* Fixed bug when SkinVertices for a soft body is not called every frame, the previous position of the skin was still used causing a replay of the motion of the previous frame.
* Fixed bug in cast ray vs soft body which caused missed collisions in case a back facing triangle was hit.
* Fixed handling of mass override from SoftBodyContactListener. Previously if the inverse mass of both of the soft body and the colliding body were set to 0, the soft body would still react.
* Fixed crash in Ragdoll::DriveToPoseUsingMotors when using constraints other than SwingTwistConstraint.
* Fixed -Wunused-parameter warning on GCC when building in Release mode with -Wextra.
* Fixed tolerance in assert in GetPenetrationDepthStepEPA.
* Due to a difference between the used instructions in NEON and SSE -Vec3::sZero() returned different binary results on ARM vs x86. When JPH_CROSS_PLATFORM_DETERMINISTIC is defined, we ensure that the calculation is the same now.
* Forgot to free a temporary allocation on an early out in HeightFieldShape::SetMaterials.
* Fix SSE not being enabled on x86 32-bits.
* Fixed a bug in the enhanced internal edge removal that could cause rigid bodies and characters to be affected by internal edges.
* Fixed a bug in MutableCompoundShape::AdjustCenterOfMass which would fail to update the bounding box of the shape.
* The 32 bit and 64 bit versions of the library now produce the same binary stream when serializing data to a StreamOut. Before some values would be stored as size_t which is platform dependent.

## v5.0.0

### New Functionality

#### Soft Body

* Added soft body skinning constraints. This can be used to limit the movement of soft body vertices based on a skinned mesh. See [documentation](https://jrouwe.github.io/JoltPhysics/index.html#skinning-soft-bodies) for more info or watch this [movie](https://www.youtube.com/watch?v=NXw8yMczHJg).
* Added ability to turn on/off skinning constraints and to update the max distance for all constraints with a distance multiplier.
* Added dihedral bend constraints for soft bodies. See [movie](https://www.youtube.com/watch?v=A1iswelnGH4).
* Added long range attachment constraints (also called tethers) for soft bodies.
* Added SoftBodyContactListener which allows you to get callbacks for collisions between soft bodies and rigid bodies. See [movie](https://www.youtube.com/watch?v=DmS_8d2bdOw).
* Added support for a vertex radius for soft bodies. This keeps the vertices a fixed distance away from the surface which can be used to avoid z-fighting while rendering the soft body.
* Added SoftBodySharedSettings::CreateConstraints function that can automatically generate constraints based on the faces of the soft body.
* Added ability to update a soft body outside of the physics simulation step using SoftBodyMotionProperties::CustomUpdate. This is e.g. useful if the soft body is teleported and needs to 'settle'.

#### Vehicles

* Added support for less than 1 collision test per simulation step for vehicle wheels. This behavior can be configured differently when the vehicle is active / inactive. This can be used for LODding vehicles.
* Added wheel index to VehicleConstraint::CombineFunction friction callback and calculating longitudinal and lateral friction in the same call so you can have more differentiation between wheels.
* Added ability to override the max tire impulse calculations for wheeled vehicles. See WheeledVehicleController::SetTireMaxImpulseCallback.
* Added ability to disable the lean steering limit for the motorcycle, turning this off makes the motorcycle more unstable, but gives you more control over the final steering angle.

#### Character

* CharacterVirtual will now receive an OnContactAdded callback when it collides with a sensor (but will have no further interaction).
* Added user data to CharacterVirtual.

#### Constraints

* Swing limits do not need to be symmetrical anymore for SixDOFConstraints. This requires using the new pyramid shaped swing limits (ESwingType::Pyramid). SwingTwistConstraints still requires symmetrical limits but can use the pyramid swing limits too. These are cheaper to evaluate but are less smooth.
* Twist limits no longer need to be centered around zero for SixDOFConstraints and SwingTwistConstraints, any value between -PI and PI is supported now.
* Changed the meaning of Constraint::mNumVelocity/PositionStepsOverride. Before the number of steps would be the maximum of all constraints and the default value, now an overridden value of 0 means that the constraint uses the default value, otherwise it will use the value as specified. This means that if all constraints in an island have a lower value than the default, we will now use the lower value instead of the default. This allows simulating an island at a lower precision than the default.
* Bodies can now also override the default number of solver iterations. This value is used when the body collides with another body and a contact constraint is created (for constraints, the constraint override is always used).
* Added fraction hint to PathConstraintPath::GetClosestPoint. This can be used to speed up the search along the curve and to disambiguate fractions in case a path reaches the same point multiple times (i.e. a figure-8).
* Added Constraint::ResetWarmStart and Ragdoll::ResetWarmStart. Used to notify the system that the configuration of the bodies and/or constraint has changed enough so that the warm start impulses should not be applied the next frame. You can use this function for example when repositioning a ragdoll through Ragdoll::SetPose in such a way that the orientation of the bodies completely changes so that the previous frame impulses are no longer a good approximation of what the impulses will be in the next frame.
* Multithreading the SetupVelocityConstraints job. This was causing a bottleneck in the case that there are a lot of constraints but very few possible collisions.

#### Collision Detection

* Created an object layer filter implementation that is similar to Bullet's group & mask filtering, see ObjectLayerPairFilterMask.
* Created implementations of BroadPhaseLayerInterface, ObjectVsBroadPhaseLayerFilter and ObjectLayerPairFilter that use a bit table internally. These make it easier to define ObjectLayers and with which object layers they collide.
* Renamed SensorDetectsStatic to CollideKinematicVsNonDynamic and made it work for non-sensors. This means that kinematic bodies can now get collision callbacks when they collide with other static / kinematic objects.
* Added function to query the bounding box of all bodies in the physics system, see PhysicsSystem::GetBounds.

#### Simulation

* Implemented enhanced internal edge removal algorithm. This should help reduce ghost collisions. See BodyCreationSettings::mEnhancedInternalEdgeRemoval and [movie](https://www.youtube.com/watch?v=Wh5MIiiPVDE).
* Added BodyInterface::SetUseManifoldReduction which will clear the contact cache and ensure that you get consistent contact callbacks in case the body that you're changing already has contacts.

#### Various

* Ability to enable gyroscopic forces on bodies to create the [Dzhanibekov effect](https://en.wikipedia.org/wiki/Tennis_racket_theorem).
* Supporting SIMD for WASM build. Use -msimd128 -msse4.2 options with emscripten to enable this.
* Allowing WASM build to use a custom memory allocator.
* Added DebugRendererSimple which can be used to simplify the creation of your own DebugRenderer implementation. It only requires a DrawLine, DrawTriangle and DrawText3D function to be implemented (which can be left empty).
* Added ability to update the height field materials after creation.

### Removed functionality
* Ability to restrict rotational degrees of freedom in local space, instead this is now done in world space.

### Bug fixes

* Fixed a bug in cast sphere vs triangle that could return a false positive hit against a degenerate triangle.
* Fixed bug in soft body vs tapered capsule. The calculations were slightly off causing a normal on the top or bottom sphere to be returned while the tapered part was actually closest.
* Fixed bug where colliding a cyclinder against a large triangle could return an incorrect contact point.
* Fixed bug where soft bodies would collide with sensors as if they were normal bodies.
* Sensors will no longer use speculative contacts, so will no longer report contacts before an actual contact is detected.
* Hinge limit constraint forces were clamped wrongly when the hinge was exactly at the minimum limit, making it harder to push the hinge towards the maximum limit.
* Fixed bug when a body with limited DOFs collides with static. If the resulting contact had an infinite effective mass, we would divide by zero and crash.
* Fixed unit tests failing when compiling for 32-bit Linux. The compiler defaults to using x87 instructions in this case which does not work well with the collision detection pipeline. Now defaulting to the SSE instructions.
* Fixed assert and improved interaction between a fast moving rigid body of quality LinearCast and a soft body.
* When creating a MeshShape with triangles that have near identical positions it was possible that the degenerate check decided that a triangle was not degenerate while the triangle in fact would be degenerate after vertex quantization. The simulation would crash when colliding with this triangle.
* A scaled compound shape with a center of mass of non zero would not apply the correct transform to its sub shapes when colliding with a soft body
* A soft body without any edges would hang the solver
* Fixed GCC 11.4 warning in JobSystemThreadPool.cpp: output may be truncated copying 15 bytes from a string of length 63
* Longitudinal friction impulse for wheeled/tracked vehicles could become much higher than the calculated max because each iteration it was clamped to the max friction impulse which meant the total friction impulse could be PhysicsSettings::mNumVelocitySteps times too high.
* Properly initializing current engine RPM to min RPM for wheeled/tracked vehicles. When min RPM was lower than the default min RPM the engine would not start at min RPM.
* Fixed a possible division by zero in Body::GetBodyCreationSettings when the inverse inertia diagonal had 0's.
* When specifying a -1 for min/max distance of a distance constraint and the calculated distance is incompatible with the other limit, we'll clamp it to that value now instead of ending up with min > max.
* Fixed bug that contact cache was partially uninitialized when colliding two objects with inv mass override of 0. When the contact listener would report a non zero inv mass override the next simulation step this would mean that the simulation would read garbage and potentially crash due to NaNs.

## v4.0.2

### New functionality
* Support for compiling with ninja on Windows.

### Bug fixes
* Fixed bug in Indexify function that caused it to be really slow when passing 10K identical vertices. Also fixed a problem that could have led to some vertices not being welded.
* Fixed bug in SixDOFConstraint::RestoreState that would cause motors to not properly turn on.
* Fixed a determinism issue in CharacterVirtual. The order of the contacts returned by GetActiveContacts() was not deterministic.
* Fixed issue in sample application that mouse is very sensitive when viewing with Parsec.

## v4.0.1

### New functionality
* Ability to stop overriding CMAKE_CXX_FLAGS_DEBUG/CMAKE_CXX_FLAGS_RELEASE which is important for Android as it uses a lot of extra flags. Set the OVERRIDE_CXX_FLAGS=NO cmake flag to enable this.
* Reduced size of a contact constraint which saves a bit of memory during simulation.
* Can now build a linux shared library using GCC.

### Bug fixes
* Fixed mass scaling (as provided by the ContactListener) not applied correctly to CCD objects & during solve position constraints. This led to kinematic objects being pushed by dynamic objects.
* Workaround for MSVC 17.8, limits.h doesn't include corecrt.h and triggers an error that \_\_STDC_WANT_SECURE_LIB\_\_ is not defined.
* Fixed bug in MustIncludeC logic in GetClosestPointOnTriangle.
* Removed the need for specifying -Wno-comment when compiling with GCC.

## v4.0.0

### New functionality
* Added support for soft bodies (feature still in development, see [announcement](https://x.com/jrouwe/status/1687051655898955776?s=20)).
* Support for limiting the degrees of freedom of a body to support 2D simulations (see [announcement](https://x.com/jrouwe/status/1676311800797622279?s=20)).
* Support for setting surface velocity of a body (see [announcement](https://x.com/jrouwe/status/1662727355553443844?s=20)).
* Added ability to update a height field after creation (see [announcement](https://x.com/jrouwe/status/1713670512801390829?s=20)).
* Support for non-power of 2 height fields.
* Expose a function to compare the JOLT_VERSION_ID with the version the library was compiled with to detect mismatches between library and client code.
* Added ability to specify extra ragdoll constraints that are not parent/child related.
* Ability to selectively save the state of a physics system to support replicating state over the network.
* Added constraint priority to control the order of evaluation of constraints (and thereby the most influential constraints).
* Sensors can now detect static objects.
* Ability to override mass and inertia of bodies from the ContactListener.
* Ability to specify stiffness/damping for springs instead of frequency/damping.
* Added option to disable the lean spring controller for motorcycles.
* Added vehicle callbacks at the beginning of the step listener and after wheel checks.
* Ability to override the position where the suspension and tire forces are applied.
* Support for building Jolt as a shared library on Windows.
* Optimized Indexify function from O(N^2) to O(N log(N)).

### Removed functionality
* Removed support for integration sub steps for PhysicsSystem::Update.

### New supported platforms
* 32-bit versions of Android on ARM and x86.

### Bug fixes
* Motor frequency/stiffness of 0 should turn the motor off.
* RotatedTranslatedShape::GetPosition returned the wrong value.
* If a body is removed between the broad phase detecting an overlap and the narrow phase locking the body, callbacks could be called on a body that has already been removed.
* Fixed flipped normals in EPA penetration depth algorithm which could cause the normal to point in the wrong direction for collision tests.
* Respecting the IsSensor flag for CCD bodies.
* Fixed double locking issue that could cause a deadlock while updating the AABB of a body during simulation.
* Fixed a crash when fetching a body using an invalid BodyID.
* Windows 32 vs 64-bit versions produce the same deterministic results now.
* Heightfield vs convex was not filled in in collision dispatch table. This caused sensors to assert and not detect collisions with heightfields.
* The friction applied during continuous collision detection could be sqrt(2) times too large.
* The friction was clamped independently on both tangential axis which meant that the total friction could be larger than the amount of friction that should have been allowed. It also meant that an object would slow down quicker on one axis than on another causing a curved trajectory.
* When an object wasn't moving fast enough to trigger restitution for a speculative contact, the contact was enforced at the current position rather than at the distance of the speculative contact.
* Fixed CharacterVirtual jittering issue when moving down on elevator.
* CharacterVirtual was speeding up beyond the requested speed when sliding along a wall.
* CharacterVirtual reported to be on ground for one more frame after jumping against a wall.
* Added missing delta time term in CharacterVirtual::DetermineConstraints.
* CastShape had incorrect early out condition which could cause it to miss the deepest penetration.
* Pitch/roll limit constraint for vehicles didn't work when local vehicle up did not match world up.
* Wheel contact point did not return deepest point in certain cases.
* Fix for engine RPM being much higher than wheel RPM when measured at clutch. Before we were ignoring bake and wheel torques in engine RPM calculation.
* Don't allow the vehicle to sleep when the transmission is switching.
* Fixed bug that caused suspension to be weaker when driving a vehicle over dynamic bodies.

## v3.0.0

* Support for double precision simulation for large worlds (see [announcement](https://twitter.com/jrouwe/status/1599366630273712128))
* Performance optimization that allows solving large islands on multiple threads (see [announcement](https://twitter.com/jrouwe/status/1633229953775828994))
* Vehicles now support suspensions that are at an angle with the vehicle body (instead of 90 degrees)
* Supporting cylinder based wheels for vehicles
* Experimental motor cycle physics (see [announcement](https://twitter.com/jrouwe/status/1642479907383959553))
* CharacterVirtual can now move relative to a moving object (e.g. a space ship)
* Added 2D physics example
* Added functionality to estimate the collision impulse in the contact added callback
* Added a JobSystemWithBarrier class that makes it easier to integrate with your own job system
* Support for 32-bit object layers to allow easier integration with existing collision filtering systems

## v2.0.1

* Adds ARM 32-bit support to support vcpkg-tool

## v2.0.0

### Major new functionality
* Simulation is now deterministic between Windows, Linux and macOS.
* Support for custom memory allocators.
* A new character class that lives outside the main simulation update and is mainly used for player movement (CharacterVirtual).
* Implemented skeleton mapper that can convert an animated skeleton to a ragdoll skeleton and back.
* Rack and pinion, gear and pulley constraints have been added.
* Ability for sensors to detect collisions with sleeping bodies.
* Improved engine model for wheeled vehicles.
* Most constraints can now also be configured in local space.

### New supported compilers
* MinGW
* GCC

### New supported platforms
* All intel platforms supporting SSE2 and higher (was SSE4.2)
* 32-bit applications (was 64 bit only)
* Windows on ARM
* Windows UWP
* macOS
* iOS
* WebAssembly

## v1.1.0

* Optimizations.

## v1.0.0

* Initial stable release.

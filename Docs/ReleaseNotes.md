# Release Notes

For breaking API changes see [this document](https://github.com/jrouwe/JoltPhysics/blob/master/Docs/APIChanges.md).

## Unreleased changes

### New functionality
* Created an object layer filter implementation that is similar to Bullet's group & mask filtering, see ObjectLayerPairFilterMask.
* Ability to enable gyroscopic forces on bodies to create the [Dzhanibekov effect](https://en.wikipedia.org/wiki/Tennis_racket_theorem).
* Swing limits do not need to be symmetrical anymore for SixDOFConstraints. This requires using the new pyramid shaped swing limits (ESwingType::Pyramid). SwingTwistConstraints still requires symmetrical limits but can use the pyramid swing limits too. These are cheaper to evaluate but are less smooth.
* Twist limits no longer need to be centered around zero for SixDOFConstraints and SwingTwistConstraints, any value between -PI and PI is supported now.
* Changed the meaning of Constraint::mNumVelocity/PositionStepsOverride. Before the number of steps would be the maximum of all constraints and the default value, now an overridden value of 0 means that the constraint uses the default value, otherwise it will use the value as specified. This means that if all constraints in an island have a lower value than the default, we will now use the lower value instead of the default. This allows simulating an island at a lower precision than the default.
* Bodies can now also override the default number of solver iterations. This value is used when the body collides with another body and a contact constraint is created (for constraints, the constraint override is always used).
* Added BodyInterface::SetUseManifoldReduction which will clear the contact cache and ensure that you get consistent contact callbacks in case the body that you're changing already has contacts.
* Created implementations of BroadPhaseLayerInterface, ObjectVsBroadPhaseLayerFilter and ObjectLayerPairFilter that use a bit table internally. These make it easier to define ObjectLayers and with which object layers they collide.
* Added support for less than 1 collision test per simulation step for vehicle wheels. This behavior can be configured differently when the vehicle is active / inactive. This can be used for LODding vehicles.
* Added wheel index to VehicleConstraint::CombineFunction friction callback and calculating longitudinal and lateral friction in the same call so you can have more differentiation between wheels.
* Added ability to disable the lean steering limit for the motorcycle, turning this off makes the motorcycle more unstable, but gives you more control over the final steering angle.
* Added function to query the bounding box of all bodies in the physics system, see PhysicsSystem::GetBounds.
* Renamed SensorDetectsStatic to CollideKinematicVsNonDynamic and made it work for non-sensors. This means that kinematic bodies can now get collision callbacks when they collide with other static / kinematic objects.
* CharacterVirtual will now receive an OnContactAdded callback when it collides with a sensor (but will have no further interaction).

### Improvements
* Multithreading the SetupVelocityConstraints job. This was causing a bottleneck in the case that there are a lot of constraints but very few possible collisions.

### Bug fixes
* Fixed bug in soft body vs tapered capsule. The calculations were slightly off causing a normal on the top or bottom sphere to be returned while the tapered part was actually closest.
* Sensors will no longer use speculative contacts, so will no longer report contacts before an actual contact is detected.
* Hinge limit constraint forces were clamped wrongly when the hinge was exactly at the minimum limit, making it harder to push the hinge towards the maximum limit.
* Fixed bug when a body with limited DOFs collides with static. If the resulting contact had an infinite effective mass, we would divide by zero and crash.
* Fixed unit tests failing when compiling for 32-bit Linux. The compiler defaults to using x87 instructions in this case which does not work well with the collision detection pipeline. Now defaulting to the SSE instructions.
* Fixed assert and improved interaction between a fast moving rigid body of quality LinearCast and a soft body.

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

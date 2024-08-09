[![CLA assistant](https://cla-assistant.io/readme/badge/jrouwe/JoltPhysics)](https://cla-assistant.io/jrouwe/JoltPhysics)
[![Build Status](https://github.com/jrouwe/JoltPhysics/actions/workflows/build.yml/badge.svg)](https://github.com/jrouwe/JoltPhysics/actions/)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=alert_status)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=bugs)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=code_smells)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=coverage)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)

# Jolt Physics

A multi core friendly rigid body physics and collision detection library. Suitable for games and VR applications. Used by Horizon Forbidden West.

[![Horizon Forbidden West Cover Art](https://jrouwe.nl/jolt/Horizon_Forbidden_West.png)](https://www.playstation.com/en-us/games/horizon-forbidden-west/)

|[![Ragdoll Pile](https://img.youtube.com/vi/pwyCW0yNKMA/hqdefault.jpg)](https://www.youtube.com/watch?v=pwyCW0yNKMA)|
|:-|
|*A YouTube video showing a ragdoll pile simulated with Jolt Physics.*|

For more demos and [videos](https://www.youtube.com/watch?v=pwyCW0yNKMA&list=PLYXVwtOr1CBxbA50jVg2dKUQvHW_5OOom) go to the [Samples](Docs/Samples.md) section.

## Design considerations

Why create yet another physics engine? Firstly, it has been a personal learning project. Secondly, I wanted to address some issues that I had with existing physics engines:

* Games do more than simulating physics. These things happen across multiple threads. We emphasize on concurrently accessing physics data outside of the main simulation update:
	* Sections of the simulation can be loaded / unloaded in the background. We prepare a batch of physics bodies on a background thread without locking or affecting the simulation. We insert the batch into the simulation with a minimal impact on performance.
	* Collision queries can run parallel to adding / removing or updating a body. If a change to a body happened on the same thread, the change will be immediately visible. If the change happened on another thread, the query will see a consistent before or after state. An alternative would be to have a read and write version of the world. This prevents changes from being visible immediately, so we avoid this.
	* Collision queries can run parallel to the main physics simulation. We do a coarse check (broad phase query) before the simulation step and do fine checks (narrow phase query) in the background. This way, long running processes (like navigation mesh generation) can be spread out across multiple frames.
* Accidental wake up of bodies cause performance problems when loading / unloading content. Therefore, bodies will not automatically wake up when created. Neighboring bodies will not be woken up when bodies are removed. This can be triggered manually if desired.
* The simulation runs deterministically. You can replicate a simulation to a remote client by merely replicating the inputs to the simulation. Read the [Deterministic Simulation](https://jrouwe.github.io/JoltPhysics/#deterministic-simulation) section to understand the limits.
* We try to simulate behavior of rigid bodies in the real world but make approximations. Therefore, this library should mainly be used for games or VR simulations.

## Features

* Simulation of rigid bodies of various shapes using continuous collision detection:
	* Sphere
	* Box
	* Capsule
	* Tapered-capsule
	* Cylinder
	* Convex hull
	* Compound
	* Mesh (triangle)
	* Terrain (height field)
* Simulation of constraints between bodies:
	* Fixed
	* Point
	* Distance (including springs)
	* Hinge
	* Slider (also called prismatic)
	* Cone
	* Rack and pinion
	* Gear
	* Pulley
	* Smooth spline paths
	* Swing-twist (for humanoid shoulders)
	* 6 DOF
* Motors to drive the constraints.
* Collision detection:
	* Casting rays.
	* Testing shapes vs shapes.
	* Casting a shape vs another shape.
	* Broadphase only tests to quickly determine which objects may intersect.
* Sensors (trigger volumes).
* Animated ragdolls:
	* Hard keying (kinematic only rigid bodies).
	* Soft keying (setting velocities on dynamic rigid bodies).
	* Driving constraint motors to an animated pose.
	* Mapping a high detail (animation) skeleton onto a low detail (ragdoll) skeleton and vice versa.
* Game character simulation (capsule)
	* Rigid body character. Moves during the physics simulation. Cheapest option and most accurate collision response between character and dynamic bodies.
	* Virtual character. Does not have a rigid body in the simulation but simulates one using collision checks. Updated outside of the physics update for more control. Less accurate interaction with dynamic bodies.
* Vehicles
	* Wheeled vehicles.
	* Tracked vehicles.
	* Motorcycles.
* Soft body simulation (e.g. a soft ball or piece of cloth).
	* Edge constraints.
	* Dihedral bend constraints.
	* Tetrahedron volume constraints.
	* Long range attachment constraints (also called tethers).
	* Limiting the simulation to stay within a certain range of a skinned vertex.
	* Internal pressure.
	* Collision with simulated rigid bodies.
	* Collision tests against soft bodies.
* Water buoyancy calculations.
* An optional double precision mode that allows large simulations.

## Supported platforms

* Windows (Desktop or UWP) x86/x64/ARM32/ARM64
* Linux (tested on Ubuntu) x64/ARM64
* FreeBSD
* Android x86/x64/ARM32/ARM64
* Platform Blue (a popular game console) x64
* macOS x64/ARM64
* iOS x64/ARM64
* WebAssembly, see [this](https://github.com/jrouwe/JoltPhysics.js) separate project.

## Required CPU features

* On x86/x64 the minimal requirements are SSE2. The library can be compiled using SSE4.1, SSE4.2, AVX, AVX2, or AVX512.
* On ARM64 the library uses NEON and FP16. On ARM32 it can be compiled without any special CPU instructions.

## Documentation

To learn more about Jolt go to the latest [Architecture and API documentation](https://jrouwe.github.io/JoltPhysics/). Documentation for [a specific release is also available](https://jrouwe.github.io/JoltPhysicsDocs/).

To get started, look at the [HelloWorld](HelloWorld/HelloWorld.cpp) example. A [HelloWorld example using CMake FetchContent](https://github.com/jrouwe/JoltPhysicsHelloWorld) is also available to show how you can integrate Jolt Physics in a CMake project.

Some algorithms used by Jolt are described in detail in my GDC 2022 talk: Architecting Jolt Physics for 'Horizon Forbidden West' ([slides](https://gdcvault.com/play/1027560/Architecting-Jolt-Physics-for-Horizon), [slides with speaker notes](https://jrouwe.nl/architectingjolt/ArchitectingJoltPhysics_Rouwe_Jorrit_Notes.pdf), [video](https://gdcvault.com/play/1027891/Architecting-Jolt-Physics-for-Horizon)).

## Compiling

* Compiles with Visual Studio 2019+, Clang 10+ or GCC 9+.
* Uses C++ 17.
* Depends only on the standard template library.
* Doesn't use RTTI.
* Doesn't use exceptions.

If you want to run on Platform Blue you'll need to provide your own build environment and PlatformBlue.h due to NDA requirements. This file is available on the Platform Blue developer forum.

For build instructions go to the [Build](Build/README.md) section. When upgrading from an older version of the library go to the [Release Notes](Docs/ReleaseNotes.md) or [API Changes](Docs/APIChanges.md) sections.

## Performance

If you're interested in how Jolt scales with multiple CPUs and compares to other physics engines, take a look at [this document](https://jrouwe.nl/jolt/JoltPhysicsMulticoreScaling.pdf).

## Folder structure

* Assets - This folder contains assets used by the TestFramework, Samples and JoltViewer.
* Build - Contains everything needed to build the library, see the [Build](Build/README.md) section.
* Docs - Contains documentation for the library.
* HelloWorld - A simple application demonstrating how to use the Jolt Physics library.
* Jolt - All source code for the library is in this folder.
* JoltViewer - It is possible to record the output of the physics engine using the DebugRendererRecorder class (a .jor file), this folder contains the source code to an application that can visualize a recording. This is useful for e.g. visualizing the output of the PerformanceTest from different platforms. Currently available on Windows only.
* PerformanceTest - Contains a simple application that runs a [performance test](Docs/PerformanceTest.md) and collects timing information.
* Samples - This contains the sample application, see the [Samples](Docs/Samples.md) section. Currently available on Windows only.
* TestFramework - A rendering framework to visualize the results of the physics engine. Used by Samples and JoltViewer. Currently available on Windows only.
* UnitTests - A set of unit tests to validate the behavior of the physics engine.
* WebIncludes - A number of JavaScript resources used by the internal profiling framework of the physics engine.

## Bindings for other languages

* C [here](https://github.com/michal-z/zig-gamedev/tree/main/libs/zphysics/libs) and [here](https://github.com/amerkoleci/JoltPhysicsSharp/tree/main/src/joltc)
* [C#](https://github.com/amerkoleci/JoltPhysicsSharp)
* [JavaScript](https://github.com/jrouwe/JoltPhysics.js)
* [Zig](https://github.com/michal-z/zig-gamedev/tree/main/libs/zphysics)

## Integrations in other engines

* [Godot](https://github.com/godot-jolt/godot-jolt)
* [Source Engine](https://github.com/Joshua-Ashton/VPhysics-Jolt)

See [a list of projects that use Jolt Physics here](Docs/ProjectsUsingJolt.md).

## License

The project is distributed under the [MIT license](LICENSE).

## Contributions

All contributions are welcome! If you intend to make larger changes, please discuss first in the GitHub Discussion section. For non-trivial changes, we require that you agree to a [Contributor Agreement](ContributorAgreement.md). When you create a PR, [CLA assistant](https://cla-assistant.io/) will prompt you to sign it.

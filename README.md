[![CLA assistant](https://cla-assistant.io/readme/badge/jrouwe/JoltPhysics)](https://cla-assistant.io/jrouwe/JoltPhysics)
[![Build Status](https://github.com/jrouwe/JoltPhysics/actions/workflows/build.yml/badge.svg)](https://github.com/jrouwe/JoltPhysics/actions/)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=alert_status)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=bugs)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=jrouwe_JoltPhysics&metric=code_smells)](https://sonarcloud.io/dashboard?id=jrouwe_JoltPhysics)

# Jolt Physics

A multi core friendly rigid body physics and collision detection library suitable for games and VR applications, used by Horizon Forbidden West.

[![Horizon Forbidden West Cover Art](https://jrouwe.nl/jolt/Horizon_Forbidden_West.png)](https://www.playstation.com/en-us/games/horizon-forbidden-west/)

|[![Ragdoll Pile](https://img.youtube.com/vi/pwyCW0yNKMA/hqdefault.jpg)](https://www.youtube.com/watch?v=pwyCW0yNKMA)|
|:-|
|*A YouTube video showing a ragdoll pile simulated with Jolt Physics.*|

For more demos and [videos](https://www.youtube.com/watch?v=pwyCW0yNKMA&list=PLYXVwtOr1CBxbA50jVg2dKUQvHW_5OOom) go to the [Samples](Docs/Samples.md) section.

To get started, look at the [HelloWorld](HelloWorld/HelloWorld.cpp) example.

If you're interested in how Jolt scales with multiple CPUs and compares to other physics engines, take a look at [this document](https://jrouwe.nl/jolt/JoltPhysicsMulticoreScaling.pdf).

The slides for my GDC 2022 talk [Architecting Jolt Physics for 'Horizon Forbidden West'](https://gdcvault.com/play/1027560/Architecting-Jolt-Physics-for-Horizon) are now available ([video here](https://gdcvault.com/play/1027891/Architecting-Jolt-Physics-for-Horizon))!

## Design Considerations

So why create yet another physics engine? First of all, this has been a personal learning project and secondly I wanted to address some issues that I had with existing physics engines:

* In games we usually need to do many more things than to simulate the physics world and we need to do this across multiple threads. We therefore place a lot of emphasis on concurrently accessing the physics simulation data outside of the main physics simulation update:
	* Sections of the world can be loaded / unloaded in the background. A batch of physics bodies can be prepared on a background thread without locking or affecting the physics simulation and then inserted into the world all at once with a minimal impact on performance.
	* Collision queries can run in parallel with other operations like insertion / removal of bodies. The query code is guaranteed to see a body in a consistent state, but when a body is changed during a collision query there is no guarantee if the change is visible to the query or not. If a thread modifies the position of a body and then does a collision query, it will immediately see the updated state (this is often a problem when working with a read version and a write version of the world).
	* It is also possible to run collision queries in parallel to the main physics simulation by doing the broad phase query before the simulation step. This way, long running processes (like navigation mesh generation) can be spread out across multiple frames while still running the physics simulation every frame.
* One of the main sources of performance problems we found was waking up too many bodies while loading / unloading content. Therefore, bodies will not automatically wake up when created and neighboring bodies will not be woken up when bodies are removed. This can be triggered manually if desired.
* The simulation runs deterministically, so you could replicate a simulation to a remote client by merely replicating the inputs to the simulation. Read the [Deterministic Simulation](https://jrouwe.github.io/JoltPhysics/) section to understand the limits of this.
* The simulation of this physics engine tries to simulate behavior of rigid bodies in the real world but makes approximations in the simulation so should mainly be used for games or VR simulations.

For more information see the [Architecture and API documentation](https://jrouwe.github.io/JoltPhysics/) section.

## Features

* Simulation of rigid bodies of various shapes using continous collision detection:
	* Sphere.
	* Box.
	* Capsule.
	* Tapered-capsule.
	* Cylinder.
	* Convex hull.
	* Compound.
	* Mesh (triangle).
	* Terrain (height field).
* Simulation of constraints between bodies:
	* Fixed.
	* Point.
	* Distance (including springs).
	* Hinge.
	* Slider (also called prismatic).
	* Cone.
	* Smooth spline paths.
	* Swing-twist (for humanoid shoulders).
	* 6 DOF.
* Motors to drive the constraints.
* Collision detection:
	* Casting rays.
	* Testing shapes vs shapes.
	* Casting a shape vs another shape.
	* Broadphase only tests for quickly determining which objects may intersect.
* Sensors (trigger volumes).
* Animated ragdolls:
	* Hard keying (kinematic only rigid bodies).
	* Soft keying (setting velocities on dynamic rigid bodies).
	* Driving constraint motors to an animated pose.
* Game character simulation (capsule)
	* Rigid body character. Moves during the physics simulation. Cheapest option and most accurate collision response between character and dynamic bodies.
	* Virtual character. Does not have a rigid body in the world but simulates one using collision checks. Updated outside of the physics update for more control. Less accurate interaction with dynamic bodies.
* Vehicle simulation of wheeled and tracked vehicles.
* Water buoyancy calculations.

## Supported Platforms

* Windows (VS2019, VS2022) x64 (Desktop/UWP)
* Linux (tested on Ubuntu 20.04) x64/ARM64
* Android (tested on Android 10) x64/ARM64
* Platform Blue (a popular game console) x64
* MacOS (tested on Monterey) x64/ARM64
* iOS (tested on iOS 15) x64/ARM64

## Required CPU features

* On x86 the minimal requirements are SSE2 but the library can be compiled using SSE4.1, SSE4.2, AVX or AVX2.
* On ARM64 the library requires NEON with FP16 support.

## Compiling

* The library has been tested to compile with Cl (Visual Studio 2019-2022), Clang 10+ and GCC 9+.
* It uses C++17 and only depends on the standard template library.
* It doesn't make use of compiler generated RTTI or exceptions.
* If you want to run on Platform Blue you'll need to provide your own build environment and PlatformBlue.h file due to NDA requirements (see Core.h for further info).

For build instructions go to the [Build](Build/README.md) section.

## Folder Structure

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

## License

The project is distributed under the [MIT license](LICENSE).

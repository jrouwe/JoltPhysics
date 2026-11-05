# AGENTS.md

This file provides guidance to LLMs.

Jolt Physics is a multi-core friendly rigid body / soft body physics and collision detection library written in C++17. The library itself lives in `Jolt/`; everything else (samples, viewer, tests) consumes it.

## Building

All CMake entry points live in `Build/` (the root `CMakeLists.txt` is `Build/CMakeLists.txt`). Run the helper scripts from inside `Build/`; they generate into a sibling folder such as `Build/VS2026_CL` or `Build/Linux_Debug`. See `Build/README.md` for all platforms.

Windows (MSVC, primary dev environment):
```
cd Build
cmake_vs2026_cl.bat                  # extra -D options are forwarded, e.g. -DDOUBLE_PRECISION=ON
msbuild VS2026_CL\JoltPhysics.slnx /property:Configuration=Debug -m
VS2026_CL\Debug\UnitTests.exe
```
Variants: `cmake_vs2026_clang.bat`, `cmake_vs2026_cl_double.bat`, `cmake_vs2026_cl_cross_platform_deterministic.bat`, `cmake_vs2026_cl_no_object_stream.bat`, `_32bit`, `_arm`.

Linux:
```
cd Build
./cmake_linux_clang_gcc.sh [Debug|Release|Distribution|ReleaseASAN|ReleaseUBSAN|ReleaseTSAN|ReleaseCoverage] [clang++|g++] [--double] [--deterministic] [--shared] [-D...]
cmake --build Linux_Debug -j$(nproc)
cd Linux_Debug && ./UnitTests        # or: ctest --output-on-failure
```
`./coverage_report_linux.sh [LLVM_VERSION]` builds `ReleaseCoverage`, runs the unit tests and produces an llvm-cov HTML report.

Build configs: `Debug` (asserts on), `Release` (profiling + debug renderer, no asserts), `Distribution` (shippable, no debug support), plus sanitizer/coverage variants (clang only).

Useful CMake options (`Build/CMakeLists.txt`): `TARGET_UNIT_TESTS`, `TARGET_SAMPLES`, `TARGET_VIEWER`, `TARGET_HELLO_WORLD`, `TARGET_PERFORMANCE_TEST` (turn off Samples/Viewer to avoid needing the Vulkan SDK / dxc on Linux), `DOUBLE_PRECISION`, `CROSS_PLATFORM_DETERMINISTIC`, `USE_ASSERTS`, `ENABLE_OBJECT_STREAM`, `JPH_USE_DX12/VK/MTL/CPU_COMPUTE`, the `USE_<instruction set>` toggles. `ENABLE_ALL_WARNINGS` is on by default: warnings are errors (`/Wall /WX` on MSVC), so new code must compile warning-free on MSVC, clang and gcc.

## Tests

Unit tests use doctest (`UnitTests/doctest.h`), one `TEST_SUITE("XxxTests")` per file with `TEST_CASE`s inside. Standard doctest command-line filters work:
```
UnitTests -ts=SoftBodyTests          # one suite
UnitTests -tc=TestBendConstraint     # one test case (wildcards allowed)
```
Physics-level tests typically use `PhysicsTestContext` (`UnitTests/PhysicsTestContext.h`) to set up a `PhysicsSystem` and step it, with logging listeners (`UnitTests/Logging*Listener.h`) to check callbacks.

`PerformanceTest` doubles as a determinism check: CI runs e.g. `./PerformanceTest -q=LinearCast -t=max -s=Ragdoll -validate_hash=<hash>` on a `Distribution` + `CROSS_PLATFORM_DETERMINISTIC` build (`.github/workflows/determinism_check.yml`). Changes that alter simulation output will change these hashes, and the expected hashes in that workflow must be updated.

## Adding files

Source files are listed explicitly — there is no globbing. A new file must be added to the matching list:
- Library: `Jolt/Jolt.cmake` (`JOLT_PHYSICS_SRC_FILES`)
- Unit tests: `UnitTests/UnitTests.cmake`
- Samples: `Samples/Samples.cmake`, and register the test class in `Samples/SamplesApp.cpp` (`JPH_DECLARE_RTTI_FOR_FACTORY` + an entry in the relevant category table)
- Shaders (`Jolt/Shaders`, HLSL, cross-compiled with dxc / spirv-cross for Vulkan/Metal) are also listed in `Jolt/Jolt.cmake`.

## Repository layout

- `Jolt/` — the library. `Core/` (allocators, `Array`/`UnorderedMap` containers, job system, RTTI, profiler, `StreamIn/Out`), `Math/` (SIMD `Vec3/Vec4/Mat44/Quat`, `RVec3`/`RMat44` which become doubles under `JPH_DOUBLE_PRECISION`), `Geometry/` (GJK, EPA, convex support, ray/AABox tests), `AABBTree/` (mesh BVH building/encoding), `Physics/` (simulation), `Skeleton/`, `Renderer/` (`DebugRenderer` interface), `Compute/` + `Shaders/` (GPU compute abstraction over DX12/Vulkan/Metal/CPU, used by hair simulation), `ObjectStream/` (text/binary reflection-based serialization).
- `TestFramework/` — rendering/input/UI framework shared by `Samples` and `JoltViewer`.
- `Samples/` — interactive demo app; each demo is a `Test` subclass under `Samples/Tests/<Category>/`.
- `JoltViewer/` — plays back recordings made with `DebugRendererRecorder`.
- `HelloWorld/` — minimal usage example.
- `PerformanceTest/` — benchmark/determinism scenes.
- `Docs/Architecture.md` — the authoritative design document (bodies, shapes, constraints, collision filtering, the simulation step in detail, determinism, multithreading rules). Read the relevant section before changing core behavior.

## Architecture essentials

- **PhysicsSystem** (`Jolt/Physics/PhysicsSystem.h`) owns `BodyManager`, `BroadPhase`, `ConstraintManager`, `ContactConstraintManager`, `IslandBuilder`, etc. `PhysicsSystem::Update` splits the step into many jobs with explicit dependencies on a `JobSystem`; the job graph and per-stage behavior are documented in `Docs/Architecture.md` ("The Simulation Step in Detail") and `Docs/PhysicsSystemUpdate.svg`.
- **Body access** goes through `BodyInterface` (locking or non-locking) and `BodyLockRead/Write` using `BodyID`s; bodies are protected by a mutex array. Collision queries are designed to run concurrently with body add/remove and with the simulation — keep that invariant in mind when touching broad phase or body management.
- **Shapes** derive from `Shape`; each has a `ShapeSettings` counterpart (serializable, `Create()` returns `ShapeResult`) and a runtime shape. Pairwise collide/cast functions are registered in `CollisionDispatch` by shape sub-type (each shape's `sRegister()`, called via `RegisterTypes()`). Adding a shape type means registering collide/cast functions against existing types.
- **Constraints** derive from `TwoBodyConstraint`; they're built from reusable constraint parts in `Physics/Constraints/ConstraintPart/` (axis, angle, rotation, spring parts) solved in velocity and position steps.
- **Settings vs. runtime objects**: `*Settings` classes are serializable via `SaveBinaryState`/`RestoreBinaryState` and (if `JPH_OBJECT_STREAM`) via `ObjectStream` RTTI attributes declared with `JPH_IMPLEMENT_SERIALIZABLE_*`. When adding a member to a settings class, update both serialization paths.
- **Determinism**: the simulation must stay deterministic (same inputs + same order → same output) and, under `JPH_CROSS_PLATFORM_DETERMINISTIC`, identical across platforms/compilers. Avoid iteration over unordered containers, thread-order-dependent results, or FMA/math that differs per platform in simulation paths.
- **Memory**: use `JPH::Array`, `Ref`/`RefConst`, allocation through the overridable `Allocate`/`Free` hooks (`Core/Memory.h`, `JPH_OVERRIDE_NEW_DELETE`), and `TempAllocator` for per-step scratch memory. No exceptions or RTTI by default.

## Conventions

- Every header starts with the SPDX license block, `#pragma once`, and wraps code in `JPH_NAMESPACE_BEGIN` / `JPH_NAMESPACE_END`. Consumers must include `<Jolt/Jolt.h>` before any other Jolt header; library headers do not include it themselves.
- Tabs for indentation (see `.editorconfig`); clang-format is disabled — match surrounding style by hand.
- Line endings: the Windows working tree uses CRLF (`.gitattributes` has `* text=auto`). Write new files and edits with CRLF so files don't end up with mixed endings. Exceptions: `*.sh` and `gradlew` are always LF.
- Naming prefixes: `m` members, `s` static functions/members, `c` constants, `in`/`out`/`io` for parameter direction, `Arg` suffix types (`Vec3Arg`, `RVec3Arg`) for SIMD parameters. Classes and methods are PascalCase.
- Exported symbols use `JPH_EXPORT`; use `JPH_ASSERT`, `JPH_ENABLE_ASSERTS`, `JPH_IF_DEBUG`, `JPH_PROFILE_FUNCTION` / `JPH_PROFILE` for instrumentation.
- User-visible changes go into `Docs/ReleaseNotes.md` (under "Unreleased changes"); breaking API changes or changes that break saved binary state (*SBS*) also go into `Docs/APIChanges.md` with date and commit hash.

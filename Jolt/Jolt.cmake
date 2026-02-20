# Requires C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Root
set(JOLT_PHYSICS_ROOT ${PHYSICS_REPO_ROOT}/Jolt)

# Source files
set(JOLT_PHYSICS_SRC_FILES
	${JOLT_PHYSICS_ROOT}/AABBTree/AABBTreeBuilder.cpp
	${JOLT_PHYSICS_ROOT}/AABBTree/AABBTreeBuilder.h
	${JOLT_PHYSICS_ROOT}/AABBTree/AABBTreeToBuffer.h
	${JOLT_PHYSICS_ROOT}/AABBTree/NodeCodec/NodeCodecQuadTreeHalfFloat.h
	${JOLT_PHYSICS_ROOT}/AABBTree/TriangleCodec/TriangleCodecIndexed8BitPackSOA4Flags.h
	${JOLT_PHYSICS_ROOT}/ConfigurationString.h
	${JOLT_PHYSICS_ROOT}/Compute/ComputeBuffer.h
	${JOLT_PHYSICS_ROOT}/Compute/ComputeQueue.h
	${JOLT_PHYSICS_ROOT}/Compute/ComputeSystem.cpp
	${JOLT_PHYSICS_ROOT}/Compute/ComputeSystem.h
	${JOLT_PHYSICS_ROOT}/Compute/ComputeShader.h
	${JOLT_PHYSICS_ROOT}/Core/ARMNeon.h
	${JOLT_PHYSICS_ROOT}/Core/Array.h
	${JOLT_PHYSICS_ROOT}/Core/Atomics.h
	${JOLT_PHYSICS_ROOT}/Core/BinaryHeap.h
	${JOLT_PHYSICS_ROOT}/Core/ByteBuffer.h
	${JOLT_PHYSICS_ROOT}/Core/Color.cpp
	${JOLT_PHYSICS_ROOT}/Core/Color.h
	${JOLT_PHYSICS_ROOT}/Core/Core.h
	${JOLT_PHYSICS_ROOT}/Core/Factory.cpp
	${JOLT_PHYSICS_ROOT}/Core/Factory.h
	${JOLT_PHYSICS_ROOT}/Core/FixedSizeFreeList.h
	${JOLT_PHYSICS_ROOT}/Core/FixedSizeFreeList.inl
	${JOLT_PHYSICS_ROOT}/Core/FPControlWord.h
	${JOLT_PHYSICS_ROOT}/Core/FPException.h
	${JOLT_PHYSICS_ROOT}/Core/FPFlushDenormals.h
	${JOLT_PHYSICS_ROOT}/Core/HashCombine.h
	${JOLT_PHYSICS_ROOT}/Core/HashTable.h
	${JOLT_PHYSICS_ROOT}/Core/IncludeWindows.h
	${JOLT_PHYSICS_ROOT}/Core/InsertionSort.h
	${JOLT_PHYSICS_ROOT}/Core/IssueReporting.cpp
	${JOLT_PHYSICS_ROOT}/Core/IssueReporting.h
	${JOLT_PHYSICS_ROOT}/Core/JobSystem.h
	${JOLT_PHYSICS_ROOT}/Core/JobSystem.inl
	${JOLT_PHYSICS_ROOT}/Core/JobSystemSingleThreaded.cpp
	${JOLT_PHYSICS_ROOT}/Core/JobSystemSingleThreaded.h
	${JOLT_PHYSICS_ROOT}/Core/JobSystemThreadPool.cpp
	${JOLT_PHYSICS_ROOT}/Core/JobSystemThreadPool.h
	${JOLT_PHYSICS_ROOT}/Core/JobSystemWithBarrier.cpp
	${JOLT_PHYSICS_ROOT}/Core/JobSystemWithBarrier.h
	${JOLT_PHYSICS_ROOT}/Core/LinearCurve.cpp
	${JOLT_PHYSICS_ROOT}/Core/LinearCurve.h
	${JOLT_PHYSICS_ROOT}/Core/LockFreeHashMap.h
	${JOLT_PHYSICS_ROOT}/Core/LockFreeHashMap.inl
	${JOLT_PHYSICS_ROOT}/Core/LSANSuppressions.h
	${JOLT_PHYSICS_ROOT}/Core/Memory.cpp
	${JOLT_PHYSICS_ROOT}/Core/Memory.h
	${JOLT_PHYSICS_ROOT}/Core/Mutex.h
	${JOLT_PHYSICS_ROOT}/Core/MutexArray.h
	${JOLT_PHYSICS_ROOT}/Core/NonCopyable.h
	${JOLT_PHYSICS_ROOT}/Core/ObjectToIDMap.h
	${JOLT_PHYSICS_ROOT}/Core/Profiler.cpp
	${JOLT_PHYSICS_ROOT}/Core/Profiler.h
	${JOLT_PHYSICS_ROOT}/Core/Profiler.inl
	${JOLT_PHYSICS_ROOT}/Core/QuickSort.h
	${JOLT_PHYSICS_ROOT}/Core/Reference.h
	${JOLT_PHYSICS_ROOT}/Core/Result.h
	${JOLT_PHYSICS_ROOT}/Core/RTTI.cpp
	${JOLT_PHYSICS_ROOT}/Core/RTTI.h
	${JOLT_PHYSICS_ROOT}/Core/ScopeExit.h
	${JOLT_PHYSICS_ROOT}/Core/Semaphore.cpp
	${JOLT_PHYSICS_ROOT}/Core/Semaphore.h
	${JOLT_PHYSICS_ROOT}/Core/StaticArray.h
	${JOLT_PHYSICS_ROOT}/Core/STLAlignedAllocator.h
	${JOLT_PHYSICS_ROOT}/Core/STLAllocator.h
	${JOLT_PHYSICS_ROOT}/Core/STLLocalAllocator.h
	${JOLT_PHYSICS_ROOT}/Core/STLTempAllocator.h
	${JOLT_PHYSICS_ROOT}/Core/StreamIn.h
	${JOLT_PHYSICS_ROOT}/Core/StreamOut.h
	${JOLT_PHYSICS_ROOT}/Core/StreamUtils.h
	${JOLT_PHYSICS_ROOT}/Core/StreamWrapper.h
	${JOLT_PHYSICS_ROOT}/Core/StridedPtr.h
	${JOLT_PHYSICS_ROOT}/Core/StringTools.cpp
	${JOLT_PHYSICS_ROOT}/Core/StringTools.h
	${JOLT_PHYSICS_ROOT}/Core/TempAllocator.h
	${JOLT_PHYSICS_ROOT}/Core/TickCounter.cpp
	${JOLT_PHYSICS_ROOT}/Core/TickCounter.h
	${JOLT_PHYSICS_ROOT}/Core/UnorderedMap.h
	${JOLT_PHYSICS_ROOT}/Core/UnorderedMapFwd.h
	${JOLT_PHYSICS_ROOT}/Core/UnorderedSet.h
	${JOLT_PHYSICS_ROOT}/Core/UnorderedSetFwd.h
	${JOLT_PHYSICS_ROOT}/Geometry/AABox.h
	${JOLT_PHYSICS_ROOT}/Geometry/AABox4.h
	${JOLT_PHYSICS_ROOT}/Geometry/ClipPoly.h
	${JOLT_PHYSICS_ROOT}/Geometry/ClosestPoint.h
	${JOLT_PHYSICS_ROOT}/Geometry/ConvexHullBuilder.cpp
	${JOLT_PHYSICS_ROOT}/Geometry/ConvexHullBuilder.h
	${JOLT_PHYSICS_ROOT}/Geometry/ConvexHullBuilder2D.cpp
	${JOLT_PHYSICS_ROOT}/Geometry/ConvexHullBuilder2D.h
	${JOLT_PHYSICS_ROOT}/Geometry/ConvexSupport.h
	${JOLT_PHYSICS_ROOT}/Geometry/Ellipse.h
	${JOLT_PHYSICS_ROOT}/Geometry/EPAConvexHullBuilder.h
	${JOLT_PHYSICS_ROOT}/Geometry/EPAPenetrationDepth.h
	${JOLT_PHYSICS_ROOT}/Geometry/GJKClosestPoint.h
	${JOLT_PHYSICS_ROOT}/Geometry/IndexedTriangle.h
	${JOLT_PHYSICS_ROOT}/Geometry/Indexify.cpp
	${JOLT_PHYSICS_ROOT}/Geometry/Indexify.h
	${JOLT_PHYSICS_ROOT}/Geometry/MortonCode.h
	${JOLT_PHYSICS_ROOT}/Geometry/OrientedBox.cpp
	${JOLT_PHYSICS_ROOT}/Geometry/OrientedBox.h
	${JOLT_PHYSICS_ROOT}/Geometry/Plane.h
	${JOLT_PHYSICS_ROOT}/Geometry/RayAABox.h
	${JOLT_PHYSICS_ROOT}/Geometry/RayCapsule.h
	${JOLT_PHYSICS_ROOT}/Geometry/RayCylinder.h
	${JOLT_PHYSICS_ROOT}/Geometry/RaySphere.h
	${JOLT_PHYSICS_ROOT}/Geometry/RayTriangle.h
	${JOLT_PHYSICS_ROOT}/Geometry/Sphere.h
	${JOLT_PHYSICS_ROOT}/Geometry/Triangle.h
	${JOLT_PHYSICS_ROOT}/Jolt.cmake
	${JOLT_PHYSICS_ROOT}/Jolt.h
	${JOLT_PHYSICS_ROOT}/Math/BVec16.h
	${JOLT_PHYSICS_ROOT}/Math/BVec16.inl
	${JOLT_PHYSICS_ROOT}/Math/DMat44.h
	${JOLT_PHYSICS_ROOT}/Math/DMat44.inl
	${JOLT_PHYSICS_ROOT}/Math/Double3.h
	${JOLT_PHYSICS_ROOT}/Math/DVec3.h
	${JOLT_PHYSICS_ROOT}/Math/DVec3.inl
	${JOLT_PHYSICS_ROOT}/Math/DynMatrix.h
	${JOLT_PHYSICS_ROOT}/Math/EigenValueSymmetric.h
	${JOLT_PHYSICS_ROOT}/Math/FindRoot.h
	${JOLT_PHYSICS_ROOT}/Math/Float2.h
	${JOLT_PHYSICS_ROOT}/Math/Float3.h
	${JOLT_PHYSICS_ROOT}/Math/Float4.h
	${JOLT_PHYSICS_ROOT}/Math/GaussianElimination.h
	${JOLT_PHYSICS_ROOT}/Math/HalfFloat.h
	${JOLT_PHYSICS_ROOT}/Math/Mat44.h
	${JOLT_PHYSICS_ROOT}/Math/Mat44.inl
	${JOLT_PHYSICS_ROOT}/Math/Math.h
	${JOLT_PHYSICS_ROOT}/Math/MathTypes.h
	${JOLT_PHYSICS_ROOT}/Math/Matrix.h
	${JOLT_PHYSICS_ROOT}/Math/Quat.h
	${JOLT_PHYSICS_ROOT}/Math/Quat.inl
	${JOLT_PHYSICS_ROOT}/Math/Real.h
	${JOLT_PHYSICS_ROOT}/Math/Swizzle.h
	${JOLT_PHYSICS_ROOT}/Math/Trigonometry.h
	${JOLT_PHYSICS_ROOT}/Math/UVec4.h
	${JOLT_PHYSICS_ROOT}/Math/UVec4.inl
	${JOLT_PHYSICS_ROOT}/Math/Vec3.cpp
	${JOLT_PHYSICS_ROOT}/Math/Vec3.h
	${JOLT_PHYSICS_ROOT}/Math/Vec3.inl
	${JOLT_PHYSICS_ROOT}/Math/Vec4.h
	${JOLT_PHYSICS_ROOT}/Math/Vec4.inl
	${JOLT_PHYSICS_ROOT}/Math/Vector.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStream.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/SerializableAttribute.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/SerializableAttributeEnum.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/SerializableAttributeTyped.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/SerializableObject.cpp
	${JOLT_PHYSICS_ROOT}/ObjectStream/SerializableObject.h
	${JOLT_PHYSICS_ROOT}/ObjectStream/TypeDeclarations.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/AllowedDOFs.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/Body.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/Body.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/Body.inl
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyAccess.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyActivationListener.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyCreationSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyCreationSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyFilter.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyID.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyInterface.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyInterface.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyLock.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyLockInterface.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyLockMulti.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyManager.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyManager.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyPair.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/BodyType.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/MassProperties.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/MassProperties.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/MotionProperties.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Body/MotionProperties.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/MotionProperties.inl
	${JOLT_PHYSICS_ROOT}/Physics/Body/MotionQuality.h
	${JOLT_PHYSICS_ROOT}/Physics/Body/MotionType.h
	${JOLT_PHYSICS_ROOT}/Physics/Character/Character.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Character/Character.h
	${JOLT_PHYSICS_ROOT}/Physics/Character/CharacterBase.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Character/CharacterBase.h
	${JOLT_PHYSICS_ROOT}/Physics/Character/CharacterID.h
	${JOLT_PHYSICS_ROOT}/Physics/Character/CharacterVirtual.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Character/CharacterVirtual.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/AABoxCast.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ActiveEdgeMode.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ActiveEdges.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BackFaceMode.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhase.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhase.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseBruteForce.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseBruteForce.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseLayer.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceMask.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseQuadTree.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/BroadPhaseQuery.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterMask.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/QuadTree.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/BroadPhase/QuadTree.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CastConvexVsTriangles.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CastConvexVsTriangles.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CastResult.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CastSphereVsTriangles.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CastSphereVsTriangles.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollectFacesMode.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideConvexVsTriangles.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideConvexVsTriangles.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollidePointResult.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideShapeVsShapePerLeaf.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideSoftBodyVertexIterator.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideSoftBodyVerticesVsTriangles.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideSphereVsTriangles.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollideSphereVsTriangles.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionCollector.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionCollectorImpl.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionDispatch.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionDispatch.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionGroup.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/CollisionGroup.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ContactListener.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/EstimateCollisionResponse.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/EstimateCollisionResponse.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/GroupFilter.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/GroupFilter.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/GroupFilterTable.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/GroupFilterTable.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/InternalEdgeRemovingCollector.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ManifoldBetweenTwoFaces.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ManifoldBetweenTwoFaces.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/NarrowPhaseQuery.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/NarrowPhaseQuery.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/NarrowPhaseStats.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/NarrowPhaseStats.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ObjectLayer.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ObjectLayerPairFilterMask.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ObjectLayerPairFilterTable.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/PhysicsMaterial.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/PhysicsMaterial.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/PhysicsMaterialSimple.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/PhysicsMaterialSimple.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/RayCast.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/BoxShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/BoxShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CapsuleShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CapsuleShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CompoundShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CompoundShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CompoundShapeVisitors.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ConvexHullShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ConvexHullShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ConvexShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ConvexShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CylinderShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/CylinderShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/DecoratedShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/DecoratedShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/EmptyShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/EmptyShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/GetTrianglesContext.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/HeightFieldShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/HeightFieldShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/MeshShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/MeshShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/MutableCompoundShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/MutableCompoundShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/OffsetCenterOfMassShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/OffsetCenterOfMassShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/PlaneShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/PlaneShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/PolyhedronSubmergedVolumeCalculator.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/RotatedTranslatedShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/RotatedTranslatedShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ScaledShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ScaledShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/ScaleHelpers.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/Shape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/Shape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/SphereShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/SphereShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/StaticCompoundShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/StaticCompoundShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/SubShapeID.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/SubShapeIDPair.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TaperedCapsuleShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TaperedCapsuleShape.gliffy
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TaperedCapsuleShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TaperedCylinderShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TaperedCylinderShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TriangleShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/Shape/TriangleShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ShapeCast.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/ShapeFilter.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/SimShapeFilter.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/SimShapeFilterWrapper.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/SortReverseAndStore.h
	${JOLT_PHYSICS_ROOT}/Physics/Collision/TransformedShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Collision/TransformedShape.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/CalculateSolverSteps.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConeConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConeConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/Constraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/Constraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintManager.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintManager.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/AngleConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/AxisConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/DualAxisConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/GearConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/HingeRotationConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/IndependentAxisConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/PointConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/RackAndPinionConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/RotationEulerConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/RotationQuatConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/SpringPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ConstraintPart/SwingTwistConstraintPart.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ContactConstraintManager.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/ContactConstraintManager.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/DistanceConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/DistanceConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/FixedConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/FixedConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/GearConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/GearConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/HingeConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/HingeConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/MotorSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/MotorSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraintPath.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraintPath.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraintPathHermite.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PathConstraintPathHermite.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PointConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PointConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PulleyConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/PulleyConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/RackAndPinionConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/RackAndPinionConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SixDOFConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SixDOFConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SliderConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SliderConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SpringSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SpringSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SwingTwistConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/SwingTwistConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/TwoBodyConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Constraints/TwoBodyConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/DeterminismLog.cpp
	${JOLT_PHYSICS_ROOT}/Physics/DeterminismLog.h
	${JOLT_PHYSICS_ROOT}/Physics/EActivation.h
	${JOLT_PHYSICS_ROOT}/Physics/EPhysicsUpdateError.h
	${JOLT_PHYSICS_ROOT}/Physics/Hair/Hair.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Hair/Hair.h
	${JOLT_PHYSICS_ROOT}/Physics/Hair/HairSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Hair/HairSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/Hair/HairShaders.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Hair/HairShaders.h
	${JOLT_PHYSICS_ROOT}/Physics/IslandBuilder.cpp
	${JOLT_PHYSICS_ROOT}/Physics/IslandBuilder.h
	${JOLT_PHYSICS_ROOT}/Physics/LargeIslandSplitter.cpp
	${JOLT_PHYSICS_ROOT}/Physics/LargeIslandSplitter.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsLock.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsScene.cpp
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsScene.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsStepListener.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsSystem.cpp
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsSystem.h
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsUpdateContext.cpp
	${JOLT_PHYSICS_ROOT}/Physics/PhysicsUpdateContext.h
	${JOLT_PHYSICS_ROOT}/Physics/Ragdoll/Ragdoll.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Ragdoll/Ragdoll.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyContactListener.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyCreationSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyCreationSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyManifold.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyMotionProperties.cpp
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyMotionProperties.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyShape.cpp
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyShape.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodySharedSettings.cpp
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodySharedSettings.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyUpdateContext.h
	${JOLT_PHYSICS_ROOT}/Physics/SoftBody/SoftBodyVertex.h
	${JOLT_PHYSICS_ROOT}/Physics/StateRecorder.h
	${JOLT_PHYSICS_ROOT}/Physics/StateRecorderImpl.cpp
	${JOLT_PHYSICS_ROOT}/Physics/StateRecorderImpl.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/MotorcycleController.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/MotorcycleController.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/TrackedVehicleController.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/TrackedVehicleController.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleAntiRollBar.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleAntiRollBar.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleCollisionTester.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleCollisionTester.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleConstraint.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleConstraint.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleController.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleController.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleDifferential.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleDifferential.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleEngine.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleEngine.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleTrack.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleTrack.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleTransmission.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/VehicleTransmission.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/Wheel.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/Wheel.h
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/WheeledVehicleController.cpp
	${JOLT_PHYSICS_ROOT}/Physics/Vehicle/WheeledVehicleController.h
	${JOLT_PHYSICS_ROOT}/RegisterTypes.cpp
	${JOLT_PHYSICS_ROOT}/RegisterTypes.h
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRenderer.cpp
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRenderer.h
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererPlayback.cpp
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererPlayback.h
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererRecorder.cpp
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererRecorder.h
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererSimple.cpp
	${JOLT_PHYSICS_ROOT}/Renderer/DebugRendererSimple.h
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletalAnimation.cpp
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletalAnimation.h
	${JOLT_PHYSICS_ROOT}/Skeleton/Skeleton.cpp
	${JOLT_PHYSICS_ROOT}/Skeleton/Skeleton.h
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletonMapper.cpp
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletonMapper.h
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletonPose.cpp
	${JOLT_PHYSICS_ROOT}/Skeleton/SkeletonPose.h
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitter.cpp
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitter.h
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitterBinning.cpp
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitterBinning.h
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitterMean.cpp
	${JOLT_PHYSICS_ROOT}/TriangleSplitter/TriangleSplitterMean.h
)

if (ENABLE_OBJECT_STREAM)
	set(JOLT_PHYSICS_SRC_FILES
		${JOLT_PHYSICS_SRC_FILES}
		${JOLT_PHYSICS_ROOT}/ObjectStream/GetPrimitiveTypeOfType.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStream.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamBinaryIn.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamBinaryIn.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamBinaryOut.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamBinaryOut.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamIn.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamIn.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamOut.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamOut.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamTextIn.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamTextIn.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamTextOut.cpp
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamTextOut.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/ObjectStreamTypes.h
		${JOLT_PHYSICS_ROOT}/ObjectStream/TypeDeclarations.cpp
	)
endif()

if (JPH_USE_DX12 OR JPH_USE_VK OR JPH_USE_MTL OR JPH_USE_CPU_COMPUTE)
	# Compute shaders
	set(JOLT_PHYSICS_SHADERS
		${JOLT_PHYSICS_ROOT}/Shaders/HairApplyDeltaTransform.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairApplyGlobalPose.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairCalculateCollisionPlanes.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairCalculateRenderPositions.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridAccumulate.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridClear.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridNormalize.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairIntegrate.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairSkinRoots.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairSkinVertices.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairTeleport.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateRoots.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateStrands.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateVelocity.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateVelocityIntegrate.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/TestCompute.hlsl
		${JOLT_PHYSICS_ROOT}/Shaders/TestCompute2.hlsl
	)

	set(JOLT_PHYSICS_SHADER_HEADERS
		${JOLT_PHYSICS_ROOT}/Shaders/HairApplyDeltaTransformBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairApplyGlobalPose.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairApplyGlobalPoseBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairCalculateCollisionPlanesBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairCalculateRenderPositions.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairCalculateRenderPositionsBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairCommon.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridAccumulateBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridClearBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairGridNormalizeBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairIntegrate.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairIntegrateBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairSkinRootsBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairSkinVerticesBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairStructs.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairTeleportBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateRootsBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateStrandsBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateVelocity.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateVelocityBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairUpdateVelocityIntegrateBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderCore.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderMat44.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderMath.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderPlane.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderQuat.h
		${JOLT_PHYSICS_ROOT}/Shaders/ShaderVec3.h
		${JOLT_PHYSICS_ROOT}/Shaders/TestComputeBindings.h
		${JOLT_PHYSICS_ROOT}/Shaders/TestCompute2Bindings.h
	)
endif()

# CPU compute support
if (JPH_USE_CPU_COMPUTE)
	set(JOLT_PHYSICS_SRC_FILES
		${JOLT_PHYSICS_SRC_FILES}
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeQueueCPU.cpp
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeQueueCPU.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeBufferCPU.cpp
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeBufferCPU.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeSystemCPU.cpp
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeSystemCPU.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ComputeShaderCPU.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/HLSLToCPP.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/ShaderWrapper.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/WrapShaderBegin.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/WrapShaderBindings.h
		${JOLT_PHYSICS_ROOT}/Compute/CPU/WrapShaderEnd.h
		${JOLT_PHYSICS_ROOT}/Shaders/HairWrapper.cpp
		${JOLT_PHYSICS_ROOT}/Shaders/HairWrapper.h
		${JOLT_PHYSICS_ROOT}/Shaders/TestComputeWrapper.cpp
	)
endif()

if (WIN32)
	# Add natvis file
	set(JOLT_PHYSICS_SRC_FILES ${JOLT_PHYSICS_SRC_FILES} ${JOLT_PHYSICS_ROOT}/Jolt.natvis)

	# Set properties to compile shaders as compute shaders
	set_source_files_properties(${JOLT_PHYSICS_SHADERS} PROPERTIES VS_SHADER_FLAGS "/WX /T cs_5_0")

	# DirectX support
	if (JPH_USE_DX12)
		# DirectX source files
		set(JOLT_PHYSICS_SRC_FILES
			${JOLT_PHYSICS_SRC_FILES}
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeQueueDX12.cpp
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeQueueDX12.h
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeBufferDX12.cpp
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeBufferDX12.h
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeSystemDX12.cpp
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeSystemDX12.h
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeSystemDX12Impl.cpp
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeSystemDX12Impl.h
			${JOLT_PHYSICS_ROOT}/Compute/DX12/ComputeShaderDX12.h
			${JOLT_PHYSICS_ROOT}/Compute/DX12/IncludeDX12.h
		)
	endif()
else()
	set(JPH_USE_DX12 OFF)
endif()

if (APPLE)
	# Metal support
	if (JPH_USE_MTL)
		# Metal source files
		set(JOLT_PHYSICS_SRC_FILES
			${JOLT_PHYSICS_SRC_FILES}
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeBufferMTL.mm
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeBufferMTL.h
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeQueueMTL.mm
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeQueueMTL.h
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeShaderMTL.mm
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeShaderMTL.h
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeSystemMTL.mm
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeSystemMTL.h
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeSystemMTLImpl.mm
			${JOLT_PHYSICS_ROOT}/Compute/MTL/ComputeSystemMTLImpl.h
		)

		find_program(DXC_COMPILER NAMES dxc)
		find_program(SPIRV_CROSS_COMPILER NAMES spirv-cross)
		if (NOT DXC_COMPILER)
			MESSAGE("Application 'dxc' not found. Can't compile compute shaders. Some functionality will be unavailable. You can install it by e.g. installing the Vulkan SDK.")
		elseif (NOT SPIRV_CROSS_COMPILER)
			MESSAGE("Application 'spirv-cross' not found. Can't compile compute shaders. Some functionality will be unavailable. You can install it by e.g. installing the Vulkan SDK.")
		else()
			# Determine target for shader compiler
			if (IOS)
				set(METAL_SDK_TARGET "iphonesimulator")
			else()
				set(METAL_SDK_TARGET "macosx")
			endif()

			# Compile Metal shaders
			foreach(SHADER ${JOLT_PHYSICS_SHADERS})
				cmake_path(GET SHADER STEM SHADER_STEM) # Filename without extension
				set(SPV_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_STEM}.spv")
				set(MTL_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_STEM}.metal")
				set(AIR_SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_STEM}.air")
				add_custom_command(OUTPUT ${AIR_SHADER}
					COMMAND ${DXC_COMPILER} -E main -T cs_6_0 -I Jolt/Shaders -WX -O3 -all_resources_bound ${SHADER} -spirv -fvk-use-dx-layout -fspv-entrypoint-name=${SHADER_STEM} -Fo ${SPV_SHADER}
					COMMAND ${SPIRV_CROSS_COMPILER} ${SPV_SHADER} --msl --output ${MTL_SHADER}
					COMMAND xcrun -sdk ${METAL_SDK_TARGET} metal -c ${MTL_SHADER} -o ${AIR_SHADER}
					DEPENDS ${SHADER} ${JOLT_PHYSICS_SHADER_HEADERS} # Currently don't have a way to detect header dependencies, so making dependent on all
					COMMENT "Compiling Metal ${SHADER}")
				list(APPEND JOLT_PHYSICS_MTL_SHADERS ${AIR_SHADER})
			endforeach()

			# Link Metal shaders
			set(JOLT_PHYSICS_METAL_LIB ${JOLT_PHYSICS_ROOT}/Shaders/Jolt.metallib)
			add_custom_command(OUTPUT ${JOLT_PHYSICS_METAL_LIB}
				COMMAND xcrun -sdk ${METAL_SDK_TARGET} metallib -o ${JOLT_PHYSICS_METAL_LIB} ${JOLT_PHYSICS_MTL_SHADERS}
				DEPENDS ${JOLT_PHYSICS_MTL_SHADERS}
				COMMENT "Linking shaders")

			# Group intermediate files
			source_group(Intermediate FILES ${JOLT_PHYSICS_MTL_SHADERS} ${JOLT_PHYSICS_METAL_LIB})
		endif()
	endif()

	# Ignore PCH files for .mm files
	foreach(SRC_FILE ${JOLT_PHYSICS_SRC_FILES})
		if (SRC_FILE MATCHES "\.mm")
			set_source_files_properties(${SRC_FILE} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
		endif()
	endforeach()
else()
	set(JPH_USE_MTL OFF)
endif()

# Vulkan support
if (JPH_USE_VK)
	find_package(Vulkan)
	if (Vulkan_FOUND)
		# Vulkan source files
		set(JOLT_PHYSICS_SRC_FILES
			${JOLT_PHYSICS_SRC_FILES}
			${JOLT_PHYSICS_ROOT}/Compute/VK/BufferVK.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeBufferVK.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeBufferVK.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeQueueVK.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeQueueVK.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeShaderVK.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeShaderVK.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVK.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVK.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVKImpl.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVKImpl.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVKWithAllocator.cpp
			${JOLT_PHYSICS_ROOT}/Compute/VK/ComputeSystemVKWithAllocator.h
			${JOLT_PHYSICS_ROOT}/Compute/VK/IncludeVK.h
		)

		# TODO: For some reason it errors on finding dxc when we specify the dxc component to find_vulkan (and update cmake version)
		# For now, just set it manually
		string(REPLACE "glslc" "dxc" Vulkan_dxc_EXECUTABLE ${Vulkan_GLSLC_EXECUTABLE})

		# Compile Vulkan shaders
		foreach(SHADER ${JOLT_PHYSICS_SHADERS})
			string(REPLACE ".hlsl" ".spv" SPV_SHADER ${SHADER})
			add_custom_command(OUTPUT ${SPV_SHADER}
				# We use dxc instead of: ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=compute ${SHADER} -o ${SPV_SHADER}
				# The glslc compiler has the following issues:
				# - All buffers bind to slot 0. We don't want to manually specify registers so this requires going into the SPIRV code and patching it.
				# - It automatically aligns float3 to 16 byte boundaries which wastes a lot of memory in structs. We only seem to be able to override this alignment when compiling a GLSL shader and not with HLSL.
				COMMAND ${Vulkan_dxc_EXECUTABLE} -E main -T cs_6_0 -I Jolt/Shaders -WX -O3 -all_resources_bound ${SHADER} -spirv -fvk-use-dx-layout -Fo ${SPV_SHADER}
				DEPENDS ${SHADER} ${JOLT_PHYSICS_SHADER_HEADERS} # Currently don't have a way to detect header dependencies, so making dependent on all
				COMMENT "Compiling Vulkan ${SHADER}")
			list(APPEND JOLT_PHYSICS_SPV_SHADERS ${SPV_SHADER})
		endforeach()

		# Group intermediate files
		source_group(Intermediate FILES ${JOLT_PHYSICS_SPV_SHADERS})
	else()
		set(JPH_USE_VK OFF)
	endif()
endif()

# Group source files
source_group(TREE ${JOLT_PHYSICS_ROOT} FILES ${JOLT_PHYSICS_SRC_FILES} ${JOLT_PHYSICS_SHADERS} ${JOLT_PHYSICS_SHADER_HEADERS})

# Create Jolt lib
add_library(Jolt ${JOLT_PHYSICS_SRC_FILES} ${JOLT_PHYSICS_SHADERS} ${JOLT_PHYSICS_SHADER_HEADERS} ${JOLT_PHYSICS_SPV_SHADERS} ${JOLT_PHYSICS_METAL_LIB})
add_library(Jolt::Jolt ALIAS Jolt)

if (BUILD_SHARED_LIBS)
	# Set default visibility to hidden
	set(CMAKE_CXX_VISIBILITY_PRESET hidden)

	if (GENERATE_DEBUG_SYMBOLS)
		if (MSVC)
			# MSVC specific option to enable PDB generation
			set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FASTLINK")
		else()
			# Clang/GCC option to enable debug symbol generation
			set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -g")
		endif()
	endif()

	# Set linker flags for other build types to be the same as release
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASEASAN "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASEUBSAN "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASETSAN "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASECOVERAGE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
	set(CMAKE_SHARED_LINKER_FLAGS_DISTRIBUTION "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")

	# Public define to instruct user code to import Jolt symbols (rather than use static linking)
	target_compile_definitions(Jolt PUBLIC JPH_SHARED_LIBRARY)

	# Private define to instruct the library to export symbols for shared linking
	target_compile_definitions(Jolt PRIVATE JPH_BUILD_SHARED_LIBRARY)
endif()

# Use repository as include directory when building, install directory when installing
target_include_directories(Jolt PUBLIC
	$<BUILD_INTERFACE:${PHYSICS_REPO_ROOT}>
	$<INSTALL_INTERFACE:include/>)

# Code coverage doesn't work when using precompiled headers
if (CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND MSVC)
	# The Ninja Multi-Config generator errors out when selectively disabling precompiled headers for certain configurations.
	# See: https://github.com/jrouwe/JoltPhysics/issues/1211
	target_precompile_headers(Jolt PRIVATE "${JOLT_PHYSICS_ROOT}/Jolt.h")
else()
	target_precompile_headers(Jolt PRIVATE "$<$<NOT:$<CONFIG:ReleaseCoverage>>:${JOLT_PHYSICS_ROOT}/Jolt.h>")
endif()

# Set the NDEBUG define for release builds
target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Release,Distribution,ReleaseASAN,ReleaseUBSAN,ReleaseTSAN,ReleaseCoverage>:NDEBUG>")

# ASAN and TSAN should use the default allocators
target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:ReleaseASAN,ReleaseTSAN>:JPH_DISABLE_TEMP_ALLOCATOR;JPH_DISABLE_CUSTOM_ALLOCATOR>")

# Setting floating point exceptions
if (FLOATING_POINT_EXCEPTIONS_ENABLED AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Debug,Release>:JPH_FLOATING_POINT_EXCEPTIONS_ENABLED>")
endif()

# Setting the disable custom allocator flag
if (DISABLE_CUSTOM_ALLOCATOR)
	target_compile_definitions(Jolt PUBLIC JPH_DISABLE_CUSTOM_ALLOCATOR)
endif()

# Setting enable asserts flag
if (USE_ASSERTS)
	target_compile_definitions(Jolt PUBLIC JPH_ENABLE_ASSERTS)
endif()

# Setting double precision flag
if (DOUBLE_PRECISION)
	target_compile_definitions(Jolt PUBLIC JPH_DOUBLE_PRECISION)
endif()

# Setting to attempt cross platform determinism
if (CROSS_PLATFORM_DETERMINISTIC)
	target_compile_definitions(Jolt PUBLIC JPH_CROSS_PLATFORM_DETERMINISTIC)
endif()

# Setting to determine number of bits in ObjectLayer
if (OBJECT_LAYER_BITS)
	target_compile_definitions(Jolt PUBLIC JPH_OBJECT_LAYER_BITS=${OBJECT_LAYER_BITS})
endif()

if (USE_STD_VECTOR)
	target_compile_definitions(Jolt PUBLIC JPH_USE_STD_VECTOR)
endif()

# Setting to periodically trace broadphase stats to help determine if the broadphase layer configuration is optimal
if (TRACK_BROADPHASE_STATS)
	target_compile_definitions(Jolt PUBLIC JPH_TRACK_BROADPHASE_STATS)
endif()

# Setting to periodically trace narrowphase stats to help determine which collision queries could be optimized
if (TRACK_NARROWPHASE_STATS)
	target_compile_definitions(Jolt PUBLIC JPH_TRACK_NARROWPHASE_STATS)
endif()

# Setting to track simulation timings per body
if (JPH_TRACK_SIMULATION_STATS)
	target_compile_definitions(Jolt PUBLIC JPH_TRACK_SIMULATION_STATS)
endif()

# Compile against DirectX 12
if (JPH_USE_DX12)
	target_compile_definitions(Jolt PUBLIC JPH_USE_DX12)
	target_link_libraries(Jolt LINK_PUBLIC dxgi.lib d3d12.lib d3dcompiler.lib dxguid.lib)

	# Use DXC compiler to compile shaders, when off falls back to FXC
	if (JPH_USE_DXC)
		target_compile_definitions(Jolt PUBLIC JPH_USE_DXC)
		target_link_libraries(Jolt LINK_PUBLIC dxcompiler.lib)
	endif()
endif()

# Compile against Vulkan
if (JPH_USE_VK)
	target_compile_definitions(Jolt PUBLIC JPH_USE_VK)

	target_include_directories(Jolt PUBLIC ${Vulkan_INCLUDE_DIRS})
	target_link_libraries(Jolt LINK_PUBLIC ${Vulkan_LIBRARIES})
endif()

# Compile against Metal
if (JPH_USE_MTL)
	target_compile_definitions(Jolt PUBLIC JPH_USE_MTL)

	target_link_libraries(Jolt LINK_PUBLIC "-framework Foundation -framework Metal -framework MetalKit")
endif()

# Compile CPU compute support
if (JPH_USE_CPU_COMPUTE)
	target_compile_definitions(Jolt PUBLIC JPH_USE_CPU_COMPUTE)
endif()

# Enable the debug renderer
if (DEBUG_RENDERER_IN_DISTRIBUTION)
	target_compile_definitions(Jolt PUBLIC "JPH_DEBUG_RENDERER")
elseif (DEBUG_RENDERER_IN_DEBUG_AND_RELEASE)
	target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Debug,Release,ReleaseASAN,ReleaseUBSAN,ReleaseTSAN>:JPH_DEBUG_RENDERER>")
endif()

# Enable the profiler
if (JPH_USE_EXTERNAL_PROFILE)
	set(JOLT_PROFILE_DEFINE JPH_EXTERNAL_PROFILE)
else()
	set(JOLT_PROFILE_DEFINE JPH_PROFILE_ENABLED)
endif()
if (PROFILER_IN_DISTRIBUTION)
	target_compile_definitions(Jolt PUBLIC "${JOLT_PROFILE_DEFINE}")
elseif (PROFILER_IN_DEBUG_AND_RELEASE)
	target_compile_definitions(Jolt PUBLIC "$<$<CONFIG:Debug,Release,ReleaseASAN,ReleaseUBSAN,ReleaseTSAN>:${JOLT_PROFILE_DEFINE}>")
endif()

# Compile the ObjectStream class and RTTI attribute information
if (ENABLE_OBJECT_STREAM)
	target_compile_definitions(Jolt PUBLIC JPH_OBJECT_STREAM)
endif()

# Emit the instruction set definitions to ensure that child projects use the same settings even if they override the used instruction sets (a mismatch causes link errors)
function(EMIT_X86_INSTRUCTION_SET_DEFINITIONS)
	if (USE_AVX512)
		target_compile_definitions(Jolt PUBLIC JPH_USE_AVX512)
	endif()
	if (USE_AVX2)
		target_compile_definitions(Jolt PUBLIC JPH_USE_AVX2)
	endif()
	if (USE_AVX)
		target_compile_definitions(Jolt PUBLIC JPH_USE_AVX)
	endif()
	if (USE_SSE4_1)
		target_compile_definitions(Jolt PUBLIC JPH_USE_SSE4_1)
	endif()
	if (USE_SSE4_2)
		target_compile_definitions(Jolt PUBLIC JPH_USE_SSE4_2)
	endif()
	if (USE_LZCNT)
		target_compile_definitions(Jolt PUBLIC JPH_USE_LZCNT)
	endif()
	if (USE_TZCNT)
		target_compile_definitions(Jolt PUBLIC JPH_USE_TZCNT)
	endif()
	if (USE_F16C)
		target_compile_definitions(Jolt PUBLIC JPH_USE_F16C)
	endif()
	if (USE_FMADD AND NOT CROSS_PLATFORM_DETERMINISTIC)
		target_compile_definitions(Jolt PUBLIC JPH_USE_FMADD)
	endif()
endfunction()

# Add the compiler commandline flags to select the right instruction sets
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86" OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		if (USE_AVX512)
			target_compile_options(Jolt PUBLIC /arch:AVX512)
		elseif (USE_AVX2)
			target_compile_options(Jolt PUBLIC /arch:AVX2)
		elseif (USE_AVX)
			target_compile_options(Jolt PUBLIC /arch:AVX)
		endif()
		EMIT_X86_INSTRUCTION_SET_DEFINITIONS()
	endif()
else()
	if (XCODE)
		# XCode builds for multiple architectures, we can't set global flags
	elseif (CROSS_COMPILE_ARM OR CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
		# ARM64 uses no special commandline flags
	elseif (EMSCRIPTEN)
		if (USE_WASM_SIMD)
			# Jolt currently doesn't implement the WASM specific SIMD intrinsics so uses the SSE 4.2 intrinsics
			# See: https://emscripten.org/docs/porting/simd.html#webassembly-simd-intrinsics
			# Note that this does not require the browser to actually support SSE 4.2 it merely means that it can translate those instructions to WASM SIMD instructions
			target_compile_options(Jolt PUBLIC -msimd128 -msse4.2)
		endif()
		if (JPH_USE_WASM64)
			target_compile_options(Jolt PUBLIC -sMEMORY64)
			target_link_options(Jolt PUBLIC -sMEMORY64)
		endif()
	elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
		# x86 and x86_64
		# On 32-bit builds we need to default to using SSE instructions, the x87 FPU instructions have higher intermediate precision
		# which will cause problems in the collision detection code (the effect is similar to leaving FMA on, search for
		# JPH_PRECISE_MATH_ON for the locations where this is a problem).

		if (USE_AVX512)
			target_compile_options(Jolt PUBLIC -mavx512f -mavx512vl -mavx512dq -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c)
		elseif (USE_AVX2)
			target_compile_options(Jolt PUBLIC -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c)
		elseif (USE_AVX)
			target_compile_options(Jolt PUBLIC -mavx -mpopcnt)
		elseif (USE_SSE4_2)
			target_compile_options(Jolt PUBLIC -msse4.2 -mpopcnt)
		elseif (USE_SSE4_1)
			target_compile_options(Jolt PUBLIC -msse4.1)
		else()
			target_compile_options(Jolt PUBLIC -msse2)
		endif()
		if (USE_LZCNT)
			target_compile_options(Jolt PUBLIC -mlzcnt)
		endif()
		if (USE_TZCNT)
			target_compile_options(Jolt PUBLIC -mbmi)
		endif()
		if (USE_F16C)
			target_compile_options(Jolt PUBLIC -mf16c)
		endif()
		if (USE_FMADD AND NOT CROSS_PLATFORM_DETERMINISTIC)
			target_compile_options(Jolt PUBLIC -mfma)
		endif()

		if (NOT MSVC)
			target_compile_options(Jolt PUBLIC -mfpmath=sse)
		endif()

		EMIT_X86_INSTRUCTION_SET_DEFINITIONS()
	endif()
endif()

# On Unix flavors we need the pthread library
if (NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows") AND NOT EMSCRIPTEN)
	target_compile_options(Jolt PUBLIC -pthread)
	target_link_options(Jolt PUBLIC -pthread)
endif()

if (EMSCRIPTEN)
	# We need more than the default 64KB stack and 16MB memory
	# In your application, specify at least -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728
	# Also disable warning: running limited binaryen optimizations because DWARF info requested (or indirectly required)
	target_link_options(Jolt PUBLIC -Wno-limited-postlink-optimizations)
endif()

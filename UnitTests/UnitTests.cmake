# Root
set(UNIT_TESTS_ROOT ${PHYSICS_REPO_ROOT}/UnitTests)

# Source files
set(UNIT_TESTS_SRC_FILES
	${UNIT_TESTS_ROOT}/Core/ArrayTest.cpp
	${UNIT_TESTS_ROOT}/Core/BinaryHeapTest.cpp
	${UNIT_TESTS_ROOT}/Core/FPFlushDenormalsTest.cpp
	${UNIT_TESTS_ROOT}/Core/HashCombineTest.cpp
	${UNIT_TESTS_ROOT}/Core/InsertionSortTest.cpp
	${UNIT_TESTS_ROOT}/Core/JobSystemTest.cpp
	${UNIT_TESTS_ROOT}/Core/LinearCurveTest.cpp
	${UNIT_TESTS_ROOT}/Core/PreciseMathTest.cpp
	${UNIT_TESTS_ROOT}/Core/ScopeExitTest.cpp
	${UNIT_TESTS_ROOT}/Core/STLLocalAllocatorTest.cpp
	${UNIT_TESTS_ROOT}/Core/StringToolsTest.cpp
	${UNIT_TESTS_ROOT}/Core/QuickSortTest.cpp
	${UNIT_TESTS_ROOT}/Core/UnorderedSetTest.cpp
	${UNIT_TESTS_ROOT}/Core/UnorderedMapTest.cpp
	${UNIT_TESTS_ROOT}/doctest.h
	${UNIT_TESTS_ROOT}/Geometry/ClosestPointTests.cpp
	${UNIT_TESTS_ROOT}/Geometry/ConvexHullBuilderTest.cpp
	${UNIT_TESTS_ROOT}/Geometry/EllipseTest.cpp
	${UNIT_TESTS_ROOT}/Geometry/EPATests.cpp
	${UNIT_TESTS_ROOT}/Geometry/GJKTests.cpp
	${UNIT_TESTS_ROOT}/Geometry/PlaneTests.cpp
	${UNIT_TESTS_ROOT}/Geometry/RayAABoxTests.cpp
	${UNIT_TESTS_ROOT}/Layers.h
	${UNIT_TESTS_ROOT}/LoggingBodyActivationListener.h
	${UNIT_TESTS_ROOT}/LoggingContactListener.h
	${UNIT_TESTS_ROOT}/Math/BVec16Tests.cpp
	${UNIT_TESTS_ROOT}/Math/DMat44Tests.cpp
	${UNIT_TESTS_ROOT}/Math/DVec3Tests.cpp
	${UNIT_TESTS_ROOT}/Math/EigenValueSymmetricTests.cpp
	${UNIT_TESTS_ROOT}/Math/HalfFloatTests.cpp
	${UNIT_TESTS_ROOT}/Math/Mat44Tests.cpp
	${UNIT_TESTS_ROOT}/Math/MathTests.cpp
	${UNIT_TESTS_ROOT}/Math/MatrixTests.cpp
	${UNIT_TESTS_ROOT}/Math/QuatTests.cpp
	${UNIT_TESTS_ROOT}/Math/UVec4Tests.cpp
	${UNIT_TESTS_ROOT}/Math/TrigonometryTests.cpp
	${UNIT_TESTS_ROOT}/Math/Vec3Tests.cpp
	${UNIT_TESTS_ROOT}/Math/Vec4Tests.cpp
	${UNIT_TESTS_ROOT}/Math/VectorTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ActiveEdgesTests.cpp
	${UNIT_TESTS_ROOT}/Physics/BroadPhaseTests.cpp
	${UNIT_TESTS_ROOT}/Physics/CastShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/CharacterVirtualTests.cpp
	${UNIT_TESTS_ROOT}/Physics/CollideShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/CollidePointTests.cpp
	${UNIT_TESTS_ROOT}/Physics/CollisionGroupTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ContactListenerTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ConvexVsTrianglesTest.cpp
	${UNIT_TESTS_ROOT}/Physics/DistanceConstraintTests.cpp
	${UNIT_TESTS_ROOT}/Physics/EstimateCollisionResponseTest.cpp
	${UNIT_TESTS_ROOT}/Physics/HeightFieldShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/HingeConstraintTests.cpp
	${UNIT_TESTS_ROOT}/Physics/MotionQualityLinearCastTests.cpp
	${UNIT_TESTS_ROOT}/Physics/MutableCompoundShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ObjectLayerPairFilterTableTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ObjectLayerPairFilterMaskTests.cpp
	${UNIT_TESTS_ROOT}/Physics/OffsetCenterOfMassShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/PathConstraintTests.cpp
	${UNIT_TESTS_ROOT}/Physics/PhysicsDeterminismTests.cpp
	${UNIT_TESTS_ROOT}/Physics/PhysicsStepListenerTests.cpp
	${UNIT_TESTS_ROOT}/Physics/PhysicsTests.cpp
	${UNIT_TESTS_ROOT}/Physics/RayShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/SensorTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ShapeFilterTests.cpp
	${UNIT_TESTS_ROOT}/Physics/ShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/SixDOFConstraintTests.cpp
	${UNIT_TESTS_ROOT}/Physics/SliderConstraintTests.cpp
	${UNIT_TESTS_ROOT}/Physics/SoftBodyTests.cpp
	${UNIT_TESTS_ROOT}/Physics/SubShapeIDTest.cpp
	${UNIT_TESTS_ROOT}/Physics/TaperedCylinderShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/TransformedShapeTests.cpp
	${UNIT_TESTS_ROOT}/Physics/WheeledVehicleTests.cpp
	${UNIT_TESTS_ROOT}/PhysicsTestContext.cpp
	${UNIT_TESTS_ROOT}/PhysicsTestContext.h
	${UNIT_TESTS_ROOT}/UnitTestFramework.cpp
	${UNIT_TESTS_ROOT}/UnitTestFramework.h
	${UNIT_TESTS_ROOT}/UnitTests.cmake
)

if (ENABLE_OBJECT_STREAM)
	set(UNIT_TESTS_SRC_FILES
		${UNIT_TESTS_SRC_FILES}
		${UNIT_TESTS_ROOT}/ObjectStream/ObjectStreamTest.cpp
	)
endif()

# Group source files
source_group(TREE ${UNIT_TESTS_ROOT} FILES ${UNIT_TESTS_SRC_FILES})

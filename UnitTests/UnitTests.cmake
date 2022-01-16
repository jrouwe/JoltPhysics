# Root
set(UNIT_TESTS_ROOT ${PHYSICS_REPO_ROOT}/UnitTests)

# Source files
set(UNIT_TESTS_SRC_FILES
	${UNIT_TESTS_ROOT}/doctest.h
	${UNIT_TESTS_ROOT}/Physics/ConvexVsTrianglesTest.cpp
	${UNIT_TESTS_ROOT}/PhysicsTestContext.cpp
	${UNIT_TESTS_ROOT}/PhysicsTestContext.h
	${UNIT_TESTS_ROOT}/UnitTestFramework.cpp
	${UNIT_TESTS_ROOT}/UnitTestFramework.h
	${UNIT_TESTS_ROOT}/UnitTests.cmake
)

# Group source files
source_group(TREE ${UNIT_TESTS_ROOT} FILES ${UNIT_TESTS_SRC_FILES})

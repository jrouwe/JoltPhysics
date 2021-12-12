# Root
set(PERFORMANCE_TEST_ROOT ${PHYSICS_REPO_ROOT}/PerformanceTest)

# Source files
set(PERFORMANCE_TEST_SRC_FILES
	${PERFORMANCE_TEST_ROOT}/PerformanceTest.cpp
	${PERFORMANCE_TEST_ROOT}/PerformanceTest.cmake
)

# Group source files
source_group(TREE ${PERFORMANCE_TEST_ROOT} FILES ${PERFORMANCE_TEST_SRC_FILES})

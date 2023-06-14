# Root
set(PERFORMANCE_TEST_ROOT ${PHYSICS_REPO_ROOT}/PerformanceTest)

# Source files
set(PERFORMANCE_TEST_SRC_FILES
	${PERFORMANCE_TEST_ROOT}/PyramidScene.h
	${PERFORMANCE_TEST_ROOT}/PerformanceTest.cpp
	${PERFORMANCE_TEST_ROOT}/PerformanceTest.cmake
	${PERFORMANCE_TEST_ROOT}/PerformanceTestScene.h
	${PERFORMANCE_TEST_ROOT}/RagdollScene.h
	${PERFORMANCE_TEST_ROOT}/ConvexVsMeshScene.h
	${PERFORMANCE_TEST_ROOT}/Layers.h
)

# Group source files
source_group(TREE ${PERFORMANCE_TEST_ROOT} FILES ${PERFORMANCE_TEST_SRC_FILES})

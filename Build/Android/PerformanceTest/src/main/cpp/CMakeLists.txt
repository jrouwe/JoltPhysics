cmake_minimum_required(VERSION 3.10.2)

project("JoltPhysicsPerformanceTest")

# Make sure we include the app glue sources
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})

# Set repository root
set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../")

# Make targets
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
include(${PHYSICS_REPO_ROOT}/PerformanceTest/PerformanceTest.cmake)

# Link shared native library
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
add_library(PerformanceTest SHARED ${PERFORMANCE_TEST_SRC_FILES} ${APP_GLUE_DIR}/android_native_app_glue.c)
target_include_directories(PerformanceTest PUBLIC Jolt ${JOLT_PHYSICS_ROOT} ${PERFORMANCE_TEST_ROOT})
target_link_libraries(PerformanceTest Jolt android log)

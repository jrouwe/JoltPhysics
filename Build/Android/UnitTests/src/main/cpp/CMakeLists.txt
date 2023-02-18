cmake_minimum_required(VERSION 3.10.2)

project("JoltPhysicsUnitTests")

# Make sure we include the app glue sources
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})

# Set repository root
set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../")

# Make targets
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
include(${PHYSICS_REPO_ROOT}/UnitTests/UnitTests.cmake)

# Link shared native library
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
add_library(UnitTests SHARED ${UNIT_TESTS_SRC_FILES} ${APP_GLUE_DIR}/android_native_app_glue.c)
target_include_directories(UnitTests PUBLIC Jolt ${JOLT_PHYSICS_ROOT} ${UNIT_TESTS_ROOT})
target_link_libraries(UnitTests Jolt android log)

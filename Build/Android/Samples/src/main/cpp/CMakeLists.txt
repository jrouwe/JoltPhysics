cmake_minimum_required(VERSION 3.10.2)

project("JoltPhysicsSamples")

# Make sure we include the app glue sources
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})

# Set repository root
set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../")

# Configure Jolt
set(ENABLE_OBJECT_STREAM ON)
set(JPH_USE_VK ON)
set(JPH_USE_CPU_COMPUTE ON)
set(JPH_SHADER_OPTIMIZATION ON)

# Make targets
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
include(${PHYSICS_REPO_ROOT}/TestFramework/TestFramework.cmake)
if (NOT TEST_FRAMEWORK_AVAILABLE)
	message(FATAL_ERROR "Cannot build Samples because Vulkan is not available! Install the Vulkan SDK so that the shaders can be compiled.")
endif()
include(${PHYSICS_REPO_ROOT}/Samples/Samples.cmake)

# Link the app glue to the samples library
add_library(NativeAppGlue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)
target_link_libraries(Samples LINK_PUBLIC NativeAppGlue)

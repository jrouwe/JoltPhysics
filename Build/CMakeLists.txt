cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(JoltPhysics CXX)

option(USE_SSE4_2 "Enable SSE4.2" ON)
option(USE_AVX2 "Enable AVX2" ON)
option(USE_LZCNT "Enable LZCNT" ON)
option(USE_TZCNT "Enable TZCNT" ON)
option(USE_F16C "Enable F16C" ON)
option(USE_FMADD "Enable FMADD" ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseASAN;ReleaseUBSAN;ReleaseCoverage;Distribution")
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# Fill in the path to the asan libraries
	set(CLANG_LIB_PATH "\"$(VSInstallDir)\\VC\\Tools\\Llvm\\x64\\lib\\clang\\${CMAKE_CXX_COMPILER_VERSION}\\lib\\windows\"")
	
	# 64 bit architecture
	set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x64")

	# Set runtime library
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Set general compiler flags
	set(CMAKE_CXX_FLAGS "/std:c++17 /Zc:__cplusplus /GR- /Gm- /Wall /WX /EHsc /nologo /diagnostics:classic /FC /arch:AVX2 /fp:except- /Zc:inline /Zi /DWIN32 /D_WINDOWS /DUNICODE /D_UNICODE")
	
	# Set compiler flags for various configurations
	set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /GS /Od /Ob0 /RTC1 /DJPH_PROFILE_ENABLED /DJPH_DEBUG_RENDERER")
	set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /GS- /GL /Gy /O2 /Oi /Ot /DJPH_PROFILE_ENABLED /DJPH_DEBUG_RENDERER")
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "/DNDEBUG /GS- /GL /Gy /O2 /Oi /Ot")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "/DNDEBUG /DJPH_PROFILE_ENABLED /DJPH_DISABLE_TEMP_ALLOCATOR /DJPH_DEBUG_RENDERER -fsanitize=address /Od")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "/DNDEBUG /DJPH_PROFILE_ENABLED /DJPH_DEBUG_RENDERER -fsanitize=undefined,implicit-conversion")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "/DNDEBUG -fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	set(CMAKE_EXE_LINKER_FLAGS "/machine:x64 /SUBSYSTEM:WINDOWS /ignore:4221 /DEBUG:FASTLINK")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:fast") # Clang doesn't use fast math because it cannot be turned off inside a single compilation unit
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED") # Clang turns Float2 into a vector sometimes causing floating point exceptions
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /LTCG:incremental /OPT:ICF /OPT:REF")
		set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showFilenames -msse4.1")
		if (USE_SSE4_2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mpopcnt")
		endif()
		if (USE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mbmi")
		endif()
		if (USE_LZCNT)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlzcnt")
		endif()
		if (USE_TZCNT)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi")
		endif()
		if (USE_F16C)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mf16c")
		endif()
		if (USE_FMADD)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
		endif()
		set(CMAKE_EXE_LINKER_FLAGS_RELEASEASAN "/SUBSYSTEM:CONSOLE /LIBPATH:${CLANG_LIB_PATH} clang_rt.asan-x86_64.lib -wholearchive:clang_rt.asan-x86_64.lib clang_rt.asan_cxx-x86_64.lib -wholearchive:clang_rt.asan_cxx-x86_64.lib")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASEUBSAN "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASECOVERAGE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
	endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	# Set general compiler flags (do not use -ffast-math since it cannot be turned off in a single compilation unit)
	set(CMAKE_CXX_FLAGS "-g -std=c++17 -I. -Wall -Werror")

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		# Somehow -Wcomment doesn't want to be turned off from code and we need this because Doxygen MathJax uses it
		# Also turn off automatic fused multiply add contractions, there doesn't seem to be a way to do this selectively through the macro JPH_PRECISE_MATH_OFF
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment -ffp-contract=off")
	endif()

	# Platform specific compiler flags
	if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
		# X64
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
		if (USE_SSE4_2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mpopcnt")
		endif()
		if (USE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mbmi")
		endif()
		if (USE_LZCNT)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlzcnt")
		endif()
		if (USE_TZCNT)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi")
		endif()
		if (USE_F16C)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mf16c")
		endif()
		if (USE_FMADD)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfma")
		endif()
	elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
		# ARM64
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

	# Set compiler flags for various configurations
	set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DJPH_PROFILE_ENABLED -DJPH_DEBUG_RENDERER") # Clang reorders variables so that div by zero occurs if we enable exception checking
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DJPH_PROFILE_ENABLED -DJPH_DEBUG_RENDERER")
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-DNDEBUG -DJPH_PROFILE_ENABLED -DJPH_DISABLE_TEMP_ALLOCATOR -DJPH_DEBUG_RENDERER -fsanitize=address")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-DNDEBUG -DJPH_PROFILE_ENABLED -DJPH_DEBUG_RENDERER -fsanitize=undefined,implicit-conversion")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-DNDEBUG -fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Set repository root
set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)

# Make Jolt Library
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)

# Create UnitTests executable
include(${PHYSICS_REPO_ROOT}/UnitTests/UnitTests.cmake)
add_executable(UnitTests ${UNIT_TESTS_SRC_FILES})
target_include_directories(UnitTests PUBLIC ${JOLT_PHYSICS_ROOT} ${UNIT_TESTS_ROOT})
target_link_libraries (UnitTests LINK_PUBLIC Jolt)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_link_options(UnitTests PUBLIC "/SUBSYSTEM:CONSOLE")
endif()

# Register unit tests as a test so that it can be run with: 
# ctest --output-on-failure
enable_testing()
add_test(UnitTests UnitTests)

# Example 'Hello World' application
include(${PHYSICS_REPO_ROOT}/HelloWorld/HelloWorld.cmake)
add_executable(HelloWorld ${HELLO_WORLD_SRC_FILES})
target_include_directories(HelloWorld PUBLIC ${JOLT_PHYSICS_ROOT} ${HELLO_WORLD_ROOT})
target_link_libraries (HelloWorld LINK_PUBLIC Jolt)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_link_options(HelloWorld PUBLIC "/SUBSYSTEM:CONSOLE")
endif()

# Performance Test application
include(${PHYSICS_REPO_ROOT}/PerformanceTest/PerformanceTest.cmake)
add_executable(PerformanceTest ${PERFORMANCE_TEST_SRC_FILES})
target_include_directories(PerformanceTest PUBLIC ${JOLT_PHYSICS_ROOT} ${PERFORMANCE_TEST_ROOT})
target_link_libraries (PerformanceTest LINK_PUBLIC Jolt)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_link_options(PerformanceTest PUBLIC "/SUBSYSTEM:CONSOLE")
endif()
set_property(TARGET PerformanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	# Windows only targets
	include(${PHYSICS_REPO_ROOT}/TestFramework/TestFramework.cmake)
	include(${PHYSICS_REPO_ROOT}/Samples/Samples.cmake)
	include(${PHYSICS_REPO_ROOT}/JoltViewer/JoltViewer.cmake)
endif()

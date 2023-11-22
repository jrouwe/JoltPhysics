cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(JoltPhysics CXX)

# When turning this option on, the library will be compiled using doubles for positions. This allows for much bigger worlds.
option(DOUBLE_PRECISION "Use double precision math" OFF)

# When turning this option on, the library will be compiled with debug symbols
option(GENERATE_DEBUG_SYMBOLS "Generate debug symbols" ON)

# When turning this option on, the library will override the default CMAKE_CXX_FLAGS_DEBUG/RELEASE values, otherwise they will use the platform defaults
option(OVERRIDE_CXX_FLAGS "Override CMAKE_CXX_FLAGS_DEBUG/RELEASE" ON)

# When turning this option on, the library will be compiled in such a way to attempt to keep the simulation deterministic across platforms
option(CROSS_PLATFORM_DETERMINISTIC "Cross platform deterministic" OFF)

# When turning this option on, the library will be compiled for ARM (aarch64-linux-gnu), requires compiling with clang
option(CROSS_COMPILE_ARM "Cross compile to aarch64-linux-gnu" OFF)

# When turning this option on, Jolt will be compiled as a shared library and public symbols will be exported.
option(BUILD_SHARED_LIBS "Compile Jolt as a shared library" OFF)

# When turning this option on, the library will be compiled with interprocedural optimizations enabled, also known as link-time optimizations or link-time code generation.
# Note that if you turn this on you need to use SET_INTERPROCEDURAL_OPTIMIZATION() or set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) to enable LTO specificly for your own project as well.
# If you don't do this you may get an error: /usr/bin/ld: libJolt.a: error adding symbols: file format not recognized
option(INTERPROCEDURAL_OPTIMIZATION "Enable interprocedural optimizations" ON)

# When turning this on, in Debug and Release mode, the library will emit extra code to ensure that the 4th component of a 3-vector is kept the same as the 3rd component
# and will enable floating point exceptions during simulation to detect divisions by zero.
# Note that this currently only works using MSVC. Clang turns Float2 into a SIMD vector sometimes causing floating point exceptions (the option is ignored).
option(FLOATING_POINT_EXCEPTIONS_ENABLED "Enable floating point exceptions" ON)

# Number of bits to use in ObjectLayer. Can be 16 or 32.
option(OBJECT_LAYER_BITS "Number of bits in ObjectLayer" 16)

# Select X86 processor features to use (if everything is off it will be SSE2 compatible)
option(USE_SSE4_1 "Enable SSE4.1" ON)
option(USE_SSE4_2 "Enable SSE4.2" ON)
option(USE_AVX "Enable AVX" ON)
option(USE_AVX2 "Enable AVX2" ON)
option(USE_AVX512 "Enable AVX512" OFF)
option(USE_LZCNT "Enable LZCNT" ON)
option(USE_TZCNT "Enable TZCNT" ON)
option(USE_F16C "Enable F16C" ON)
option(USE_FMADD "Enable FMADD" ON)

# Enable all warnings
option(ENABLE_ALL_WARNINGS "Enable all warnings and warnings as errors" ON)

# Setting to periodically trace broadphase stats to help determine if the broadphase layer configuration is optimal
option(TRACK_BROADPHASE_STATS "Track Broadphase Stats" OFF)

# Setting to periodically trace narrowphase stats to help determine which collision queries could be optimized
option(TRACK_NARROWPHASE_STATS "Track Narrowphase Stats" OFF)

include(CMakeDependentOption)

# Ability to toggle between the static and DLL versions of the MSVC runtime library
# Windows Store only supports the DLL version
cmake_dependent_option(USE_STATIC_MSVC_RUNTIME_LIBRARY "Use the static MSVC runtime library" ON "MSVC;NOT WINDOWS_STORE" OFF)

# Determine which configurations exist
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseASAN;ReleaseUBSAN;ReleaseCoverage;Distribution")
endif()

if (MSVC)
	# Fill in the path to the asan libraries
	set(CLANG_LIB_PATH "\"$(VSInstallDir)\\VC\\Tools\\Llvm\\x64\\lib\\clang\\${CMAKE_CXX_COMPILER_VERSION}\\lib\\windows\"")

	# 64 bit architecture
	set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x64")

	# Set runtime library
	if (USE_STATIC_MSVC_RUNTIME_LIBRARY)
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()

	# Set general compiler flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /Gm- /MP /nologo /diagnostics:classic /FC /fp:except- /Zc:inline")

	# Enable warnings
	if (ENABLE_ALL_WARNINGS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /WX")
	endif()

	# Optionally generate debug symbols
	if (GENERATE_DEBUG_SYMBOLS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	endif()

	# Remove any existing compiler flag that enables RTTI
	string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

	# Set compiler flag for disabling RTTI
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")

	if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
		# On ARM the exception handling flag is missing which causes warnings
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()

	# Set compiler flags for various configurations
	set(CMAKE_CXX_FLAGS_DEBUG "/GS /Od /Ob0 /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "/GS- /Gy /O2 /Oi /Ot")
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "/GS- /Gy /O2 /Oi /Ot")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address /Od")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS /ignore:4221")
	if (GENERATE_DEBUG_SYMBOLS)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	endif()
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		if (CROSS_PLATFORM_DETERMINISTIC)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast") # Clang doesn't use fast math because it cannot be turned off inside a single compilation unit
		endif()
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showFilenames")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments") # Clang emits warnings about unused arguments such as /MP and /GL
		set(CMAKE_EXE_LINKER_FLAGS_RELEASEASAN "/SUBSYSTEM:CONSOLE /LIBPATH:${CLANG_LIB_PATH} clang_rt.asan-x86_64.lib -wholearchive:clang_rt.asan-x86_64.lib clang_rt.asan_cxx-x86_64.lib -wholearchive:clang_rt.asan_cxx-x86_64.lib")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASEUBSAN "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASECOVERAGE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
	endif()
else()
	# Enable warnings
	if (ENABLE_ALL_WARNINGS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
	endif()

	# Optionally generate debug symbols
	if (GENERATE_DEBUG_SYMBOLS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	endif()

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		# Also disable -Wstringop-overflow or it will generate false positives that can't be disabled from code when link-time optimizations are enabled
		# Also turn off automatic fused multiply add contractions, there doesn't seem to be a way to do this selectively through the macro JPH_PRECISE_MATH_OFF
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-stringop-overflow -ffp-contract=off")
	else()
		# Do not use -ffast-math since it cannot be turned off in a single compilation unit under clang, see Core.h
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-model=precise")

		# On clang 14 and later we can turn off float contraction through a pragma, older versions and deterministic versions need it off always, see Core.h
		if (CMAKE_CXX_COMPILER_VERSION LESS 14 OR CROSS_PLATFORM_DETERMINISTIC)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-contract=off")
		endif()
	endif()

	# Cross compiler flags
	if (CROSS_COMPILE_ARM)
		set(CMAKE_CXX_FLAGS "--target=aarch64-linux-gnu ${CMAKE_CXX_FLAGS}")
	endif()

	# Set compiler flags for various configurations
	if (OVERRIDE_CXX_FLAGS)
		set(CMAKE_CXX_FLAGS_DEBUG "")
		set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	endif()
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-O0 -DJPH_NO_FORCE_INLINE -fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	if (NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"))
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	endif()
endif()

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Enable link time optimization in Release and Distribution mode if requested and available
function(SET_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF PARENT_SCOPE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DISTRIBUTION OFF PARENT_SCOPE)

	# On ARM, whole program optimization triggers an internal compiler error during code gen, so we don't turn it on
	if (INTERPROCEDURAL_OPTIMIZATION AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64") AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM"))
		include(CheckIPOSupported)
		check_ipo_supported(RESULT IS_IPO_SUPPORTED OUTPUT IPO_CHECK_OUTPUT)

		if (IS_IPO_SUPPORTED)
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON PARENT_SCOPE)
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DISTRIBUTION ON PARENT_SCOPE)
		else()
			message(WARNING "Interprocedural optimizations are not supported: ${IPO_CHECK_OUTPUT}")
		endif()
	endif()
endfunction()
SET_INTERPROCEDURAL_OPTIMIZATION()

# Set repository root
set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)

# Make Jolt Library
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
if (XCODE)
	# Ensure that we enable SSE4.2 for the x86_64 build, XCode builds multiple architectures
	set_property(TARGET Jolt PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=x86_64] "$(inherited) -msse4.2 -mpopcnt")
endif()

# Install Jolt library and includes
install(TARGETS Jolt DESTINATION lib)
foreach(SRC_FILE ${JOLT_PHYSICS_SRC_FILES})
	string(REPLACE ${PHYSICS_REPO_ROOT} "" RELATIVE_SRC_FILE ${SRC_FILE})
	get_filename_component(DESTINATION_PATH ${RELATIVE_SRC_FILE} DIRECTORY)
	if (NOT RELATIVE_SRC_FILE MATCHES "\.cpp")
		install(FILES ${SRC_FILE} DESTINATION include/${DESTINATION_PATH})
	endif()
endforeach()

# Check if we're the root CMakeLists.txt, if not we are included by another CMake file and we should disable everything except for the main library
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	# Ability to turn ON/OFF individual applications
	option(TARGET_UNIT_TESTS "Build Unit Tests" ON)
	option(TARGET_HELLO_WORLD "Build Hello World" ON)
	option(TARGET_PERFORMANCE_TEST "Build Performance Test" ON)
	option(TARGET_SAMPLES "Build Samples" ON)
	option(TARGET_VIEWER "Build JoltViewer" ON)

	if (TARGET_UNIT_TESTS)
		# Create UnitTests executable
		include(${PHYSICS_REPO_ROOT}/UnitTests/UnitTests.cmake)
		add_executable(UnitTests ${UNIT_TESTS_SRC_FILES})
		target_include_directories(UnitTests PUBLIC ${UNIT_TESTS_ROOT})
		target_link_libraries(UnitTests LINK_PUBLIC Jolt)
		target_precompile_headers(UnitTests PRIVATE ${JOLT_PHYSICS_ROOT}/Jolt.h)
		if (MSVC)
			target_link_options(UnitTests PUBLIC "/SUBSYSTEM:CONSOLE")
		endif()
		if (IOS)
			# Set the bundle information
			set_property(TARGET UnitTests PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/iOS/UnitTestsInfo.plist")
			set_property(TARGET UnitTests PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.joltphysics.unittests")
		endif()
		if (XCODE)
			# Ensure that we enable SSE4.2 for the x86_64 build, XCode builds multiple architectures
			set_property(TARGET UnitTests PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=x86_64] "$(inherited) -msse4.2 -mpopcnt")
		endif()

		# Register unit tests as a test so that it can be run with:
		# ctest --output-on-failure
		enable_testing()
		add_test(UnitTests UnitTests)
	endif()

	if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
		if (TARGET_HELLO_WORLD)
			# Example 'Hello World' application
			include(${PHYSICS_REPO_ROOT}/HelloWorld/HelloWorld.cmake)
			add_executable(HelloWorld ${HELLO_WORLD_SRC_FILES})
			target_include_directories(HelloWorld PUBLIC ${HELLO_WORLD_ROOT})
			target_link_libraries(HelloWorld LINK_PUBLIC Jolt)
			if (MSVC)
				target_link_options(HelloWorld PUBLIC "/SUBSYSTEM:CONSOLE")
			endif()
		endif()

		if (TARGET_PERFORMANCE_TEST)
			# Performance Test application
			include(${PHYSICS_REPO_ROOT}/PerformanceTest/PerformanceTest.cmake)
			add_executable(PerformanceTest ${PERFORMANCE_TEST_SRC_FILES})
			target_include_directories(PerformanceTest PUBLIC ${PERFORMANCE_TEST_ROOT})
			target_link_libraries(PerformanceTest LINK_PUBLIC Jolt)
			if (MSVC)
				target_link_options(PerformanceTest PUBLIC "/SUBSYSTEM:CONSOLE")
			endif()
			set_property(TARGET PerformanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")

			# Copy the assets folder
			add_custom_command(TARGET PerformanceTest PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PHYSICS_REPO_ROOT}/Assets/ $<TARGET_FILE_DIR:PerformanceTest>/Assets/)
		   endif()
	endif()

	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")) # ARM 32-bit is missing dinput8.lib
		# Windows only targets
		if (TARGET_SAMPLES OR TARGET_VIEWER)
			include(${PHYSICS_REPO_ROOT}/TestFramework/TestFramework.cmake)
		endif()
		if (TARGET_SAMPLES)
			include(${PHYSICS_REPO_ROOT}/Samples/Samples.cmake)
		endif()
		if (TARGET_VIEWER)
			include(${PHYSICS_REPO_ROOT}/JoltViewer/JoltViewer.cmake)
		endif()
	endif()
endif()

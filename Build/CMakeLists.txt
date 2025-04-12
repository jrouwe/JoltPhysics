cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(JoltPhysics VERSION 5.3.1 LANGUAGES CXX)

# When turning this option on, the library will be compiled using assertions. By default asserts are enabled in Debug build.
option(USE_ASSERTS "Enable asserts" OFF)

# When turning this option on, the library will be compiled using doubles for positions. This allows for much bigger worlds.
option(DOUBLE_PRECISION "Use double precision math" OFF)

# When turning this option on, the library will be compiled with debug symbols
option(GENERATE_DEBUG_SYMBOLS "Generate debug symbols" ON)

# Which type of debug symbols to generate, e.g. using source-map when compiling with emscripten makes compilation a lot faster
set(JPH_DEBUG_SYMBOL_FORMAT "" CACHE STRING "Which type of debug symbols to generate")

# When turning this option on, the library will override the default CMAKE_CXX_FLAGS_DEBUG/RELEASE values, otherwise they will use the platform defaults
option(OVERRIDE_CXX_FLAGS "Override CMAKE_CXX_FLAGS_DEBUG/RELEASE" ON)

# When turning this option on, the library will be compiled in such a way to attempt to keep the simulation deterministic across platforms
option(CROSS_PLATFORM_DETERMINISTIC "Cross platform deterministic" OFF)

# When turning this option on, the library will be compiled for ARM using the CROSS_COMPILE_ARM_TARGET architecture, requires compiling with clang
option(CROSS_COMPILE_ARM "Cross compile to the CROSS_COMPILE_ARM_TARGET architecture" OFF)

# When cross compiling to ARM this specifies which target to use. Can be 'aarch64-linux-gnu' for 64-bit or 'arm-linux-gnueabihf' for 32-bit
set(CROSS_COMPILE_ARM_TARGET "aarch64-linux-gnu" CACHE STRING "The target to use")

# When turning this option on, Jolt will be compiled as a shared library and public symbols will be exported.
option(BUILD_SHARED_LIBS "Compile Jolt as a shared library" OFF)

# When turning this option on, the library will be compiled with interprocedural optimizations enabled, also known as link-time optimizations or link-time code generation.
# Note that if you turn this on you need to use SET_INTERPROCEDURAL_OPTIMIZATION() or set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) to enable LTO specifically for your own project as well.
# If you don't do this you may get an error: /usr/bin/ld: libJolt.a: error adding symbols: file format not recognized
option(INTERPROCEDURAL_OPTIMIZATION "Enable interprocedural optimizations" ON)

# When turning this on, in Debug and Release mode, the library will emit extra code to ensure that the 4th component of a 3-vector is kept the same as the 3rd component
# and will enable floating point exceptions during simulation to detect divisions by zero.
# Note that this currently only works using MSVC. Clang turns Float2 into a SIMD vector sometimes causing floating point exceptions (the option is ignored).
option(FLOATING_POINT_EXCEPTIONS_ENABLED "Enable floating point exceptions" ON)

# When turning this on, the library will be compiled with C++ exceptions enabled.
# This adds some overhead and Jolt doesn't use exceptions so by default it is off.
option(CPP_EXCEPTIONS_ENABLED "Enable C++ exceptions" OFF)

# When turning this on, the library will be compiled with C++ RTTI enabled.
# This adds some overhead and Jolt doesn't use RTTI so by default it is off.
option(CPP_RTTI_ENABLED "Enable C++ RTTI" OFF)

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

# Enable SIMD for the WASM build. Note that this is currently off by default since not all browsers support this.
# See: https://caniuse.com/?search=WebAssembly%20SIMD (Safari got support in March 2023 and was the last major browser to get support).
option(USE_WASM_SIMD "Enable SIMD for WASM" OFF)

# Enable 64 bit WASM instead of the default 32 bit WASM. Note that this currently requires special commandline flags in browsers and nodejs to enable.
# E.g. use 'node --experimental-wasm-memory64 UnitTests.js' to run the unit tests in nodejs in 64 bit.
option(JPH_USE_WASM64 "Enable 64 bit WASM" OFF)

# Enable all warnings
option(ENABLE_ALL_WARNINGS "Enable all warnings and warnings as errors" ON)

# Setting to periodically trace broadphase stats to help determine if the broadphase layer configuration is optimal
option(TRACK_BROADPHASE_STATS "Track Broadphase Stats" OFF)

# Setting to periodically trace narrowphase stats to help determine which collision queries could be optimized
option(TRACK_NARROWPHASE_STATS "Track Narrowphase Stats" OFF)

# Enable the debug renderer in the Debug and Release builds. Note that DEBUG_RENDERER_IN_DISTRIBUTION will override this setting.
option(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE "Enable debug renderer in Debug and Release builds" ON)

# Setting to enable the debug renderer in all builds.
# Note that enabling this reduces the performance of the library even if you're not drawing anything.
option(DEBUG_RENDERER_IN_DISTRIBUTION "Enable debug renderer in all builds" OFF)

# Enable the profiler in Debug and Release builds. Note that PROFILER_IN_DISTRIBUTION will override this setting.
option(PROFILER_IN_DEBUG_AND_RELEASE "Enable the profiler in Debug and Release builds" ON)

# Enable the profiler in all builds.
# Note that enabling this reduces the performance of the library.
option(PROFILER_IN_DISTRIBUTION "Enable the profiler in all builds" OFF)

# Ability to use the external profiler using CMake config. Defines preprocessor JPH_EXTERNAL_PROFILE.
# Use external profiler set using ProfileStartMeasurement and ProfileEndMeasurement to profile.
# Option is available only when profiling is enabled using PROFILER_IN_DEBUG_AND_RELEASE or PROFILER_IN_DISTRIBUTION.
option(JPH_USE_EXTERNAL_PROFILE "Use external profiler when profiling is enabled" OFF)

# Setting this option will force the library to use malloc/free instead of allowing the user to override the memory allocator
option(DISABLE_CUSTOM_ALLOCATOR "Disable support for a custom memory allocator" OFF)

# Setting this option will force the library to use the STL vector instead of the custom Array class
option(USE_STD_VECTOR "Use std::vector instead of own Array class" OFF)

# Setting this option will compile the ObjectStream class and RTTI attribute information
option(ENABLE_OBJECT_STREAM "Compile the ObjectStream class and RTTI attribute information" ON)

# Enable installation
option(ENABLE_INSTALL "Generate installation target"  ON)

include(CMakeDependentOption)

# Ability to toggle between the static and DLL versions of the MSVC runtime library
# Windows Store only supports the DLL version
cmake_dependent_option(USE_STATIC_MSVC_RUNTIME_LIBRARY "Use the static MSVC runtime library" ON "MSVC;NOT WINDOWS_STORE" OFF)

# Enable Vulkan instead of DirectX
cmake_dependent_option(JPH_ENABLE_VULKAN "Enable Vulkan" ON "LINUX" OFF)

# Determine which configurations exist
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR) # Only do this when we're at the top level, see: https://gitlab.kitware.com/cmake/cmake/-/issues/24181
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseASAN;ReleaseUBSAN;ReleaseTSAN;ReleaseCoverage;Distribution")
	endif()
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

	if (NOT CPP_RTTI_ENABLED)
		# Set compiler flag for disabling RTTI
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
	else()
		# Set compiler flag for enabling RTTI
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")
	endif()

	if (NOT CPP_EXCEPTIONS_ENABLED)
		# Remove any existing compiler flag that enables exceptions
		string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

		# Disable warning about STL and compiler-generated types using noexcept when exceptions are disabled
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4577")
	else()
		# Enable exceptions
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
	endif()

	# Set compiler flags for various configurations
	if (OVERRIDE_CXX_FLAGS)
		set(CMAKE_CXX_FLAGS_DEBUG "/GS /Od /Ob0 /RTC1")
		set(CMAKE_CXX_FLAGS_RELEASE "/GS- /Gy /O2 /Oi /Ot")
	endif()
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address /Od")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
	set(CMAKE_CXX_FLAGS_RELEASETSAN "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=thread")
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
		set(CMAKE_EXE_LINKER_FLAGS_RELEASETSAN "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASECOVERAGE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
	endif()
else()
	# Enable warnings
	if (ENABLE_ALL_WARNINGS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
	endif()

	# Optionally generate debug symbols
	if (GENERATE_DEBUG_SYMBOLS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g${JPH_DEBUG_SYMBOL_FORMAT}")
	endif()

	if (NOT CPP_RTTI_ENABLED)
		# Set compiler flag for disabling RTTI
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	else()
		# Set compiler flag for enabling RTTI
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
	endif()

	if (NOT CPP_EXCEPTIONS_ENABLED)
		# Set compiler flag for disabling exception-handling
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
	else()
		# Set compiler flag for enabling exception-handling
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
	endif()

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		# Also disable -Wstringop-overflow or it will generate false positives that can't be disabled from code when link-time optimizations are enabled
		# Also disable -Wno-psabi to avoid messages of the form note: parameter passing for argument of type '...' changed in GCC 7.1
		# Also turn off automatic fused multiply add contractions, there doesn't seem to be a way to do this selectively through the macro JPH_PRECISE_MATH_OFF
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-stringop-overflow -Wno-psabi -ffp-contract=off")
	else()
		# Do not use -ffast-math since it cannot be turned off in a single compilation unit under clang, see Core.h
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-model=precise")

		# On clang 14 and later we can turn off float contraction through a pragma, older versions and deterministic versions need it off always, see Core.h
		# clang on LoongArch does not support such pragma, also turn off contraction for it.
		if (CMAKE_CXX_COMPILER_VERSION LESS 14 OR CROSS_PLATFORM_DETERMINISTIC OR "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "loongarch")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-contract=off")

			# Clang 20 and later complain with: overriding '-ffp-model=precise' option with '-ffp-contract=off' [-Woverriding-option], but this is exactly what we want.
			if (CMAKE_CXX_COMPILER_VERSION GREATER 19)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overriding-option")
			endif()
		endif()

		# Cross compiler flags
		if (CROSS_COMPILE_ARM AND NOT ("${CROSS_COMPILE_ARM_TARGET}" STREQUAL ""))
			set(CMAKE_CXX_FLAGS "--target=${CROSS_COMPILE_ARM_TARGET} ${CMAKE_CXX_FLAGS}")
		endif()
	endif()

	# See https://github.com/jrouwe/JoltPhysics/issues/922. When compiling with DOUBLE_PRECISION=YES and CMAKE_OSX_DEPLOYMENT_TARGET=10.12 clang triggers a warning that we silence here.
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-allocation")
	endif()

	# Set compiler flags for various configurations
	if (OVERRIDE_CXX_FLAGS)
		set(CMAKE_CXX_FLAGS_DEBUG "")
		set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	endif()
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
	set(CMAKE_CXX_FLAGS_RELEASETSAN "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=thread")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-O0 -DJPH_NO_FORCE_INLINE -fprofile-instr-generate -fcoverage-mapping")
endif()

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Enable link time optimization in Release and Distribution mode if requested and available
function(SET_INTERPROCEDURAL_OPTIMIZATION)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF PARENT_SCOPE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DISTRIBUTION OFF PARENT_SCOPE)

	# On ARM, whole program optimization triggers an internal compiler error during code gen, so we don't turn it on
	# When compiling as a shared lib with MinGW, turning on LTO causes errors of the form 'ld.exe: cannot export symbol X wrong type (4 vs 3)'
	if (INTERPROCEDURAL_OPTIMIZATION
		AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM64")
		AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
		AND (NOT CROSS_COMPILE_ARM OR ("${CROSS_COMPILE_ARM_TARGET}" STREQUAL "aarch64-linux-gnu"))
		AND NOT (MINGW AND BUILD_SHARED_LIBS))
		include(CheckIPOSupported)
		check_ipo_supported(RESULT IS_IPO_SUPPORTED OUTPUT IPO_CHECK_OUTPUT)

		if (IS_IPO_SUPPORTED)
			message("Interprocedural optimizations are turned on")
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON PARENT_SCOPE)
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DISTRIBUTION ON PARENT_SCOPE)
		else()
			message("Warning: Interprocedural optimizations are not supported for this target, turn off the option INTERPROCEDURAL_OPTIMIZATION to disable this warning")
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
if (ENABLE_INSTALL)
	include(GNUInstallDirs)
	install(TARGETS Jolt
		EXPORT JoltExport
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	foreach(SRC_FILE ${JOLT_PHYSICS_SRC_FILES})
		string(REPLACE ${PHYSICS_REPO_ROOT} "" RELATIVE_SRC_FILE ${SRC_FILE})
		get_filename_component(DESTINATION_PATH ${RELATIVE_SRC_FILE} DIRECTORY)
		if (NOT RELATIVE_SRC_FILE MATCHES "\.cpp")
			cmake_path(SET DST_FILE NORMALIZE "${CMAKE_INSTALL_INCLUDEDIR}/${DESTINATION_PATH}")
			install(FILES ${SRC_FILE} DESTINATION ${DST_FILE})
		endif()
	endforeach()

	# Export Jolt library
	export(TARGETS Jolt
		NAMESPACE Jolt::
		FILE JoltConfig.cmake)
	install(EXPORT JoltExport
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Jolt/
		NAMESPACE Jolt::
		FILE JoltConfig.cmake)
endif()

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

		if (EMSCRIPTEN)
			target_link_options(UnitTests PUBLIC -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728)
		endif()

		# Code coverage doesn't work when using precompiled headers
		if (CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND MSVC)
			# The Ninja Multi-Config generator errors out when selectively disabling precompiled headers for certain configurations.
			# See: https://github.com/jrouwe/JoltPhysics/issues/1211
			target_precompile_headers(UnitTests PRIVATE "${JOLT_PHYSICS_ROOT}/Jolt.h")
		else()
			target_precompile_headers(UnitTests PRIVATE "$<$<NOT:$<CONFIG:ReleaseCoverage>>:${JOLT_PHYSICS_ROOT}/Jolt.h>")
		endif()

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
			if (EMSCRIPTEN)
				target_link_options(HelloWorld PUBLIC -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728)
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
			if (EMSCRIPTEN)
				# Embed the assets for the RagdollScene
				target_link_options(PerformanceTest PUBLIC "SHELL:--preload-file ${PHYSICS_REPO_ROOT}/Assets/Human.tof@/Assets/Human.tof")
				target_link_options(PerformanceTest PUBLIC "SHELL:--preload-file ${PHYSICS_REPO_ROOT}/Assets/Human/dead_pose1.tof@/Assets/Human/dead_pose1.tof")
				target_link_options(PerformanceTest PUBLIC "SHELL:--preload-file ${PHYSICS_REPO_ROOT}/Assets/terrain2.bof@/Assets/terrain2.bof")
				target_link_options(PerformanceTest PUBLIC -sSTACK_SIZE=1048576 -sINITIAL_MEMORY=134217728)
			endif()
			set_property(TARGET PerformanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")
		endif()
	endif()

	if ((WIN32 OR LINUX OR ("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")) AND NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")) # ARM 32-bit is missing dinput8.lib
		# Windows only targets
		if (TARGET_SAMPLES OR TARGET_VIEWER)
			include(${PHYSICS_REPO_ROOT}/TestFramework/TestFramework.cmake)
		endif()
		if (TARGET_SAMPLES)
			if (TEST_FRAMEWORK_AVAILABLE)
				include(${PHYSICS_REPO_ROOT}/Samples/Samples.cmake)
			else()
				message("Cannot build Samples because Vulkan/DirectX SDK is not available!")
			endif()
		endif()
		if (TARGET_VIEWER)
			if (TEST_FRAMEWORK_AVAILABLE)
				include(${PHYSICS_REPO_ROOT}/JoltViewer/JoltViewer.cmake)
			else()
				message("Cannot build JoltViewer because Vulkan/DirectX SDK is not available!")
			endif()
		endif()
	endif()
endif()

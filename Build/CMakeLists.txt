cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(JoltPhysics CXX)

# Ability to turn ON/OFF individual applications
option(TARGET_UNIT_TESTS "Build Unit Tests" ON)
option(TARGET_HELLO_WORLD "Build Hello World" ON)
option(TARGET_PERFORMANCE_TEST "Build Performance Test" ON)
option(TARGET_SAMPLES "Build Samples" ON)
option(TARGET_VIEWER "Build JoltViewer" ON)

# Select X86 processor features to use (if everything is off it will be SSE2 compatible)
option(USE_SSE4_1 "Enable SSE4.1" ON)
option(USE_SSE4_2 "Enable SSE4.2" ON)
option(USE_AVX "Enable AVX" ON)
option(USE_AVX2 "Enable AVX2" ON)
option(USE_LZCNT "Enable LZCNT" ON)
option(USE_TZCNT "Enable TZCNT" ON)
option(USE_F16C "Enable F16C" ON)
option(USE_FMADD "Enable FMADD" ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReleaseASAN;ReleaseUBSAN;ReleaseCoverage;Distribution")
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
	# Fill in the path to the asan libraries
	set(CLANG_LIB_PATH "\"$(VSInstallDir)\\VC\\Tools\\Llvm\\x64\\lib\\clang\\${CMAKE_CXX_COMPILER_VERSION}\\lib\\windows\"")
	
	# 64 bit architecture
	set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x64")

	# Set runtime library
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
		# Windows store only supports multithreaded DLL
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	else()
		# Otherwise use static library
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()

	# Set general compiler flags
	set(CMAKE_CXX_FLAGS "/std:c++17 /Zc:__cplusplus /GR- /Gm- /Wall /WX /EHsc /MP /nologo /diagnostics:classic /FC /fp:except- /Zc:inline /Zi /DWIN32 /D_WINDOWS /DUNICODE /D_UNICODE")
	
	# Set compiler flags for various configurations
	set(CMAKE_CXX_FLAGS_DEBUG "/GS /Od /Ob0 /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "/GS- /GL /Gy /O2 /Oi /Ot")
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "/GS- /GL /Gy /O2 /Oi /Ot")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address /Od")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	set(CMAKE_EXE_LINKER_FLAGS "/machine:x64 /SUBSYSTEM:WINDOWS /ignore:4221 /DEBUG:FASTLINK")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast") # Clang doesn't use fast math because it cannot be turned off inside a single compilation unit
		if (USE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
		elseif (USE_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
		endif()	
		if (USE_SSE4_1)
			add_compile_definitions(JPH_USE_SSE4_1)
		endif()
		if (USE_SSE4_2)
			add_compile_definitions(JPH_USE_SSE4_2)
		endif()
		if (USE_LZCNT)
			add_compile_definitions(JPH_USE_LZCNT)
		endif()
		if (USE_TZCNT)
			add_compile_definitions(JPH_USE_TZCNT)
		endif()
		if (USE_F16C)
			add_compile_definitions(JPH_USE_F16C)
		endif()
		if (USE_FMADD)
			add_compile_definitions(JPH_USE_FMADD)
		endif()
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED") # Clang turns Float2 into a vector sometimes causing floating point exceptions
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /LTCG:incremental /OPT:ICF /OPT:REF")
		set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showFilenames")
		if (USE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c -mfma")
		elseif (USE_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mpopcnt")
		elseif (USE_SSE4_2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mpopcnt")
		elseif (USE_SSE4_1)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
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
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
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
		if (USE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c -mfma")
		elseif (USE_AVX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mpopcnt")
		elseif (USE_SSE4_2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mpopcnt")
		elseif (USE_SSE4_1)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
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
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3")
	set(CMAKE_CXX_FLAGS_RELEASEASAN "-fsanitize=address")
	set(CMAKE_CXX_FLAGS_RELEASEUBSAN "-fsanitize=undefined,implicit-conversion")
	set(CMAKE_CXX_FLAGS_RELEASECOVERAGE "-fprofile-instr-generate -fcoverage-mapping")

	# Set linker flags
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Set repository root
set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)

# Make Jolt Library
include(${PHYSICS_REPO_ROOT}/Jolt/Jolt.cmake)
if (IOS)
	# Ensure that we enable SSE4.2 for the x86_64 build, CMAKE_SYSTEM_PROCESSOR is not set for iOS
	set_property(TARGET Jolt PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=x86_64] "$(inherited) -msse4.2 -mpopcnt") 
endif()

if (TARGET_UNIT_TESTS)
    # Create UnitTests executable
    include(${PHYSICS_REPO_ROOT}/UnitTests/UnitTests.cmake)
    add_executable(UnitTests ${UNIT_TESTS_SRC_FILES})
    target_include_directories(UnitTests PUBLIC ${UNIT_TESTS_ROOT})
    target_link_libraries (UnitTests LINK_PUBLIC Jolt)
    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        target_link_options(UnitTests PUBLIC "/SUBSYSTEM:CONSOLE")
    endif()
    if (IOS)
		# Set the bundle information
		set_property(TARGET UnitTests PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/iOS/UnitTestsInfo.plist")
		set_property(TARGET UnitTests PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.joltphysics.unittests")

		# Ensure that we enable SSE4.2 for the x86_64 build, CMAKE_SYSTEM_PROCESSOR is not set for iOS
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
		target_link_libraries (HelloWorld LINK_PUBLIC Jolt)
		if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
			target_link_options(HelloWorld PUBLIC "/SUBSYSTEM:CONSOLE")
		endif()
	endif()

	if (TARGET_PERFORMANCE_TEST)
		# Performance Test application
		include(${PHYSICS_REPO_ROOT}/PerformanceTest/PerformanceTest.cmake)
		add_executable(PerformanceTest ${PERFORMANCE_TEST_SRC_FILES})
		target_include_directories(PerformanceTest PUBLIC ${PERFORMANCE_TEST_ROOT})
		target_link_libraries (PerformanceTest LINK_PUBLIC Jolt)
		if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
			target_link_options(PerformanceTest PUBLIC "/SUBSYSTEM:CONSOLE")
		endif()
		set_property(TARGET PerformanceTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")
	endif()
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
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

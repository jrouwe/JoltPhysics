// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeSystem.h>
#include <Jolt/Core/IncludeWindows.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
#include <filesystem>
#ifdef JPH_PLATFORM_LINUX
#include <unistd.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif

// Sets up the shader loader of a compute system so that it loads the compiled shaders from the Jolt/Shaders folder
inline void SetComputeShaderLoader(ComputeSystem *inComputeSystem)
{
	inComputeSystem->mShaderLoader = [](const char *inName, Array<uint8> &outData, String &outError) {
	#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
		// In macOS the shaders are copied to the bundle
		CFBundleRef bundle = CFBundleGetMainBundle();
		CFURLRef resources = CFBundleCopyResourcesDirectoryURL(bundle);
		CFURLRef absolute = CFURLCopyAbsoluteURL(resources);
		CFRelease(resources);
		CFStringRef path_string = CFURLCopyFileSystemPath(absolute, kCFURLPOSIXPathStyle);
		CFRelease(absolute);
		char path[PATH_MAX];
		CFStringGetCString(path_string, path, PATH_MAX, kCFStringEncodingUTF8);
		CFRelease(path_string);
		String base_path = String(path) + "/Jolt/Shaders/";
	#else
		// On other platforms, start searching up from the application path
		#ifdef JPH_PLATFORM_WINDOWS
			char application_path[MAX_PATH] = { 0 };
			GetModuleFileName(nullptr, application_path, MAX_PATH);
		#elif defined(JPH_PLATFORM_LINUX)
			char application_path[PATH_MAX] = { 0 };
			int count = readlink("/proc/self/exe", application_path, PATH_MAX);
			if (count > 0)
				application_path[count] = 0;
		#else
			// Not implemented
			const char *application_path = "";
		#endif
		String base_path;
		filesystem::path shader_path(application_path);
		while (!shader_path.empty())
		{
			filesystem::path parent_path = shader_path.parent_path();
			if (parent_path == shader_path)
				break;
			shader_path = parent_path;
			filesystem::path full_path = shader_path / "Jolt" / "Shaders" / "";
			if (filesystem::exists(full_path))
			{
				base_path = String(full_path.string());
				break;
			}
		}
	#endif

		// Open file
		std::ifstream input((base_path + inName).c_str(), std::ios::in | std::ios::binary);
		if (!input.is_open())
		{
			outError = String("Could not open shader file: ") + base_path + inName;
			#if defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
				outError += "\nThis can fail on macOS when dxc or spirv-cross could not be found so the shaders could not be compiled.";
			#endif
			return false;
		}

		// Read contents of file
		input.seekg(0, ios_base::end);
		ifstream::pos_type length = input.tellg();
		input.seekg(0, ios_base::beg);
		outData.resize(size_t(length));
		if (length == 0)
			return true;
		input.read((char *)&outData[0], length);
		return true;
	};
}

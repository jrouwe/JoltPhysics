// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/AssetStream.h>
#include <Utils/Log.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <filesystem>
#ifdef JPH_PLATFORM_LINUX
#include <unistd.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

String AssetStream::sGetAssetsBasePath()
{
	static String result = []() {
		// Start with the application path
	#ifdef JPH_PLATFORM_WINDOWS
		char application_path[MAX_PATH] = { 0 };
		GetModuleFileName(nullptr, application_path, MAX_PATH);
	#elif defined(JPH_PLATFORM_LINUX)
		char application_path[PATH_MAX] = { 0 };
		int count = readlink("/proc/self/exe", application_path, PATH_MAX);
		if (count > 0)
			application_path[count] = 0;
	#else
		#error Unsupported platform
	#endif

		// Find the asset path
		filesystem::path asset_path(application_path);
		while (!asset_path.empty())
		{
			filesystem::path parent_path = asset_path.parent_path();
			if (parent_path == asset_path)
				break;
			asset_path = parent_path;
			if (filesystem::exists(asset_path / "Assets"))
				break;
		}
		asset_path /= "Assets";
		asset_path /= "";
		return String(asset_path.string());
	}();

	return result;
}

AssetStream::AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode) :
	mStream((sGetAssetsBasePath() + inFileName).c_str(), inOpenMode)
{
	if (!mStream.is_open())
		FatalError("Failed to open file %s", inFileName);
}

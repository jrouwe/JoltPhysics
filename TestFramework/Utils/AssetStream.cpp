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
#ifdef JPH_PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

#ifdef JPH_PLATFORM_ANDROID

static AAssetManager *sAssetManager = nullptr;

void AssetStream::sSetAssetManager(AAssetManager *inAssetManager)
{
	sAssetManager = inAssetManager;
}

String AssetStream::sGetAssetsBasePath()
{
	// Assets are packaged in the APK and are accessed through the asset manager using a relative path
	return String();
}

bool AssetStream::sExists(const String &inFileName)
{
	JPH_ASSERT(sAssetManager != nullptr);
	AAsset *asset = AAssetManager_open(sAssetManager, inFileName.c_str(), AASSET_MODE_UNKNOWN);
	if (asset == nullptr)
		return false;
	AAsset_close(asset);
	return true;
}

AssetStream::AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode)
{
	JPH_ASSERT(sAssetManager != nullptr);
	mAsset = AAssetManager_open(sAssetManager, inFileName, AASSET_MODE_BUFFER);
	if (mAsset == nullptr)
		FatalError("Failed to open file %s", inFileName);

	// Get a pointer to the (uncompressed) asset data
	const void *data = AAsset_getBuffer(mAsset);
	if (data == nullptr)
		FatalError("Failed to read file %s", inFileName);
	mBuffer.SetBuffer(static_cast<const char *>(data), size_t(AAsset_getLength64(mAsset)));
}

AssetStream::~AssetStream()
{
	if (mAsset != nullptr)
		AAsset_close(mAsset);
}

void AssetStream::MemoryStreamBuf::SetBuffer(const char *inData, size_t inSize)
{
	char *data = const_cast<char *>(inData); // The buffer is only used for reading
	setg(data, data, data + inSize);
}

AssetStream::MemoryStreamBuf::pos_type AssetStream::MemoryStreamBuf::seekoff(off_type inOffset, std::ios_base::seekdir inDir, std::ios_base::openmode inMode)
{
	off_type pos;
	switch (inDir)
	{
	case std::ios_base::beg:
		pos = inOffset;
		break;

	case std::ios_base::cur:
		pos = off_type(gptr() - eback()) + inOffset;
		break;

	case std::ios_base::end:
	default:
		pos = off_type(egptr() - eback()) + inOffset;
		break;
	}

	return seekpos(pos_type(pos), inMode);
}

AssetStream::MemoryStreamBuf::pos_type AssetStream::MemoryStreamBuf::seekpos(pos_type inPos, std::ios_base::openmode inMode)
{
	off_type pos = off_type(inPos);
	if ((inMode & std::ios_base::in) == 0 || pos < 0 || pos > off_type(egptr() - eback()))
		return pos_type(off_type(-1));

	setg(eback(), eback() + pos, egptr());
	return inPos;
}

#else

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
		std::filesystem::path asset_path(application_path);
		while (!asset_path.empty())
		{
			std::filesystem::path parent_path = asset_path.parent_path();
			if (parent_path == asset_path)
				break;
			asset_path = parent_path;
			if (std::filesystem::exists(asset_path / "Assets"))
				break;
		}
		asset_path /= "Assets";
		asset_path /= "";
		return String(asset_path.string());
	}();

	return result;
}

bool AssetStream::sExists(const String &inFileName)
{
	return std::filesystem::exists(sGetAssetsBasePath() + inFileName);
}

AssetStream::AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode) :
	mStream((sGetAssetsBasePath() + inFileName).c_str(), inOpenMode)
{
	if (!mStream.is_open())
		FatalError("Failed to open file %s", inFileName);
}

#endif

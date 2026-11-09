// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

#ifdef JPH_PLATFORM_ANDROID
struct AAssetManager;
struct AAsset;
#endif

/// An istream interface that reads data from a file in the Assets folder
class AssetStream
{
public:
	/// Constructor
					AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode);
					AssetStream(const String &inFileName, std::ios_base::openmode inOpenMode) : AssetStream(inFileName.c_str(), inOpenMode) { }

#ifdef JPH_PLATFORM_ANDROID
	/// Destructor
					~AssetStream();

	/// Set the asset manager that is used to read files from the APK
	static void		sSetAssetManager(AAssetManager *inAssetManager);
#endif

	/// Get the path to the assets folder
	static String	sGetAssetsBasePath();

	/// Check if a file exists in the Assets folder
	static bool		sExists(const String &inFileName);

	/// Get the stream
	std::istream &	Get()							{ return mStream; }

private:
#ifdef JPH_PLATFORM_ANDROID
	/// Stream buffer that reads from a block of memory
	class MemoryStreamBuf : public std::streambuf
	{
	public:
		void		SetBuffer(const char *inData, size_t inSize);

	protected:
		virtual pos_type seekoff(off_type inOffset, std::ios_base::seekdir inDir, std::ios_base::openmode inMode) override;
		virtual pos_type seekpos(pos_type inPos, std::ios_base::openmode inMode) override;
	};

	AAsset *		mAsset = nullptr;
	MemoryStreamBuf	mBuffer;
	std::istream	mStream { &mBuffer };
#else
	std::ifstream	mStream;
#endif
};

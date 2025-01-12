// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

/// An istream interface that reads data from a file in the Assets folder
class AssetStream
{
public:
	/// Constructor
					AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode);
					AssetStream(const String &inFileName, std::ios_base::openmode inOpenMode) : AssetStream(inFileName.c_str(), inOpenMode) { }

	/// Get the path to the assets folder
	static String	sGetAssetsBasePath();

	/// Get the stream
	std::istream &	Get()							{ return mStream; }

private:
	std::ifstream	mStream;
};

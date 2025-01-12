// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/AssetStream.h>
#include <Utils/Log.h>

#include <Cocoa/Cocoa.h>

String AssetStream::sGetAssetsBasePath()
{
	NSBundle *bundle = [NSBundle mainBundle];
	String path = [[[bundle resourceURL] path] cStringUsingEncoding: NSUTF8StringEncoding];
	path += "/";
	return path;
}

AssetStream::AssetStream(const char *inFileName, std::ios_base::openmode inOpenMode) :
	mStream((sGetAssetsBasePath() + inFileName).c_str(), inOpenMode)
{
	if (!mStream.is_open())
		FatalError("Failed to open file %s", inFileName);
}

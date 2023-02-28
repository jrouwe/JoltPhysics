// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/ReadData.h>
#include <Utils/Log.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

// Read file contents
Array<uint8> ReadData(const char *inFileName)
{
	Array<uint8> data;
	ifstream input(inFileName, std::ios::binary);
	if (!input)
		FatalError("Unable to open file: %s", inFileName);
	input.seekg(0, ios_base::end);
	ifstream::pos_type length = input.tellg();
	input.seekg(0, ios_base::beg);
	data.resize(size_t(length));
	input.read((char *)&data[0], length);
	if (!input)
		FatalError("Unable to read file: %s", inFileName);
	return data;
}

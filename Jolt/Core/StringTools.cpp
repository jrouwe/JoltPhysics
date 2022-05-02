// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/StringTools.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <cstdarg>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

string StringFormat(const char *inFMT, ...)
{
	char buffer[1024];

	// Format the string
	va_list list;
	va_start(list, inFMT);
	vsnprintf(buffer, sizeof(buffer), inFMT, list);

	return string(buffer);
}

void StringReplace(string &ioString, const string_view &inSearch, const string_view &inReplace)
{
	size_t index = 0;
	for (;;)
	{
		 index = ioString.find(inSearch, index);
		 if (index == std::string::npos) 
			 break;

		 ioString.replace(index, inSearch.size(), inReplace);

		 index += inReplace.size();
	}
}

void StringToVector(const string_view &inString, vector<string> &outVector, const string_view &inDelimiter, bool inClearVector)
{
	JPH_ASSERT(inDelimiter.size() > 0);

	// Ensure vector empty
	if (inClearVector)
		outVector.clear(); 

	// No string? no elements
	if (inString.empty())
		return;

	// Start with initial string
	string s(inString);

	// Add to vector while we have a delimiter
	size_t i;
	while (!s.empty() && (i = s.find(inDelimiter)) != string::npos)
	{
		outVector.push_back(s.substr(0, i));
		s.erase(0, i + inDelimiter.length());
	}

	// Add final element
	outVector.push_back(s);
}

void VectorToString(const vector<string> &inVector, string &outString, const string_view &inDelimiter)
{
	// Ensure string empty
	outString.clear();

	for (const string &s : inVector)
	{
		// Add delimiter if not first element
		if (!outString.empty())
			outString.append(inDelimiter);

		// Add element
		outString.append(s);
	}
}

string ToLower(const string_view &inString)
{
	string out;
	out.reserve(inString.length());
	for (char c : inString)
		out.push_back((char)tolower(c));
	return out;
}

const char *NibbleToBinary(uint32 inNibble)
{
	static const char *nibbles[] = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };
	return nibbles[inNibble & 0xf];
}

JPH_NAMESPACE_END

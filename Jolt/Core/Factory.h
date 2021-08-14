// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/RTTI.h>
#include <unordered_map>

namespace JPH {

/// Factory, to create RTTI objects
class Factory
{
public:
	/// Create an object
	void *						CreateObject(const char *inName);

	/// Find type info for a specific class by name
	const RTTI *				Find(const char *inName);

	/// Find type info for a specific class by hash
	const RTTI *				Find(uint32 inHash);

	/// Register an object with the factory. Returns false on failure.
	bool						Register(const RTTI *inRTTI);

	/// Get all registered classes
	vector<const RTTI *>		GetAllClasses();

	/// Singleton factory instance
	static Factory 				sInstance;

private:
	using ClassNameMap = unordered_map<string, const RTTI *>;

	using ClassHashMap = unordered_map<uint32, const RTTI *>;

	/// Map of class names to type info
	ClassNameMap				mClassNameMap;

	// Map of class hash to type info
	ClassHashMap				mClassHashMap;
};

} // JPH
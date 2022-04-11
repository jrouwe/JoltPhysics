// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// @brief Helper function that hashes a single value into ioSeed
/// Taken from: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
template <typename T>
inline void hash_combine_helper(std::size_t &ioSeed, const T &inValue)
{
	std::hash<T> hasher;
    ioSeed ^= hasher(inValue) + 0x9e3779b9 + (ioSeed << 6) + (ioSeed >> 2);
}

/// Hash combiner to use a custom struct in an unordered map or set
///
/// Usage:
///
///		struct SomeHashKey 
///		{
///		    std::string key1;
///		    std::string key2;
///		    bool key3;
///		};
/// 
///		JPH_MAKE_HASHABLE(SomeHashKey, t.key1, t.key2, t.key3)
template <typename... Values>
inline void hash_combine(std::size_t &ioSeed, Values... inValues) 
{
	// Hash all values together using a fold expression
	(hash_combine_helper(ioSeed, inValues), ...);
}

JPH_NAMESPACE_END

JPH_SUPPRESS_WARNING_PUSH
JPH_CLANG_SUPPRESS_WARNING("-Wc++98-compat-pedantic")

#define JPH_MAKE_HASH_STRUCT(type, name, ...)				\
	struct [[nodiscard]] name								\
	{														\
        std::size_t operator()(const type &t) const			\
		{													\
            std::size_t ret = 0;							\
            ::JPH::hash_combine(ret, __VA_ARGS__);			\
            return ret;										\
        }													\
    };

#define JPH_MAKE_HASHABLE(type, ...)						\
	JPH_SUPPRESS_WARNING_PUSH								\
	JPH_SUPPRESS_WARNINGS									\
    namespace std											\
	{														\
        template<>											\
		JPH_MAKE_HASH_STRUCT(type, hash<type>, __VA_ARGS__)	\
    }														\
	JPH_SUPPRESS_WARNING_POP

JPH_SUPPRESS_WARNING_POP

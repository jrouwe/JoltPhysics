// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

inline void hash_combine(std::size_t &ioSeed) 
{ 
}

/// Hash combiner to use a custom struct in an unordered map or set
/// Taken from: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
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
template <typename T, typename... Rest>
inline void hash_combine(std::size_t &ioSeed, const T &inValue, Rest... inRest) 
{
	std::hash<T> hasher;
    ioSeed ^= hasher(inValue) + 0x9e3779b9 + (ioSeed << 6) + (ioSeed >> 2);
    hash_combine(ioSeed, inRest...);
}

} // JPH

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
    namespace std											\
	{														\
        template<>											\
		JPH_MAKE_HASH_STRUCT(type, hash<type>, __VA_ARGS__)	\
    }

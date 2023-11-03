// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

/// Simple binary input stream
class JPH_EXPORT StreamIn : public NonCopyable
{
public:
	/// Virtual destructor
	virtual				~StreamIn() = default;

	/// Read a string of bytes from the binary stream
	virtual void		ReadBytes(void *outData, size_t inNumBytes) = 0;

	/// Returns true when an attempt has been made to read past the end of the file
	virtual bool		IsEOF() const = 0;

	/// Returns true if there was an IO failure
	virtual bool		IsFailed() const = 0;

	/// Read a primitive (e.g. float, int, etc.) from the binary stream
	template <class T>
	void				Read(T &outT)
	{
		ReadBytes(&outT, sizeof(outT));
	}

	/// Read a vector of primitives from the binary stream
	template <class T, class A>
	void				Read(std::vector<T, A> &outT)
	{
		typename Array<T>::size_type len = outT.size(); // Initialize to previous array size, this is used for validation in the StateRecorder class
		Read(len);
		if (!IsEOF() && !IsFailed())
		{
			outT.resize(len);
			for (typename Array<T>::size_type i = 0; i < len; ++i)
				Read(outT[i]);
		}
		else
			outT.clear();
	}

	/// Read a string from the binary stream (reads the number of characters and then the characters)
	template <class Type, class Traits, class Allocator>
	void				Read(std::basic_string<Type, Traits, Allocator> &outString)
	{
		typename std::basic_string<Type, Traits, Allocator>::size_type len = 0;
		Read(len);
		if (!IsEOF() && !IsFailed())
		{
			outString.resize(len);
			ReadBytes(outString.data(), len * sizeof(Type));
		}
		else
			outString.clear();
	}

	/// Read a Vec3 (don't read W)
	void				Read(Vec3 &outVec)
	{
		ReadBytes(&outVec, 3 * sizeof(float));
		outVec = Vec3::sFixW(outVec.mValue);
	}

	/// Read a DVec3 (don't read W)
	void				Read(DVec3 &outVec)
	{
		ReadBytes(&outVec, 3 * sizeof(double));
		outVec = DVec3::sFixW(outVec.mValue);
	}

	/// Read a DMat44 (don't read W component of translation)
	void				Read(DMat44 &outVec)
	{
		Vec4 x, y, z;
		Read(x);
		Read(y);
		Read(z);

		DVec3 t;
		Read(t);

		outVec = DMat44(x, y, z, t);
	}
};

JPH_NAMESPACE_END

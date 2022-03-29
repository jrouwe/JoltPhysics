// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Simple binary input stream
class StreamIn
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
	void				Read(vector<T, A> &outT)
	{
		typename vector<T>::size_type len = outT.size(); // Initialize to previous array size, this is used for validation in the StateRecorder class
		Read(len);
		if (!IsEOF() && !IsFailed())
		{
			outT.resize(len);
			for (typename vector<T>::size_type i = 0; i < len; ++i)
				Read(outT[i]);
		}
		else
			outT.clear();
	}

	/// Read a string from the binary stream (reads the number of characters and then the characters)
	void				Read(string &outString)
	{
		string::size_type len = 0;
		Read(len);
		if (!IsEOF() && !IsFailed())
		{
			outString.resize(len);
			ReadBytes(outString.data(), len);
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
};

JPH_NAMESPACE_END

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/StateRecorderImpl.h>

namespace JPH {

void StateRecorderImpl::WriteBytes(const void *inData, size_t inNumBytes)
{
	mStream.write((const char *)inData, inNumBytes);
}

void StateRecorderImpl::Rewind()
{
	mStream.seekg(0, stringstream::beg);
}

void StateRecorderImpl::ReadBytes(void *outData, size_t inNumBytes)
{
	if (IsValidating())
	{
		// Read data in temporary buffer to compare with current value
		void *data = JPH_STACK_ALLOC(inNumBytes);
		mStream.read((char *)data, inNumBytes);
		if (memcmp(data, outData, inNumBytes) != 0)
		{
			// Mismatch, print error
			Trace("Mismatch reading %d bytes", inNumBytes);
			for (size_t i = 0; i < inNumBytes; ++i)
			{
				int b1 = reinterpret_cast<uint8 *>(outData)[i];
				int b2 = reinterpret_cast<uint8 *>(data)[i];
				if (b1 != b2)
					Trace("Offset %d: %02X -> %02X", i, b1, b2);
			}
			JPH_BREAKPOINT;
		}

		// Copy the temporary data to the final destination
		memcpy(outData, data, inNumBytes);
		return;
	}

	mStream.read((char *)outData, inNumBytes);
}

bool StateRecorderImpl::IsEqual(StateRecorderImpl &inReference)
{	
	// Get length of new state
	mStream.seekg(0, stringstream::end);
	size_t this_len = mStream.tellg();
	mStream.seekg(0, stringstream::beg);

	// Get length of old state
	inReference.mStream.seekg(0, stringstream::end);
	size_t reference_len = inReference.mStream.tellg();
	inReference.mStream.seekg(0, stringstream::beg);

	// Compare size
	bool fail = reference_len != this_len;
	if (fail)
	{
		Trace("Failed to properly recover state, different stream length!");
		return false;
	}

	// Compare byte by byte
	for (size_t i = 0, l = this_len; !fail && i < l; ++i)
	{
		fail = inReference.mStream.get() != mStream.get();
		if (fail)
		{
			Trace("Failed to properly recover state, different at offset %d!", i);
			return false;
		}
	}

	return true;
}

} // JPH
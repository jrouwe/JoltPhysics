// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_ENABLE_DETERMINISM_LOG

#include <Jolt/Physics/Body/BodyID.h>

#include <iomanip>
#include <fstream>

JPH_NAMESPACE_BEGIN

/// A simple class that logs the state of the simulation. The resulting text file can be used to diff between platforms and find issues in determinism.
class DeterminismLog
{
private:
	JPH_INLINE uint32		Convert(float inValue) const
	{
		return *(uint32 *)&inValue;
	}

public:
							DeterminismLog()
	{
		mLog.open("detlog.txt", ios::out | ios::trunc | ios::binary); // Binary because we don't want a difference between Unix and Windows line endings.
		mLog.fill('0');
	}

	DeterminismLog &		operator << (char inValue)
	{
		mLog << inValue;
		return *this;
	}

	DeterminismLog &		operator << (const char *inValue)
	{
		mLog << dec << inValue;
		return *this;
	}

	DeterminismLog &		operator << (const string &inValue)
	{
		mLog << dec << inValue;
		return *this;
	}

	DeterminismLog &		operator << (BodyID inValue)
	{
		mLog << hex << setw(8) << inValue.GetIndexAndSequenceNumber();
		return *this;
	}

	DeterminismLog &		operator << (float inValue)
	{
		mLog << hex << setw(8) << Convert(inValue);
		return *this;
	}

	DeterminismLog &		operator << (int inValue)
	{
		mLog << inValue;
		return *this;
	}

	DeterminismLog &		operator << (uint inValue)
	{
		mLog << inValue;
		return *this;
	}

	DeterminismLog &		operator << (Vec3Arg inValue)
	{
		mLog << hex << setw(8) << Convert(inValue.GetX()) << " " << setw(8) << Convert(inValue.GetY()) << " " << setw(8) << Convert(inValue.GetZ());
		return *this;
	}
	
	DeterminismLog &		operator << (QuatArg inValue)
	{
		mLog << hex << setw(8) << Convert(inValue.GetX()) << " " << setw(8) << Convert(inValue.GetY()) << " " << setw(8) << Convert(inValue.GetZ()) << " " << setw(8) << Convert(inValue.GetW());
		return *this;
	}

	// Singleton instance
	static DeterminismLog	sLog;

private:
	ofstream				mLog;
};

/// Will log something to the determinism log, usage: JPH_DET_LOG("label " << value);
#define JPH_DET_LOG(...)	DeterminismLog::sLog << __VA_ARGS__ << '\n'

JPH_NAMESPACE_END

#else

/// By default we log nothing
#define JPH_DET_LOG(...)

#endif // JPH_ENABLE_DETERMINISM_LOG

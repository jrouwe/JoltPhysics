// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/Mutex.h>
#include <Core/NonCopyable.h>
#include <map>

#ifdef JPH_STAT_COLLECTOR

namespace JPH {

/// Singleton class for collacting simple stat values
///
/// Usage:
///
/// To start recording call:
///
///		JPH_STAT_COLLECTOR_START_CAPTURE()
///
/// Then start the next frame:
///
///		JPH_STAT_COLLECTOR_SET_NEXT_FRAME()
///
/// To add a stat:
///
///		STAT_COLLETOR_ADD("Path.To.Name", <value>)
///
/// where value is an int, float, bool, Vec3 or Quat
///
/// To dump the stats to a file call:
///
///		JPH_STAT_COLLECTOR_STOP_CAPTURE()
class StatCollector : public NonCopyable
{
public:
	/// Reset all stats
	void						Reset();

	/// Start / stop capture
	void						StartCapture();
	void						StopCapture(const char *inFileName);
	bool						IsCapturing() const				{ return mIsCapturing; }

	/// Increments the frame counter
	void						SetNextFrame();

	/// Helper class that stores data points
	class Variant
	{
	public:
		enum class EType
		{
			Undefined,
			Float,
			Int,
			Bool,
		};

								Variant() : mType(EType::Undefined) { }
								Variant(float inFloat) : mType(EType::Float), mFloat(inFloat) { }
								Variant(int inInt) : mType(EType::Int), mInt(inInt) { }
								Variant(bool inBool) : mType(EType::Bool), mBool(inBool) { }

		string					ToString() const;

	private:
		EType					mType;

		union
		{
			float				mFloat;
			int					mInt;
			bool				mBool;
		};
	};
	
	/// Add an item
	void						AddItem(const string &inName, const Variant &inValue);
	void						AddItem(const string &inName, Vec3Arg inValue);
	void						AddItem(const string &inName, QuatArg inValue);

	/// Singleton instance
	static StatCollector		sInstance;
								
private:
	/// Internal variant of Reset that does not take the lock
	void						ResetInternal();

	using KeyValueMap = map<int, Variant>;
	using FrameMap = map<int, KeyValueMap>;
	using KeyIDMap = map<string, int>;

	Mutex						mMutex;
	bool						mIsCapturing = false;
	FrameMap					mFrames;
	KeyIDMap					mKeys;
	int							mNextKey = 0;
	int							mCurrentFrameNumber = 0;
	KeyValueMap *				mCurrentFrame = nullptr;
};							

#define JPH_IF_STAT_COLLECTOR(...)					__VA_ARGS__
#define JPH_STAT_COLLECTOR_SET_NEXT_FRAME()			StatCollector::sInstance.SetNextFrame()
#define JPH_STAT_COLLECTOR_ADD(name, value)			StatCollector::sInstance.AddItem(name, value)
#define JPH_STAT_COLLECTOR_START_CAPTURE()			StatCollector::sInstance.StartCapture()
#define JPH_STAT_COLLECTOR_STOP_CAPTURE(file_name)	StatCollector::sInstance.StopCapture(file_name)
#define JPH_STAT_COLLECTOR_IS_CAPTURING()			StatCollector::sInstance.IsCapturing()
#define JPH_STAT_COLLECTOR_RESET()					StatCollector::sInstance.Reset()

} // JPH

#else

#define JPH_IF_STAT_COLLECTOR(...)
#define JPH_STAT_COLLECTOR_SET_NEXT_FRAME()
#define JPH_STAT_COLLECTOR_ADD(name, value)
#define JPH_STAT_COLLECTOR_START_CAPTURE()		
#define JPH_STAT_COLLECTOR_STOP_CAPTURE(file_name)		
#define JPH_STAT_COLLECTOR_IS_CAPTURING()			false
#define JPH_STAT_COLLECTOR_RESET()

#endif

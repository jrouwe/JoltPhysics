// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/TickCounter.h>
#include <Physics/Collision/Shape/Shape.h>

// Shorthand function to ifdef out code if narrow phase stats tracking is off
#ifdef JPH_TRACK_NARROWPHASE_STATS
	#define JPH_IF_TRACK_NARROWPHASE_STATS(...) __VA_ARGS__
#else
	#define JPH_IF_TRACK_NARROWPHASE_STATS(...)
#endif // JPH_TRACK_NARROWPHASE_STATS

#ifdef JPH_TRACK_NARROWPHASE_STATS

namespace JPH {

/// Structure that tracks narrow phase timing information for a particular combination of shapes
class NarrowPhaseStat
{
public:
	/// Trace an individual stat in CSV form.
	void					ReportStats(const char *inName, EShapeSubType inType1, EShapeSubType inType2) const;

	/// Trace the collected broadphase stats in CSV form.
	/// This report can be used to judge and tweak the efficiency of the broadphase.
	static void				sReportStats();

	atomic<uint64>			mNumQueries = 0;
	atomic<uint64>			mHitsReported = 0;
	atomic<uint64>			mTotalTicks = 0;
	atomic<uint64>			mChildTicks = 0;
	atomic<uint64>			mCollectorTicks = 0;

	static NarrowPhaseStat	sCollideShape[NumSubShapeTypes][NumSubShapeTypes];
	static NarrowPhaseStat	sCastShape[NumSubShapeTypes][NumSubShapeTypes];
};

/// Object that tracks the start and end of a narrow phase operation
class TrackNarrowPhaseStat
{
public:
							TrackNarrowPhaseStat(NarrowPhaseStat &inStat) : 
		mStat(inStat),
		mParent(sRoot),
		mStart(GetProcessorTickCount())
	{
		// Make this the new root of the chain
		sRoot = this;
	}

							~TrackNarrowPhaseStat()
	{
		uint64 delta_ticks = GetProcessorTickCount() - mStart;

		// Notify parent of time spent in child
		if (mParent != nullptr)
			mParent->mStat.mChildTicks += delta_ticks;

		// Increment total ticks
		mStat.mNumQueries++;
		mStat.mTotalTicks += delta_ticks;

		// Restore root pointer
		JPH_ASSERT(sRoot == this);
		sRoot = mParent;
	}

	NarrowPhaseStat &		mStat;
	TrackNarrowPhaseStat *	mParent;
	uint64					mStart;

	static thread_local TrackNarrowPhaseStat *sRoot;
};

/// Object that tracks the start and end of a hit being processed by a collision collector
class TrackNarrowPhaseCollector
{
public:
							TrackNarrowPhaseCollector() : 
		mStart(GetProcessorTickCount())
	{
	}

							~TrackNarrowPhaseCollector()
	{
		uint64 delta_ticks = GetProcessorTickCount() - mStart;

		for (TrackNarrowPhaseStat *track = TrackNarrowPhaseStat::sRoot; track != nullptr; track = track->mParent)
		{
			NarrowPhaseStat &stat = track->mStat;

			stat.mHitsReported++;
			stat.mCollectorTicks += delta_ticks;
		}
	}

private:
	uint64					mStart;
};

} // JPH

#endif // JPH_TRACK_NARROWPHASE_STATS

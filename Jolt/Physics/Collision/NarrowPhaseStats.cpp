// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/NarrowPhaseStats.h>

#ifdef JPH_TRACK_NARROWPHASE_STATS

JPH_NAMESPACE_BEGIN

NarrowPhaseStat	NarrowPhaseStat::sCollideShape[NumSubShapeTypes][NumSubShapeTypes];
NarrowPhaseStat	NarrowPhaseStat::sCastShape[NumSubShapeTypes][NumSubShapeTypes];

thread_local TrackNarrowPhaseStat *TrackNarrowPhaseStat::sRoot = nullptr;

void NarrowPhaseStat::ReportStats(const char *inName, EShapeSubType inType1, EShapeSubType inType2) const
{
	uint64 ticks_per_sec = GetProcessorTicksPerSecond();

	double total_time = 1000.0 * double(mTotalTicks) / double(ticks_per_sec);
	double total_time_excl_children = 1000.0 * double(mTotalTicks - mChildTicks) / double(ticks_per_sec);

	stringstream str;
	str << inName << ", " << sSubShapeTypeNames[(int)inType1] << ", " << sSubShapeTypeNames[(int)inType2] << ", " << mNumQueries << ", " << total_time << ", " << total_time_excl_children << ", " << total_time_excl_children / mNumQueries << ", " << mHitsReported;
	Trace(str.str().c_str());
}

void NarrowPhaseStat::sReportStats()
{
	Trace("Query Type, Shape Type 1, Shape Type 2, Num Queries, Total Time (ms), Total Time Excl Children (ms), Total Time Excl. Children / Query (ms), Hits Reported");

	for (EShapeSubType t1 : sAllSubShapeTypes)
		for (EShapeSubType t2 : sAllSubShapeTypes)
		{
			const NarrowPhaseStat &stat = sCollideShape[(int)t1][(int)t2];
			if (stat.mNumQueries > 0)
				stat.ReportStats("CollideShape", t1, t2);
		}

	for (EShapeSubType t1 : sAllSubShapeTypes)
		for (EShapeSubType t2 : sAllSubShapeTypes)
		{
			const NarrowPhaseStat &stat = sCastShape[(int)t1][(int)t2];
			if (stat.mNumQueries > 0)
				stat.ReportStats("CastShape", t1, t2);
		}
}

JPH_NAMESPACE_END

#endif // JPH_TRACK_NARROWPHASE_STATS

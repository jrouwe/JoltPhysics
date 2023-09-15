// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/Core/QuickSort.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

#ifdef JPH_PROFILE_ENABLED

JPH_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////////////////////
// Profiler
//////////////////////////////////////////////////////////////////////////////////////////

Profiler *Profiler::sInstance = nullptr;

#ifdef JPH_SHARED_LIBRARY
	static thread_local ProfileThread *sInstance = nullptr;

	ProfileThread *ProfileThread::sGetInstance()
	{
		return sInstance;
	}

	void ProfileThread::sSetInstance(ProfileThread *inInstance)
	{
		sInstance = inInstance;
	}
#else
	thread_local ProfileThread *ProfileThread::sInstance = nullptr;
#endif

bool ProfileMeasurement::sOutOfSamplesReported = false;

void Profiler::UpdateReferenceTime()
{
	mReferenceTick = GetProcessorTickCount();
	mReferenceTime = std::chrono::high_resolution_clock::now();
}

uint64 Profiler::GetProcessorTicksPerSecond() const
{
	uint64 ticks = GetProcessorTickCount();
	std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();

	return (ticks - mReferenceTick) * 1000000000ULL / std::chrono::duration_cast<std::chrono::nanoseconds>(time - mReferenceTime).count();
}

void Profiler::NextFrame()
{
	std::lock_guard lock(mLock);

	if (mDump)
	{
		DumpInternal();
		mDump = false;
	}

	for (ProfileThread *t : mThreads)
		t->mCurrentSample = 0;

	UpdateReferenceTime();
}

void Profiler::Dump(const string_view &inTag)
{
	mDump = true;
	mDumpTag = inTag;
}

void Profiler::AddThread(ProfileThread *inThread)
{
	std::lock_guard lock(mLock);

	mThreads.push_back(inThread);
}

void Profiler::RemoveThread(ProfileThread *inThread)
{
	std::lock_guard lock(mLock);

	Array<ProfileThread *>::iterator i = find(mThreads.begin(), mThreads.end(), inThread);
	JPH_ASSERT(i != mThreads.end());
	mThreads.erase(i);
}

void Profiler::sAggregate(int inDepth, uint32 inColor, ProfileSample *&ioSample, const ProfileSample *inEnd, Aggregators &ioAggregators, KeyToAggregator &ioKeyToAggregator)
{
	// Store depth
	ioSample->mDepth = uint8(min(255, inDepth));

	// Update color
	if (ioSample->mColor == 0)
		ioSample->mColor = inColor;
	else
		inColor = ioSample->mColor;

	// Start accumulating totals
	uint64 cycles_this_with_children = ioSample->mEndCycle - ioSample->mStartCycle;
	uint64 cycles_in_children = 0;

	// Loop over following samples until we find a sample that starts on or after our end
	ProfileSample *sample;
	for (sample = ioSample + 1; sample < inEnd && sample->mStartCycle < ioSample->mEndCycle; ++sample)
	{
		JPH_ASSERT(sample[-1].mStartCycle <= sample->mStartCycle);
		JPH_ASSERT(sample->mStartCycle >= ioSample->mStartCycle);
		JPH_ASSERT(sample->mEndCycle <= ioSample->mEndCycle);

		// This is a direct child of us, accumulate time
		cycles_in_children += sample->mEndCycle - sample->mStartCycle;

		// Recurse and skip over the children of this child
		sAggregate(inDepth + 1, inColor, sample, inEnd, ioAggregators, ioKeyToAggregator);
	}

	// Find the aggregator for this name / filename pair
	Aggregator *aggregator;
	KeyToAggregator::iterator aggregator_idx = ioKeyToAggregator.find(ioSample->mName);
	if (aggregator_idx == ioKeyToAggregator.end())
	{
		// Not found, add to map and insert in array
		ioKeyToAggregator.try_emplace(ioSample->mName, ioAggregators.size());
		ioAggregators.emplace_back(ioSample->mName);
		aggregator = &ioAggregators.back();
	}
	else
	{
		// Found
		aggregator = &ioAggregators[aggregator_idx->second];
	}

	// Add the measurement to the aggregator
	aggregator->AccumulateMeasurement(cycles_this_with_children, cycles_in_children);

	// Update ioSample to the last child of ioSample
	JPH_ASSERT(sample[-1].mStartCycle <= ioSample->mEndCycle);
	JPH_ASSERT(sample >= inEnd || sample->mStartCycle >= ioSample->mEndCycle);
	ioSample = sample - 1;
}

void Profiler::DumpInternal()
{
	// Freeze data from threads
	// Note that this is not completely thread safe: As a profile sample is added mCurrentSample is incremented
	// but the data is not written until the sample finishes. So if we dump the profile information while
	// some other thread is running, we may get some garbage information from the previous frame
	Threads threads;
	for (ProfileThread *t : mThreads)
		threads.push_back({ t->mThreadName, t->mSamples, t->mSamples + t->mCurrentSample });

	// Shift all samples so that the first sample is at zero
	uint64 min_cycle = 0xffffffffffffffffUL;
	for (const ThreadSamples &t : threads)
		if (t.mSamplesBegin < t.mSamplesEnd)
			min_cycle = min(min_cycle, t.mSamplesBegin[0].mStartCycle);
	for (const ThreadSamples &t : threads)
		for (ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			s->mStartCycle -= min_cycle;
			s->mEndCycle -= min_cycle;
		}

	// Determine tag of this profile
	String tag;
	if (mDumpTag.empty())
	{
		// Next sequence number
		static int number = 0;
		++number;
		tag = ConvertToString(number);
	}
	else
	{
		// Take provided tag
		tag = mDumpTag;
		mDumpTag.clear();
	}

	// Aggregate data across threads
	Aggregators aggregators;
	KeyToAggregator key_to_aggregators;
	for (const ThreadSamples &t : threads)
		for (ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
			sAggregate(0, Color::sGetDistinctColor(0).GetUInt32(), s, end, aggregators, key_to_aggregators);

	// Dump as list
	DumpList(tag.c_str(), aggregators);

	// Dump as chart
	DumpChart(tag.c_str(), threads, key_to_aggregators, aggregators);
}

static String sHTMLEncode(const char *inString)
{
	String str(inString);
	StringReplace(str, "<", "&lt;");
	StringReplace(str, ">", "&gt;");
	return str;
}

void Profiler::DumpList(const char *inTag, const Aggregators &inAggregators)
{
	// Open file
	std::ofstream f;
	f.open(StringFormat("profile_list_%s.html", inTag).c_str(), std::ofstream::out | std::ofstream::trunc);
	if (!f.is_open())
		return;

	// Write header
	f << R"(<!DOCTYPE html>
<html>
	<head>
		<title>Profile List</title>
		<link rel="stylesheet" href="WebIncludes/semantic.min.css">
		<script type="text/javascript" src="WebIncludes/jquery-3.6.4.min.js"></script>
		<script type="text/javascript" src="WebIncludes/semantic.min.js"></script>
		<script type="text/javascript" src="WebIncludes/tablesort.js"></script>
		<script type="text/javascript">$(document).ready(function() { $('table').tablesort({ compare: function(a, b) { return isNaN(a) || isNaN(b)? a.localeCompare(b) : Number(a) - Number(b); } }); });</script>
	</head>
	<body class="minimal pushable">
		<table id="profile" class="ui sortable celled striped table">
			<thead>
				<tr>
					<th>Description</th>
					<th class="sorted descending">Total time with children (%)</th>
					<th>Total time (%)</th>
					<th>Calls</th>
					<th>&micro;s / call with children</th>
					<th>&micro;s / call</th>
					<th>Min. &micro;s / call</th>
					<th>Max. &micro;s / call</th>
				</tr>
			</thead>
			<tbody style="text-align: right;">
)";

	// Get total time
	uint64 total_time = 0;
	for (const Aggregator &item : inAggregators)
		total_time += item.mTotalCyclesInCallWithChildren - item.mTotalCyclesInChildren;

	// Get cycles per second
	uint64 cycles_per_second = GetProcessorTicksPerSecond();

	// Sort the list
	Aggregators aggregators = inAggregators;
	QuickSort(aggregators.begin(), aggregators.end());

	// Write all aggregators
	for (const Aggregator &item : aggregators)
	{
		uint64 cycles_in_call_no_children = item.mTotalCyclesInCallWithChildren - item.mTotalCyclesInChildren;

		char str[2048];
		snprintf(str, sizeof(str), R"(<tr>
	<td style="text-align: left;">%s</td>
	<td>%.1f</td>
	<td>%.1f</td>
	<td>%u</td>
	<td>%.2f</td>
	<td>%.2f</td>
	<td>%.2f</td>
	<td>%.2f</td>
</tr>)",
			sHTMLEncode(item.mName).c_str(),															// Description
			100.0 * item.mTotalCyclesInCallWithChildren / total_time,									// Total time with children
			100.0 * cycles_in_call_no_children / total_time,											// Total time no children
			item.mCallCounter,																			// Calls
			1000000.0 * item.mTotalCyclesInCallWithChildren / cycles_per_second / item.mCallCounter,	// us / call with children
			1000000.0 * cycles_in_call_no_children / cycles_per_second / item.mCallCounter,				// us / call no children
			1000000.0 * item.mMinCyclesInCallWithChildren / cycles_per_second,							// Min. us / call with children
			1000000.0 * item.mMaxCyclesInCallWithChildren / cycles_per_second);							// Max. us / call with children

		f << str;
	}

	// End table
	f << R"(</tbody></table></body></html>)";
}

void Profiler::DumpChart(const char *inTag, const Threads &inThreads, const KeyToAggregator &inKeyToAggregators, const Aggregators &inAggregators)
{
	// Open file
	std::ofstream f;
	f.open(StringFormat("profile_chart_%s.html", inTag).c_str(), std::ofstream::out | std::ofstream::trunc);
	if (!f.is_open())
		return;

	// Write header
	f << R"(<!DOCTYPE html>
<html>
	<head>
		<title>Profile Chart</title>
		<link rel="stylesheet" href="WebIncludes/profile_chart.css">
		<script type="text/javascript" src="WebIncludes/profile_chart.js"></script>
	</head>
	<body onload="startChart();">
	<script type="text/javascript">
)";

	// Get cycles per second
	uint64 cycles_per_second = GetProcessorTicksPerSecond();
	f << "var cycles_per_second = " << cycles_per_second << ";\n";

	// Dump samples
	f << "var threads = [\n";
	bool first_thread = true;
	for (const ThreadSamples &t : inThreads)
	{
		if (!first_thread)
			f << ",\n";
		first_thread = false;

		f << "{\nthread_name: \"" << t.mThreadName << "\",\naggregator: [";
		bool first = true;
		for (const ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			if (!first)
				f << ",";
			first = false;
			f << inKeyToAggregators.find(s->mName)->second;
		}
		f << "],\ncolor: [";
		first = true;
		for (const ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			if (!first)
				f << ",";
			first = false;
			Color c(s->mColor);
			f << StringFormat("\"#%02x%02x%02x\"", c.r, c.g, c.b);
		}
		f << "],\nstart: [";
		first = true;
		for (const ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			if (!first)
				f << ",";
			first = false;
			f << s->mStartCycle;
		}
		f << "],\ncycles: [";
		first = true;
		for (const ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			if (!first)
				f << ",";
			first = false;
			f << s->mEndCycle - s->mStartCycle;
		}
		f << "],\ndepth: [";
		first = true;
		for (const ProfileSample *s = t.mSamplesBegin, *end = t.mSamplesEnd; s < end; ++s)
		{
			if (!first)
				f << ",";
			first = false;
			f << int(s->mDepth);
		}
		f << "]\n}";
	}

	// Dump aggregated data
	f << "];\nvar aggregated = {\nname: [";
	bool first = true;
	for (const Aggregator &a : inAggregators)
	{
		if (!first)
			f << ",";
		first = false;
		String name = "\"" + sHTMLEncode(a.mName) + "\"";
		f << name;
	}
	f << "],\ncalls: [";
	first = true;
	for (const Aggregator &a : inAggregators)
	{
		if (!first)
			f << ",";
		first = false;
		f << a.mCallCounter;
	}
	f << "],\nmin_cycles: [";
	first = true;
	for (const Aggregator &a : inAggregators)
	{
		if (!first)
			f << ",";
		first = false;
		f << a.mMinCyclesInCallWithChildren;
	}
	f << "],\nmax_cycles: [";
	first = true;
	for (const Aggregator &a : inAggregators)
	{
		if (!first)
			f << ",";
		first = false;
		f << a.mMaxCyclesInCallWithChildren;
	}
	f << "],\ncycles_per_frame: [";
	first = true;
	for (const Aggregator &a : inAggregators)
	{
		if (!first)
			f << ",";
		first = false;
		f << a.mTotalCyclesInCallWithChildren;
	}

	// Write footer
	f << R"(]};
</script>

<canvas id="canvas"></canvas>
<div id="tooltip"></div>

</tbody></table></body></html>)";
}

JPH_NAMESPACE_END

#endif // JPH_PROFILE_ENABLED

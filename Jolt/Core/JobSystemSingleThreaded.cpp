// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/JobSystemSingleThreaded.h>

JPH_NAMESPACE_BEGIN

JobHandle JobSystemSingleThreaded::CreateJob(const char *inJobName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies)
{
	JobHandle job(new Job(inJobName, inColor, this, inJobFunction, inNumDependencies));

	if (inNumDependencies == 0)
		QueueJob(job.GetPtr());

	return job;
}

void JobSystemSingleThreaded::FreeJob(Job *inJob)
{
	delete inJob;
}

void JobSystemSingleThreaded::QueueJob(Job *inJob)
{
	inJob->Execute();
}

void JobSystemSingleThreaded::QueueJobs(Job **inJobs, uint inNumJobs)
{
	for (uint i = 0; i < inNumJobs; ++i)
		QueueJob(inJobs[i]);
}

JobSystem::Barrier *JobSystemSingleThreaded::CreateBarrier()
{
	return new BarrierImpl;
}

void JobSystemSingleThreaded::DestroyBarrier(Barrier *inBarrier)
{
	delete static_cast<BarrierImpl *>(inBarrier);
}

void JobSystemSingleThreaded::WaitForJobs(Barrier *inBarrier)
{
}

JPH_NAMESPACE_END

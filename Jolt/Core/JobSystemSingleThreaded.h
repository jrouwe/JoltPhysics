// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/JobSystem.h>

JPH_NAMESPACE_BEGIN

/// Implementation of a JobSystem without threads, runs jobs as soon as they are added
class JPH_EXPORT JobSystemSingleThreaded final : public JobSystem
{
public:
	JPH_OVERRIDE_NEW_DELETE

	// See JobSystem
	virtual int				GetMaxConcurrency() const override				{ return 1; }
	virtual JobHandle		CreateJob(const char *inName, ColorArg inColor, const JobFunction &inJobFunction, uint32 inNumDependencies = 0) override;
	virtual Barrier *		CreateBarrier() override;
	virtual void			DestroyBarrier(Barrier *inBarrier) override;
	virtual void			WaitForJobs(Barrier *inBarrier) override;

protected:
	// Dummy implementation of Barrier, all jobs are executed immediately
	class BarrierImpl : public Barrier
	{
	public:
		JPH_OVERRIDE_NEW_DELETE

		// See Barrier
		virtual void		AddJob(const JobHandle &inJob) override			{ }
		virtual void		AddJobs(const JobHandle *inHandles, uint inNumHandles) override { }

	protected:
		/// Called by a Job to mark that it is finished
		virtual void		OnJobFinished(Job *inJob)						{ }
	};

	// See JobSystem
	virtual void			QueueJob(Job *inJob) override;
	virtual void			QueueJobs(Job **inJobs, uint inNumJobs) override;
	virtual void			FreeJob(Job *inJob) override;
};

JPH_NAMESPACE_END

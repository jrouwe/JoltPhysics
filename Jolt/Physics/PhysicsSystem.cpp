// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsUpdateContext.h>
#include <Jolt/Physics/PhysicsStepListener.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseBruteForce.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/AABoxCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideConvexVsTriangles.h>
#include <Jolt/Physics/Collision/ManifoldBetweenTwoFaces.h>
#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AxisConstraintPart.h>
#include <Jolt/Geometry/RayAABox.h>
#include <Jolt/Core/JobSystem.h>
#include <Jolt/Core/TempAllocator.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_DEBUG_RENDERER
bool PhysicsSystem::sDrawMotionQualityLinearCast = false;
#endif // JPH_DEBUG_RENDERER

//#define BROAD_PHASE BroadPhaseBruteForce
#define BROAD_PHASE BroadPhaseQuadTree

static const Color cColorUpdateBroadPhaseFinalize = Color::sGetDistinctColor(1);
static const Color cColorUpdateBroadPhasePrepare = Color::sGetDistinctColor(2);
static const Color cColorFindCollisions = Color::sGetDistinctColor(3);
static const Color cColorApplyGravity = Color::sGetDistinctColor(4);
static const Color cColorSetupVelocityConstraints = Color::sGetDistinctColor(5);
static const Color cColorBuildIslandsFromConstraints = Color::sGetDistinctColor(6);
static const Color cColorDetermineActiveConstraints = Color::sGetDistinctColor(7);
static const Color cColorFinalizeIslands = Color::sGetDistinctColor(8);
static const Color cColorContactRemovedCallbacks = Color::sGetDistinctColor(9);
static const Color cColorBodySetIslandIndex = Color::sGetDistinctColor(10);
static const Color cColorStartNextStep = Color::sGetDistinctColor(11);
static const Color cColorSolveVelocityConstraints = Color::sGetDistinctColor(12);
static const Color cColorPreIntegrateVelocity = Color::sGetDistinctColor(13);
static const Color cColorIntegrateVelocity = Color::sGetDistinctColor(14);
static const Color cColorPostIntegrateVelocity = Color::sGetDistinctColor(15);
static const Color cColorResolveCCDContacts = Color::sGetDistinctColor(16);
static const Color cColorSolvePositionConstraints = Color::sGetDistinctColor(17);
static const Color cColorStartNextSubStep = Color::sGetDistinctColor(18);
static const Color cColorFindCCDContacts = Color::sGetDistinctColor(19);
static const Color cColorStepListeners = Color::sGetDistinctColor(20);

PhysicsSystem::~PhysicsSystem()
{
	// Remove broadphase
	delete mBroadPhase;
}

void PhysicsSystem::Init(uint inMaxBodies, uint inNumBodyMutexes, uint inMaxBodyPairs, uint inMaxContactConstraints, const BroadPhaseLayerInterface &inBroadPhaseLayerInterface, ObjectVsBroadPhaseLayerFilter inObjectVsBroadPhaseLayerFilter, ObjectLayerPairFilter inObjectLayerPairFilter)
{ 
	mObjectVsBroadPhaseLayerFilter = inObjectVsBroadPhaseLayerFilter;
	mObjectLayerPairFilter = inObjectLayerPairFilter;

	// Initialize body manager
	mBodyManager.Init(inMaxBodies, inNumBodyMutexes, inBroadPhaseLayerInterface); 

	// Create broadphase
	mBroadPhase = new BROAD_PHASE();
	mBroadPhase->Init(&mBodyManager, inBroadPhaseLayerInterface);

	// Init contact constraint manager
	mContactManager.Init(inMaxBodyPairs, inMaxContactConstraints);

	// Init islands builder
	mIslandBuilder.Init(inMaxBodies);

	// Initialize body interface
	mBodyInterfaceLocking.Init(mBodyLockInterfaceLocking, mBodyManager, *mBroadPhase);
	mBodyInterfaceNoLock.Init(mBodyLockInterfaceNoLock, mBodyManager, *mBroadPhase);

	// Initialize narrow phase query
	mNarrowPhaseQueryLocking.Init(mBodyLockInterfaceLocking, *mBroadPhase);
	mNarrowPhaseQueryNoLock.Init(mBodyLockInterfaceNoLock, *mBroadPhase);
}

void PhysicsSystem::OptimizeBroadPhase()
{
	mBroadPhase->Optimize();
}

void PhysicsSystem::AddStepListener(PhysicsStepListener *inListener)
{
	lock_guard lock(mStepListenersMutex);

	JPH_ASSERT(find(mStepListeners.begin(), mStepListeners.end(), inListener) == mStepListeners.end());
	mStepListeners.push_back(inListener);
}

void PhysicsSystem::RemoveStepListener(PhysicsStepListener *inListener)
{
	lock_guard lock(mStepListenersMutex);

	StepListeners::iterator i = find(mStepListeners.begin(), mStepListeners.end(), inListener);
	JPH_ASSERT(i != mStepListeners.end());
	mStepListeners.erase(i);
}

void PhysicsSystem::Update(float inDeltaTime, int inCollisionSteps, int inIntegrationSubSteps, TempAllocator *inTempAllocator, JobSystem *inJobSystem)
{	
	JPH_PROFILE_FUNCTION();

	JPH_ASSERT(inDeltaTime >= 0.0f);
	JPH_ASSERT(inIntegrationSubSteps <= PhysicsUpdateContext::cMaxSubSteps);

	// Sync point for the broadphase. This will allow it to do clean up operations without having any mutexes locked yet.
	mBroadPhase->FrameSync();

	// If there are no active bodies or there's no time delta
	uint32 num_active_bodies = mBodyManager.GetNumActiveBodies();
	if (num_active_bodies == 0 || inDeltaTime <= 0.0f)
	{
		mBodyManager.LockAllBodies();

		// Update broadphase
		mBroadPhase->LockModifications();
		BroadPhase::UpdateState update_state = mBroadPhase->UpdatePrepare();
		mBroadPhase->UpdateFinalize(update_state);
		mBroadPhase->UnlockModifications();

		// Call contact removal callbacks from contacts that existed in the previous update
		mContactManager.ContactPointRemovedCallbacks();
		mContactManager.FinalizeContactCache(0, 0);

		mBodyManager.UnlockAllBodies();
		return;
	}

	// Calculate ratio between current and previous frame delta time to scale initial constraint forces
	float sub_step_delta_time = inDeltaTime / (inCollisionSteps * inIntegrationSubSteps);
	float warm_start_impulse_ratio = mPhysicsSettings.mConstraintWarmStart && mPreviousSubStepDeltaTime > 0.0f? sub_step_delta_time / mPreviousSubStepDeltaTime : 0.0f;
	mPreviousSubStepDeltaTime = sub_step_delta_time;

	// Create the context used for passing information between jobs
	PhysicsUpdateContext context;
	context.mPhysicsSystem = this;
	context.mTempAllocator = inTempAllocator;
	context.mJobSystem = inJobSystem;
	context.mBarrier = inJobSystem->CreateBarrier();
	context.mIslandBuilder = &mIslandBuilder;
	context.mStepDeltaTime = inDeltaTime / inCollisionSteps;
	context.mSubStepDeltaTime = sub_step_delta_time;
	context.mWarmStartImpulseRatio = warm_start_impulse_ratio;

	// Allocate space for body pairs
	JPH_ASSERT(context.mBodyPairs == nullptr);
	context.mBodyPairs = static_cast<BodyPair *>(inTempAllocator->Allocate(sizeof(BodyPair) * mPhysicsSettings.mMaxInFlightBodyPairs));

	// Lock all bodies for write so that we can freely touch them
	mStepListenersMutex.lock();
	mBodyManager.LockAllBodies();
	mBroadPhase->LockModifications();

	// Get max number of concurrent jobs
	int max_concurrency = context.GetMaxConcurrency(); 

	// Calculate how many step listener jobs we spawn
	int num_step_listener_jobs = mStepListeners.empty()? 0 : max(1, min((int)mStepListeners.size() / mPhysicsSettings.mStepListenersBatchSize / mPhysicsSettings.mStepListenerBatchesPerJob, max_concurrency));

	// Number of gravity jobs depends on the amount of active bodies.
	// Launch max 1 job per batch of active bodies
	// Leave 1 thread for update broadphase prepare and 1 for determine active constraints
	int num_apply_gravity_jobs = max(1, min(((int)num_active_bodies + cApplyGravityBatchSize - 1) / cApplyGravityBatchSize, max_concurrency - 2));

	// Number of determine active constraints jobs to run depends on number of constraints.
	// Leave 1 thread for update broadphase prepare and 1 for apply gravity
	int num_determine_active_constraints_jobs = max(1, min(((int)mConstraintManager.GetNumConstraints() + cDetermineActiveConstraintsBatchSize - 1) / cDetermineActiveConstraintsBatchSize, max_concurrency - 2));

	// Number of find collisions jobs to run depends on number of active bodies.
	int num_find_collisions_jobs = max(1, min(((int)num_active_bodies + cActiveBodiesBatchSize - 1) / cActiveBodiesBatchSize, max_concurrency));

	// Number of integrate velocity jobs depends on number of active bodies.
	int num_integrate_velocity_jobs = max(1, min(((int)num_active_bodies + cIntegrateVelocityBatchSize - 1) / cIntegrateVelocityBatchSize, max_concurrency));

	{
		JPH_PROFILE("Build Jobs");

		// Iterate over collision steps
		context.mSteps.resize(inCollisionSteps);
		for (int step_idx = 0; step_idx < inCollisionSteps; ++step_idx)
		{
			bool is_first_step = step_idx == 0;
			bool is_last_step = step_idx == inCollisionSteps - 1;

			PhysicsUpdateContext::Step &step = context.mSteps[step_idx];
			step.mContext = &context;
			step.mSubSteps.resize(inIntegrationSubSteps);

			// Create job to do broadphase finalization
			// This job must finish before integrating velocities. Until then the positions will not be updated neither will bodies be added / removed.
			step.mUpdateBroadphaseFinalize = inJobSystem->CreateJob("UpdateBroadPhaseFinalize", cColorUpdateBroadPhaseFinalize, [&context, &step]() 
				{ 
					// Validate that all find collision jobs have stopped
					JPH_ASSERT(step.mActiveFindCollisionJobs == 0);

					// Finalize the broadphase update
					context.mPhysicsSystem->mBroadPhase->UpdateFinalize(step.mBroadPhaseUpdateState);

					// Signal that it is done
					step.mSubSteps[0].mPreIntegrateVelocity.RemoveDependency(); 
				}, num_find_collisions_jobs + 2); // depends on: find collisions, broadphase prepare update, finish building jobs

			// The immediate jobs below are only immediate for the first step, the all finished job will kick them for the next step
			int previous_step_dependency_count = is_first_step? 0 : 1;

			// Start job immediately: Start the prepare broadphase
			// Must be done under body lock protection since the order is body locks then broadphase mutex
			// If this is turned around the RemoveBody call will hang since it locks in that order
			step.mBroadPhasePrepare = inJobSystem->CreateJob("UpdateBroadPhasePrepare", cColorUpdateBroadPhasePrepare, [&context, &step]() 
				{ 
					// Prepare the broadphase update
					step.mBroadPhaseUpdateState = context.mPhysicsSystem->mBroadPhase->UpdatePrepare();

					// Now the finalize can run (if other dependencies are met too)
					step.mUpdateBroadphaseFinalize.RemoveDependency();
				}, previous_step_dependency_count);

			// This job will find all collisions
			step.mBodyPairQueues.resize(max_concurrency);
			step.mMaxBodyPairsPerQueue = mPhysicsSettings.mMaxInFlightBodyPairs / max_concurrency;
			step.mActiveFindCollisionJobs = ~PhysicsUpdateContext::JobMask(0) >> (sizeof(PhysicsUpdateContext::JobMask) * 8 - num_find_collisions_jobs);
			step.mFindCollisions.resize(num_find_collisions_jobs);
			for (int i = 0; i < num_find_collisions_jobs; ++i)
			{
				step.mFindCollisions[i] = inJobSystem->CreateJob("FindCollisions", cColorFindCollisions, [&step, i]() 
					{ 
						step.mContext->mPhysicsSystem->JobFindCollisions(&step, i); 
					}, num_apply_gravity_jobs + num_determine_active_constraints_jobs + 1); // depends on: apply gravity, determine active constraints, finish building jobs
			}

			if (is_first_step)
			{
			#ifdef JPH_ENABLE_ASSERTS
				// Don't allow write operations to the active bodies list
				mBodyManager.SetActiveBodiesLocked(true);
			#endif

				// Store the number of active bodies at the start of the step
				step.mNumActiveBodiesAtStepStart = mBodyManager.GetNumActiveBodies();

				// Lock all constraints
				mConstraintManager.LockAllConstraints();

				// Allocate memory for storing the active constraints
				JPH_ASSERT(context.mActiveConstraints == nullptr);
				context.mActiveConstraints = static_cast<Constraint **>(inTempAllocator->Allocate(mConstraintManager.GetNumConstraints() * sizeof(Constraint *)));

				// Prepare contact buffer
				mContactManager.PrepareConstraintBuffer(&context);

				// Setup island builder
				mIslandBuilder.PrepareContactConstraints(mContactManager.GetMaxConstraints(), context.mTempAllocator);
			}

			// This job applies gravity to all active bodies
			step.mApplyGravity.resize(num_apply_gravity_jobs);
			for (int i = 0; i < num_apply_gravity_jobs; ++i)
				step.mApplyGravity[i] = inJobSystem->CreateJob("ApplyGravity", cColorApplyGravity, [&context, &step]() 
					{ 
						context.mPhysicsSystem->JobApplyGravity(&context, &step); 

						JobHandle::sRemoveDependencies(step.mFindCollisions);
					}, num_step_listener_jobs > 0? num_step_listener_jobs : previous_step_dependency_count); // depends on: step listeners (or previous step if no step listeners)
	
			// This job will setup velocity constraints for non-collision constraints
			step.mSetupVelocityConstraints = inJobSystem->CreateJob("SetupVelocityConstraints", cColorSetupVelocityConstraints, [&context, &step]() 
				{ 
					context.mPhysicsSystem->JobSetupVelocityConstraints(context.mSubStepDeltaTime, &step);

					JobHandle::sRemoveDependencies(step.mSubSteps[0].mSolveVelocityConstraints);
				}, num_determine_active_constraints_jobs + 1); // depends on: determine active constraints, finish building jobs

			// This job will build islands from constraints
			step.mBuildIslandsFromConstraints = inJobSystem->CreateJob("BuildIslandsFromConstraints", cColorBuildIslandsFromConstraints, [&context, &step]() 
				{ 
					context.mPhysicsSystem->JobBuildIslandsFromConstraints(&context, &step);

					step.mFinalizeIslands.RemoveDependency(); 
				}, num_determine_active_constraints_jobs + 1); // depends on: determine active constraints, finish building jobs

			// This job determines active constraints
			step.mDetermineActiveConstraints.resize(num_determine_active_constraints_jobs);
			for (int i = 0; i < num_determine_active_constraints_jobs; ++i)
				step.mDetermineActiveConstraints[i] = inJobSystem->CreateJob("DetermineActiveConstraints", cColorDetermineActiveConstraints, [&context, &step]() 
					{ 
						context.mPhysicsSystem->JobDetermineActiveConstraints(&step); 

						step.mSetupVelocityConstraints.RemoveDependency();
						step.mBuildIslandsFromConstraints.RemoveDependency();

						// Kick find collisions last as they will use up all CPU cores leaving no space for the previous 2 jobs
						JobHandle::sRemoveDependencies(step.mFindCollisions);
					}, num_step_listener_jobs > 0? num_step_listener_jobs : previous_step_dependency_count); // depends on: step listeners (or previous step if no step listeners)

			// This job calls the step listeners
			step.mStepListeners.resize(num_step_listener_jobs);
			for (int i = 0; i < num_step_listener_jobs; ++i)
				step.mStepListeners[i] = inJobSystem->CreateJob("StepListeners", cColorStepListeners, [&context, &step]()
					{
						// Call the step listeners
						context.mPhysicsSystem->JobStepListeners(&step);

						// Kick apply gravity and determine active constraint jobs
						JobHandle::sRemoveDependencies(step.mApplyGravity);
						JobHandle::sRemoveDependencies(step.mDetermineActiveConstraints);
					}, previous_step_dependency_count);

			// Unblock the previous step
			if (!is_first_step)
				context.mSteps[step_idx - 1].mStartNextStep.RemoveDependency();

			// This job will finalize the simulation islands
			step.mFinalizeIslands = inJobSystem->CreateJob("FinalizeIslands", cColorFinalizeIslands, [&context, &step]() 
				{ 
					// Validate that all find collision jobs have stopped
					JPH_ASSERT(step.mActiveFindCollisionJobs == 0);

					context.mPhysicsSystem->JobFinalizeIslands(&context); 

					JobHandle::sRemoveDependencies(step.mSubSteps[0].mSolveVelocityConstraints);
					step.mBodySetIslandIndex.RemoveDependency();
				}, num_find_collisions_jobs + 2); // depends on: find collisions, build islands from constraints, finish building jobs

			// Unblock previous job
			// Note: technically we could release find collisions here but we don't want to because that could make them run before 'setup velocity constraints' which means that job won't have a thread left
			step.mBuildIslandsFromConstraints.RemoveDependency();

			// This job will call the contact removed callbacks
			step.mContactRemovedCallbacks = inJobSystem->CreateJob("ContactRemovedCallbacks", cColorContactRemovedCallbacks, [&context, &step]()
				{
					context.mPhysicsSystem->JobContactRemovedCallbacks(&step);

					if (step.mStartNextStep.IsValid())
						step.mStartNextStep.RemoveDependency();
				}, 1); // depends on the find ccd contacts of the last sub step

			// This job will set the island index on each body (only used for debug drawing purposes)
			// It will also delete any bodies that have been destroyed in the last frame
			step.mBodySetIslandIndex = inJobSystem->CreateJob("BodySetIslandIndex", cColorBodySetIslandIndex, [&context, &step]() 
				{ 
					context.mPhysicsSystem->JobBodySetIslandIndex(); 

					if (step.mStartNextStep.IsValid())
						step.mStartNextStep.RemoveDependency();
				}, 1); // depends on: finalize islands

			// Job to start the next collision step
			if (!is_last_step)
			{
				PhysicsUpdateContext::Step *next_step = &context.mSteps[step_idx + 1];
				step.mStartNextStep = inJobSystem->CreateJob("StartNextStep", cColorStartNextStep, [this, next_step]() 
					{ 
					#ifdef _DEBUG
						// Validate that the cached bounds are correct
						mBodyManager.ValidateActiveBodyBounds();
					#endif // _DEBUG

						// Store the number of active bodies at the start of the step
						next_step->mNumActiveBodiesAtStepStart = mBodyManager.GetNumActiveBodies();

						// Clear the island builder
						TempAllocator *temp_allocator = next_step->mContext->mTempAllocator;
						mIslandBuilder.ResetIslands(temp_allocator);

						// Setup island builder
						mIslandBuilder.PrepareContactConstraints(mContactManager.GetMaxConstraints(), temp_allocator);
						
						// Restart the contact manager
						mContactManager.RecycleConstraintBuffer();

						// Kick the jobs of the next step (in the same order as the first step)
						next_step->mBroadPhasePrepare.RemoveDependency();
						if (next_step->mStepListeners.empty())
						{
							// Kick the gravity and active constraints jobs immediately
							JobHandle::sRemoveDependencies(next_step->mApplyGravity);
							JobHandle::sRemoveDependencies(next_step->mDetermineActiveConstraints);
						}
						else
						{
							// Kick the step listeners job first
							JobHandle::sRemoveDependencies(next_step->mStepListeners);
						}
					}, max_concurrency + 3); // depends on: solve position constraints of the last step, body set island index, contact removed callbacks, finish building the previous step
			}

			// Create solve jobs for each of the integration sub steps
			for (int sub_step_idx = 0; sub_step_idx < inIntegrationSubSteps; ++sub_step_idx)
			{
				bool is_first_sub_step = sub_step_idx == 0;
				bool is_last_sub_step = sub_step_idx == inIntegrationSubSteps - 1;

				PhysicsUpdateContext::SubStep &sub_step = step.mSubSteps[sub_step_idx];
				sub_step.mStep = &step;
				sub_step.mIsFirst = is_first_sub_step;
				sub_step.mIsLast = is_last_sub_step;
				sub_step.mIsLastOfAll = is_last_step && is_last_sub_step;

				// This job will solve the velocity constraints 
				int num_dependencies_solve_velocity_constraints = is_first_sub_step? 3 : 2; // in first sub step depends on: finalize islands, setup velocity constraints, in later sub steps depends on: previous sub step finished. For both: finish building jobs.
				sub_step.mSolveVelocityConstraints.resize(max_concurrency);
				for (int i = 0; i < max_concurrency; ++i)
					sub_step.mSolveVelocityConstraints[i] = inJobSystem->CreateJob("SolveVelocityConstraints", cColorSolveVelocityConstraints, [&context, &sub_step]() 
						{ 
							context.mPhysicsSystem->JobSolveVelocityConstraints(&context, &sub_step); 

							sub_step.mPreIntegrateVelocity.RemoveDependency();
						}, num_dependencies_solve_velocity_constraints); 

				// Unblock previous jobs
				if (is_first_sub_step)
				{
					// Kick find collisions after setup velocity constraints because the former job will use up all CPU cores
					step.mSetupVelocityConstraints.RemoveDependency();
					JobHandle::sRemoveDependencies(step.mFindCollisions);

					// Finalize islands is a dependency on find collisions so it can go last
					step.mFinalizeIslands.RemoveDependency();
				}
				else
				{
					step.mSubSteps[sub_step_idx - 1].mStartNextSubStep.RemoveDependency();
				}

				// This job will prepare the position update of all active bodies
				int num_dependencies_integrate_velocity = is_first_sub_step? 2 + max_concurrency : 1 + max_concurrency;  // depends on: broadphase update finalize in first step, solve velocity constraints in all steps. For both: finish building jobs.
				sub_step.mPreIntegrateVelocity = inJobSystem->CreateJob("PreIntegrateVelocity", cColorPreIntegrateVelocity, [&context, &sub_step]() 
					{ 
						context.mPhysicsSystem->JobPreIntegrateVelocity(&context, &sub_step);

						JobHandle::sRemoveDependencies(sub_step.mIntegrateVelocity);
					}, num_dependencies_integrate_velocity);

				// Unblock previous jobs
				if (is_first_sub_step)
					step.mUpdateBroadphaseFinalize.RemoveDependency();
				JobHandle::sRemoveDependencies(sub_step.mSolveVelocityConstraints);

				// This job will update the positions of all active bodies
				sub_step.mIntegrateVelocity.resize(num_integrate_velocity_jobs);
				for (int i = 0; i < num_integrate_velocity_jobs; ++i)
					sub_step.mIntegrateVelocity[i] = inJobSystem->CreateJob("IntegrateVelocity", cColorIntegrateVelocity, [&context, &sub_step]() 
						{ 
							context.mPhysicsSystem->JobIntegrateVelocity(&context, &sub_step);

							sub_step.mPostIntegrateVelocity.RemoveDependency();
						}, 2); // depends on: pre integrate velocity, finish building jobs.

				// Unblock previous job
				sub_step.mPreIntegrateVelocity.RemoveDependency();

				// This job will finish the position update of all active bodies
				sub_step.mPostIntegrateVelocity = inJobSystem->CreateJob("PostIntegrateVelocity", cColorPostIntegrateVelocity, [&context, &sub_step]() 
					{ 
						context.mPhysicsSystem->JobPostIntegrateVelocity(&context, &sub_step);

						sub_step.mResolveCCDContacts.RemoveDependency();
					}, num_integrate_velocity_jobs + 1); // depends on: integrate velocity, finish building jobs

				// Unblock previous jobs
				JobHandle::sRemoveDependencies(sub_step.mIntegrateVelocity);

				// This job will update the positions and velocities for all bodies that need continuous collision detection
				sub_step.mResolveCCDContacts = inJobSystem->CreateJob("ResolveCCDContacts", cColorResolveCCDContacts, [&context, &sub_step]()
					{
						context.mPhysicsSystem->JobResolveCCDContacts(&context, &sub_step);

						JobHandle::sRemoveDependencies(sub_step.mSolvePositionConstraints);
					}, 2); // depends on: integrate velocities, detect ccd contacts (added dynamically), finish building jobs.

				// Unblock previous job
				sub_step.mPostIntegrateVelocity.RemoveDependency();

				// Fixes up drift in positions and updates the broadphase with new body positions
				sub_step.mSolvePositionConstraints.resize(max_concurrency);
				for (int i = 0; i < max_concurrency; ++i)
					sub_step.mSolvePositionConstraints[i] = inJobSystem->CreateJob("SolvePositionConstraints", cColorSolvePositionConstraints, [&context, &sub_step]() 
						{ 
							context.mPhysicsSystem->JobSolvePositionConstraints(&context, &sub_step); 
			
							// Kick the next sub step
							if (sub_step.mStartNextSubStep.IsValid())
								sub_step.mStartNextSubStep.RemoveDependency();
						}, 2); // depends on: resolve ccd contacts, finish building jobs.

				// Unblock previous job.
				sub_step.mResolveCCDContacts.RemoveDependency();

				// This job starts the next sub step
				if (!is_last_sub_step)
				{
					PhysicsUpdateContext::SubStep &next_sub_step = step.mSubSteps[sub_step_idx + 1];
					sub_step.mStartNextSubStep = inJobSystem->CreateJob("StartNextSubStep", cColorStartNextSubStep, [&next_sub_step]() 
						{ 			
							// Kick velocity constraint solving for the next sub step
							JobHandle::sRemoveDependencies(next_sub_step.mSolveVelocityConstraints);
						}, max_concurrency + 1); // depends on: solve position constraints, finish building jobs.
				}
				else
					sub_step.mStartNextSubStep = step.mStartNextStep;

				// Unblock previous jobs
				JobHandle::sRemoveDependencies(sub_step.mSolvePositionConstraints);
			}
		}
	}
	
	// Build the list of jobs to wait for
	JobSystem::Barrier *barrier = context.mBarrier;
	{
		JPH_PROFILE("Build job barrier");

		StaticArray<JobHandle, cMaxPhysicsJobs> handles;
		for (const PhysicsUpdateContext::Step &step : context.mSteps)
		{
			if (step.mBroadPhasePrepare.IsValid())
				handles.push_back(step.mBroadPhasePrepare);
			for (const JobHandle &h : step.mStepListeners)
				handles.push_back(h);
			for (const JobHandle &h : step.mDetermineActiveConstraints)
				handles.push_back(h);
			for (const JobHandle &h : step.mApplyGravity)
				handles.push_back(h);
			for (const JobHandle &h : step.mFindCollisions)
				handles.push_back(h);
			if (step.mUpdateBroadphaseFinalize.IsValid())
				handles.push_back(step.mUpdateBroadphaseFinalize);
			handles.push_back(step.mSetupVelocityConstraints);
			handles.push_back(step.mBuildIslandsFromConstraints);
			handles.push_back(step.mFinalizeIslands);
			handles.push_back(step.mBodySetIslandIndex);
			for (const PhysicsUpdateContext::SubStep &sub_step : step.mSubSteps)
			{
				for (const JobHandle &h : sub_step.mSolveVelocityConstraints)
					handles.push_back(h);
				handles.push_back(sub_step.mPreIntegrateVelocity);
				for (const JobHandle &h : sub_step.mIntegrateVelocity)
					handles.push_back(h);
				handles.push_back(sub_step.mPostIntegrateVelocity);
				handles.push_back(sub_step.mResolveCCDContacts);
				for (const JobHandle &h : sub_step.mSolvePositionConstraints)
					handles.push_back(h);
				if (sub_step.mStartNextSubStep.IsValid())
					handles.push_back(sub_step.mStartNextSubStep);
			}
			handles.push_back(step.mContactRemovedCallbacks);
		}
		barrier->AddJobs(handles.data(), handles.size());
	}

	// Wait until all jobs finish
	// Note we don't just wait for the last job. If we would and another job
	// would be scheduled in between there is the possibility of a deadlock.
	// The other job could try to e.g. add/remove a body which would try to
	// lock a body mutex while this thread has already locked the mutex
	inJobSystem->WaitForJobs(barrier);

	// We're done with the barrier for this update
	inJobSystem->DestroyBarrier(barrier);

#ifdef _DEBUG
	// Validate that the cached bounds are correct
	mBodyManager.ValidateActiveBodyBounds();
#endif // _DEBUG
	
	// Clear the island builder
	mIslandBuilder.ResetIslands(inTempAllocator);

	// Clear the contact manager
	mContactManager.FinishConstraintBuffer();

	// Free active constraints
	inTempAllocator->Free(context.mActiveConstraints, mConstraintManager.GetNumConstraints() * sizeof(Constraint *));
	context.mActiveConstraints = nullptr;

	// Free body pairs
	inTempAllocator->Free(context.mBodyPairs, sizeof(BodyPair) * mPhysicsSettings.mMaxInFlightBodyPairs);
	context.mBodyPairs = nullptr;
	
	// Unlock the broadphase
	mBroadPhase->UnlockModifications();

	// Unlock all constraints
	mConstraintManager.UnlockAllConstraints();

#ifdef JPH_ENABLE_ASSERTS
	// Allow write operations to the active bodies list
	mBodyManager.SetActiveBodiesLocked(false);
#endif

	// Unlock all bodies
	mBodyManager.UnlockAllBodies();

	// Unlock step listeners
	mStepListenersMutex.unlock();
}

void PhysicsSystem::JobStepListeners(PhysicsUpdateContext::Step *ioStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// Read positions (broadphase updates concurrently so we can't write), read/write velocities
	BodyAccess::Grant grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);

	// Can activate bodies only (we cache the amount of active bodies at the beginning of the step in mNumActiveBodiesAtStepStart so we cannot deactivate here)
	BodyManager::GrantActiveBodiesAccess grant_active(true, false);
#endif

	float step_time = ioStep->mContext->mStepDeltaTime;
	uint32 batch_size = mPhysicsSettings.mStepListenersBatchSize;
	for (;;)
	{
		// Get the start of a new batch
		uint32 batch = ioStep->mStepListenerReadIdx.fetch_add(batch_size);
		if (batch >= mStepListeners.size())
			break;

		// Call the listeners
		for (uint32 i = batch, i_end = min((uint32)mStepListeners.size(), batch + batch_size); i < i_end; ++i)
			mStepListeners[i]->OnStep(step_time, *this);
	}
}

void PhysicsSystem::JobDetermineActiveConstraints(PhysicsUpdateContext::Step *ioStep) const
{
#ifdef JPH_ENABLE_ASSERTS
	// No body access
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
#endif

	uint32 num_constraints = mConstraintManager.GetNumConstraints();
	uint32 num_active_constraints;
	Constraint **active_constraints = (Constraint **)JPH_STACK_ALLOC(cDetermineActiveConstraintsBatchSize * sizeof(Constraint *));

	for (;;)
	{
		// Atomically fetch a batch of constraints
		uint32 constraint_idx = ioStep->mConstraintReadIdx.fetch_add(cDetermineActiveConstraintsBatchSize);
		if (constraint_idx >= num_constraints)
			break;

		// Calculate the end of the batch
		uint32 constraint_idx_end = min(num_constraints, constraint_idx + cDetermineActiveConstraintsBatchSize);

		// Store the active constraints at the start of the step (bodies get activated during the step which in turn may activate constraints leading to an inconsistent shapshot)
		mConstraintManager.GetActiveConstraints(constraint_idx, constraint_idx_end, active_constraints, num_active_constraints);

		// Copy the block of active constraints to the global list of active constraints
		if (num_active_constraints > 0)
		{
			uint32 active_constraint_idx = ioStep->mNumActiveConstraints.fetch_add(num_active_constraints);
			memcpy(ioStep->mContext->mActiveConstraints + active_constraint_idx, active_constraints, num_active_constraints * sizeof(Constraint *));
		}
	}
}

void PhysicsSystem::JobApplyGravity(const PhysicsUpdateContext *ioContext, PhysicsUpdateContext::Step *ioStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We update velocities and need the rotation to do so
	BodyAccess::Grant grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);
#endif

	// Get list of active bodies that we had at the start of the physics update.
	// Any body that is activated as part of the simulation step does not receive gravity this frame.
	// Note that bodies may be activated during this job but not deactivated, this means that only elements
	// will be added to the array. Since the array is made to not reallocate, this is a safe operation.
	const BodyID *active_bodies = mBodyManager.GetActiveBodiesUnsafe();
	uint32 num_active_bodies_at_step_start = ioStep->mNumActiveBodiesAtStepStart;

	// Fetch delta time once outside the loop
	float delta_time = ioContext->mSubStepDeltaTime;

	// Update velocities from forces
	for (;;)
	{
		// Atomically fetch a batch of bodies
		uint32 active_body_idx = ioStep->mApplyGravityReadIdx.fetch_add(cApplyGravityBatchSize);
		if (active_body_idx >= num_active_bodies_at_step_start)
			break;

		// Calculate the end of the batch
		uint32 active_body_idx_end = min(num_active_bodies_at_step_start, active_body_idx + cApplyGravityBatchSize);

		// Process the batch
		while (active_body_idx < active_body_idx_end)
		{
			Body &body = mBodyManager.GetBody(active_bodies[active_body_idx]);
			if (body.IsDynamic())
				body.GetMotionProperties()->ApplyForceTorqueAndDragInternal(body.GetRotation(), mGravity, delta_time);
			active_body_idx++;
		}
	}
}

void PhysicsSystem::JobSetupVelocityConstraints(float inDeltaTime, PhysicsUpdateContext::Step *ioStep) const
{
#ifdef JPH_ENABLE_ASSERTS
	// We only read positions
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);
#endif

	ConstraintManager::sSetupVelocityConstraints(ioStep->mContext->mActiveConstraints, ioStep->mNumActiveConstraints, inDeltaTime); 
}

void PhysicsSystem::JobBuildIslandsFromConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::Step *ioStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We read constraints and positions
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);

	// Can only activate bodies
	BodyManager::GrantActiveBodiesAccess grant_active(true, false);
#endif

	// Prepare the island builder
	mIslandBuilder.PrepareNonContactConstraints(ioStep->mNumActiveConstraints, ioContext->mTempAllocator);

	// Build the islands
	ConstraintManager::sBuildIslands(ioStep->mContext->mActiveConstraints, ioStep->mNumActiveConstraints, mIslandBuilder, mBodyManager);
}

void PhysicsSystem::TrySpawnJobFindCollisions(PhysicsUpdateContext::Step *ioStep) const
{
	// Get how many jobs we can spawn and check if we can spawn more
	uint max_jobs = ioStep->mBodyPairQueues.size();
	if (CountBits(ioStep->mActiveFindCollisionJobs) >= max_jobs)
		return;

	// Count how many body pairs we have waiting
	uint32 num_body_pairs = 0;
	for (const PhysicsUpdateContext::BodyPairQueue &queue : ioStep->mBodyPairQueues)
		num_body_pairs += queue.mWriteIdx - queue.mReadIdx;

	// Count how many active bodies we have waiting
	uint32 num_active_bodies = mBodyManager.GetNumActiveBodies() - ioStep->mActiveBodyReadIdx;

	// Calculate how many jobs we would like
	uint desired_num_jobs = min((num_body_pairs + cNarrowPhaseBatchSize - 1) / cNarrowPhaseBatchSize + (num_active_bodies + cActiveBodiesBatchSize - 1) / cActiveBodiesBatchSize, max_jobs);

	for (;;)
	{
		// Get the bit mask of active jobs and see if we can spawn more
		PhysicsUpdateContext::JobMask current_active_jobs = ioStep->mActiveFindCollisionJobs;
		if (CountBits(current_active_jobs) >= desired_num_jobs)
			break;

		// Loop through all possible job indices
		for (uint job_index = 0; job_index < max_jobs; ++job_index)
		{
			// Test if it has been started
			PhysicsUpdateContext::JobMask job_mask = PhysicsUpdateContext::JobMask(1) << job_index;
			if ((current_active_jobs & job_mask) == 0)
			{
				// Try to claim the job index
				PhysicsUpdateContext::JobMask prev_value = ioStep->mActiveFindCollisionJobs.fetch_or(job_mask);
				if ((prev_value & job_mask) == 0)
				{
					// Add dependencies from the find collisions job to the next jobs
					ioStep->mUpdateBroadphaseFinalize.AddDependency();
					ioStep->mFinalizeIslands.AddDependency();

					// Start the job
					JobHandle job = ioStep->mContext->mJobSystem->CreateJob("FindCollisions", cColorFindCollisions, [step = ioStep, job_index]() 
						{ 
							step->mContext->mPhysicsSystem->JobFindCollisions(step, job_index); 
						});

					// Add the job to the job barrier so the main updating thread can execute the job too
					ioStep->mContext->mBarrier->AddJob(job);

					// Spawn only 1 extra job at a time
					return;
				}
			}
		}
	}
}

void PhysicsSystem::JobFindCollisions(PhysicsUpdateContext::Step *ioStep, int inJobIndex)
{
#ifdef JPH_ENABLE_ASSERTS
	// We only read positions
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);
#endif

	// Allocation context for allocating new contact points
	ContactAllocator contact_allocator(mContactManager.GetContactAllocator());

	// Determine initial queue to read pairs from if no broadphase work can be done
	// (always start looking at results from the next job)
	int read_queue_idx = (inJobIndex + 1) % ioStep->mBodyPairQueues.size();

	for (;;)
	{
		// Check if there are active bodies to be processed
		uint32 active_bodies_read_idx = ioStep->mActiveBodyReadIdx;
		uint32 num_active_bodies = mBodyManager.GetNumActiveBodies();
		if (active_bodies_read_idx < num_active_bodies)
		{
			// Take a batch of active bodies
			uint32 active_bodies_read_idx_end = min(num_active_bodies, active_bodies_read_idx + cActiveBodiesBatchSize);
			if (ioStep->mActiveBodyReadIdx.compare_exchange_strong(active_bodies_read_idx, active_bodies_read_idx_end))
			{
				// Callback when a new body pair is found
				class MyBodyPairCallback : public BodyPairCollector
				{
				public:
					// Constructor
											MyBodyPairCallback(PhysicsUpdateContext::Step *inStep, ContactAllocator &ioContactAllocator, int inJobIndex) :
						mStep(inStep),
						mContactAllocator(ioContactAllocator),
						mJobIndex(inJobIndex)
					{
					}

					// Callback function when a body pair is found
					virtual void			AddHit(const BodyPair &inPair) override
					{
						// Check if we have space in our write queue
						PhysicsUpdateContext::BodyPairQueue &queue = mStep->mBodyPairQueues[mJobIndex];
						uint32 body_pairs_in_queue = queue.mWriteIdx - queue.mReadIdx;
						if (body_pairs_in_queue >= mStep->mMaxBodyPairsPerQueue)
						{
							// Buffer full, process the pair now
							mStep->mContext->mPhysicsSystem->ProcessBodyPair(mContactAllocator, inPair);
						}
						else
						{
							// Store the pair in our own queue
							mStep->mContext->mBodyPairs[mJobIndex * mStep->mMaxBodyPairsPerQueue + queue.mWriteIdx % mStep->mMaxBodyPairsPerQueue] = inPair;
							++queue.mWriteIdx;
						}
					}

				private:
					PhysicsUpdateContext::Step *	mStep;
					ContactAllocator &				mContactAllocator;
					int								mJobIndex;
				};
				MyBodyPairCallback add_pair(ioStep, contact_allocator, inJobIndex);

				// Copy active bodies to temporary array, broadphase will reorder them
				uint32 batch_size = active_bodies_read_idx_end - active_bodies_read_idx;
				BodyID *active_bodies = (BodyID *)JPH_STACK_ALLOC(batch_size * sizeof(BodyID));
				memcpy(active_bodies, mBodyManager.GetActiveBodiesUnsafe() + active_bodies_read_idx, batch_size * sizeof(BodyID));

				// Find pairs in the broadphase
				mBroadPhase->FindCollidingPairs(active_bodies, batch_size, mPhysicsSettings.mSpeculativeContactDistance, mObjectVsBroadPhaseLayerFilter, mObjectLayerPairFilter, add_pair);

				// Check if we have enough pairs in the buffer to start a new job
				const PhysicsUpdateContext::BodyPairQueue &queue = ioStep->mBodyPairQueues[inJobIndex];
				uint32 body_pairs_in_queue = queue.mWriteIdx - queue.mReadIdx;
				if (body_pairs_in_queue >= cNarrowPhaseBatchSize)
					TrySpawnJobFindCollisions(ioStep);
			}
		}
		else 
		{
			// Lockless loop to get the next body pair from the pairs buffer
			const PhysicsUpdateContext *context = ioStep->mContext;
			int first_read_queue_idx = read_queue_idx;
			for (;;)
			{
				PhysicsUpdateContext::BodyPairQueue &queue = ioStep->mBodyPairQueues[read_queue_idx];

				// Get the next pair to process
				uint32 pair_idx = queue.mReadIdx;

				// If the pair hasn't been written yet
				if (pair_idx >= queue.mWriteIdx)
				{
					// Go to the next queue
					read_queue_idx = (read_queue_idx + 1) % ioStep->mBodyPairQueues.size();

					// If we're back at the first queue, we've looked at all of them and found nothing
					if (read_queue_idx == first_read_queue_idx)
					{
						// Atomically accumulate the number of found manifolds and body pairs
						ioStep->mNumBodyPairs += contact_allocator.mNumBodyPairs;
						ioStep->mNumManifolds += contact_allocator.mNumManifolds;

						// Mark this job as inactive
						ioStep->mActiveFindCollisionJobs.fetch_and(~PhysicsUpdateContext::JobMask(1 << inJobIndex));

						// Trigger the next jobs
						ioStep->mUpdateBroadphaseFinalize.RemoveDependency();
						ioStep->mFinalizeIslands.RemoveDependency();
						return;
					}

					// Try again reading from the next queue
					continue;
				}

				// Copy the body pair out of the buffer
				const BodyPair bp = context->mBodyPairs[read_queue_idx * ioStep->mMaxBodyPairsPerQueue + pair_idx % ioStep->mMaxBodyPairsPerQueue];
			
				// Mark this pair as taken
				if (queue.mReadIdx.compare_exchange_strong(pair_idx, pair_idx + 1))
				{
					// Process the actual body pair
					ProcessBodyPair(contact_allocator, bp);
					break;
				}
			}
		}
	}
}

void PhysicsSystem::ProcessBodyPair(ContactAllocator &ioContactAllocator, const BodyPair &inBodyPair)
{
	JPH_PROFILE_FUNCTION();

#ifdef JPH_ENABLE_ASSERTS
	// We read positions and read velocities (for elastic collisions)
	BodyAccess::Grant grant(BodyAccess::EAccess::Read, BodyAccess::EAccess::Read);

	// Can only activate bodies
	BodyManager::GrantActiveBodiesAccess grant_active(true, false);
#endif

	// Fetch body pair
	Body *body1 = &mBodyManager.GetBody(inBodyPair.mBodyA);
	Body *body2 = &mBodyManager.GetBody(inBodyPair.mBodyB);
	JPH_ASSERT(body1->IsActive());

	// Ensure that body1 is dynamic, this ensures that we do the collision detection in the space of a moving body, which avoids accuracy problems when testing a very large static object against a small dynamic object
	// Ensure that body1 id < body2 id for dynamic vs dynamic
	// Keep body order unchanged when colliding with a sensor
	if ((!body1->IsDynamic() || (body2->IsDynamic() && inBodyPair.mBodyB < inBodyPair.mBodyA)) 
		&& !body2->IsSensor())
		swap(body1, body2);
	JPH_ASSERT(body1->IsDynamic() || (body1->IsKinematic() && body2->IsSensor()));

	// Check if the contact points from the previous frame are reusable and if so copy them
	bool pair_handled = false, constraint_created = false;
	if (mPhysicsSettings.mUseBodyPairContactCache && !(body1->IsCollisionCacheInvalid() || body2->IsCollisionCacheInvalid()))
		mContactManager.GetContactsFromCache(ioContactAllocator, *body1, *body2, pair_handled, constraint_created);

	// If the cache hasn't handled this body pair do actual collision detection
	if (!pair_handled)
	{
		// Create entry in the cache for this body pair
		// Needs to happen irrespective if we found a collision or not (we want to remember that no collision was found too)
		ContactConstraintManager::BodyPairHandle body_pair_handle = mContactManager.AddBodyPair(ioContactAllocator, *body1, *body2);
		if (body_pair_handle == nullptr)
			return; // Out of cache space

		// Create the query settings
		CollideShapeSettings settings;
		settings.mCollectFacesMode = ECollectFacesMode::CollectFaces;
		settings.mActiveEdgeMode = mPhysicsSettings.mCheckActiveEdges? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;
		settings.mMaxSeparationDistance = mPhysicsSettings.mSpeculativeContactDistance;
		settings.mActiveEdgeMovementDirection = body1->GetLinearVelocity() - body2->GetLinearVelocity();

		if (mPhysicsSettings.mUseManifoldReduction)
		{
			// Version WITH contact manifold reduction

			class MyManifold : public ContactManifold
			{
			public:
				Vec3				mFirstWorldSpaceNormal;
			};

			// A temporary structure that allows us to keep track of the all manifolds between this body pair
			using Manifolds = StaticArray<MyManifold, 32>;

			// Create collector
			class ReductionCollideShapeCollector : public CollideShapeCollector
			{
			public:
								ReductionCollideShapeCollector(PhysicsSystem *inSystem, const Body *inBody1, const Body *inBody2) : 
					mSystem(inSystem), 
					mBody1(inBody1),
					mBody2(inBody2)
				{ 
				}

				virtual void	AddHit(const CollideShapeResult &inResult) override
				{
					// One of the following should be true:
					// - Body 1 is dynamic and body 2 may be dynamic, static or kinematic
					// - Body 1 is kinematic in which case body 2 should be a sensor
					JPH_ASSERT(mBody1->IsDynamic() || (mBody1->IsKinematic() && mBody2->IsSensor()));
					JPH_ASSERT(!ShouldEarlyOut());

					// Test if we want to accept this hit
					if (mValidateBodyPair)
					{
						switch (mSystem->mContactManager.ValidateContactPoint(*mBody1, *mBody2, inResult))
						{
						case ValidateResult::AcceptContact:
							// We're just accepting this one, nothing to do
							break;

						case ValidateResult::AcceptAllContactsForThisBodyPair:
							// Accept and stop calling the validate callback
							mValidateBodyPair = false;
							break;

						case ValidateResult::RejectContact:
							// Skip this contact
							return;

						case ValidateResult::RejectAllContactsForThisBodyPair:
							// Skip this and early out
							ForceEarlyOut();
							return;
						}
					}

					// Calculate normal
					Vec3 world_space_normal = inResult.mPenetrationAxis.Normalized();
			
					// Check if we can add it to an existing manifold
					Manifolds::iterator manifold;
					float contact_normal_cos_max_delta_rot = mSystem->mPhysicsSettings.mContactNormalCosMaxDeltaRotation;
					for (manifold = mManifolds.begin(); manifold != mManifolds.end(); ++manifold)
						if (world_space_normal.Dot(manifold->mFirstWorldSpaceNormal) >= contact_normal_cos_max_delta_rot)
						{
							// Update average normal
							manifold->mWorldSpaceNormal += world_space_normal;
							manifold->mPenetrationDepth = max(manifold->mPenetrationDepth, inResult.mPenetrationDepth);
							break;
						}
					if (manifold == mManifolds.end())
					{
						// Check if array is full
						if (mManifolds.size() == mManifolds.capacity())
						{
							// Full, find manifold with least amount of penetration
							manifold = mManifolds.begin();
							for (Manifolds::iterator m = mManifolds.begin() + 1; m < mManifolds.end(); ++m)
								if (m->mPenetrationDepth < manifold->mPenetrationDepth)
									manifold = m;

							// If this contacts penetration is smaller than the smallest manifold, we skip this contact
							if (inResult.mPenetrationDepth < manifold->mPenetrationDepth)
								return;

							// Replace the manifold
							*manifold = { { world_space_normal, inResult.mPenetrationDepth, inResult.mSubShapeID1, inResult.mSubShapeID2, { }, { } }, world_space_normal };
						}
						else
						{
							// Not full, create new manifold
							mManifolds.push_back({ { world_space_normal, inResult.mPenetrationDepth, inResult.mSubShapeID1, inResult.mSubShapeID2, { }, { } }, world_space_normal });
							manifold = mManifolds.end() - 1;
						}
					}

					// Determine contact points
					const PhysicsSettings &settings = mSystem->mPhysicsSettings;
					ManifoldBetweenTwoFaces(inResult.mContactPointOn1, inResult.mContactPointOn2, inResult.mPenetrationAxis, Square(settings.mSpeculativeContactDistance) + settings.mManifoldToleranceSq, inResult.mShape1Face, inResult.mShape2Face, manifold->mWorldSpaceContactPointsOn1, manifold->mWorldSpaceContactPointsOn2);

					// Prune if we have more than 32 points (this means we could run out of space in the next iteration)
					if (manifold->mWorldSpaceContactPointsOn1.size() > 32)
						PruneContactPoints(mBody1->GetCenterOfMassPosition(), manifold->mFirstWorldSpaceNormal, manifold->mWorldSpaceContactPointsOn1, manifold->mWorldSpaceContactPointsOn2);
				}

				PhysicsSystem *		mSystem;
				const Body *		mBody1;
				const Body *		mBody2;
				bool				mValidateBodyPair = true;
				Manifolds			mManifolds;
			};
			ReductionCollideShapeCollector collector(this, body1, body2);

			// Perform collision detection between the two shapes
			SubShapeIDCreator part1, part2;
			CollisionDispatch::sCollideShapeVsShape(body1->GetShape(), body2->GetShape(), Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), body1->GetCenterOfMassTransform(), body2->GetCenterOfMassTransform(), part1, part2, settings, collector);

			// Add the contacts
			for (ContactManifold &manifold : collector.mManifolds)
			{
				// Normalize the normal (is a sum of all normals from merged manifolds)
				manifold.mWorldSpaceNormal = manifold.mWorldSpaceNormal.Normalized();

				// If we still have too many points, prune them now
				if (manifold.mWorldSpaceContactPointsOn1.size() > 4)
					PruneContactPoints(body1->GetCenterOfMassPosition(), manifold.mWorldSpaceNormal, manifold.mWorldSpaceContactPointsOn1, manifold.mWorldSpaceContactPointsOn2);

				// Actually add the contact points to the manager
				constraint_created |= mContactManager.AddContactConstraint(ioContactAllocator, body_pair_handle, *body1, *body2, manifold);
			}
		}
		else
		{
			// Version WITHOUT contact manifold reduction

			// Create collector
			class NonReductionCollideShapeCollector : public CollideShapeCollector
			{
			public:
								NonReductionCollideShapeCollector(PhysicsSystem *inSystem, ContactAllocator &ioContactAllocator, Body *inBody1, Body *inBody2, const ContactConstraintManager::BodyPairHandle &inPairHandle) : 
					mSystem(inSystem), 
					mContactAllocator(ioContactAllocator),
					mBody1(inBody1),
					mBody2(inBody2),
					mBodyPairHandle(inPairHandle)
				{ 
				}

				virtual void	AddHit(const CollideShapeResult &inResult) override
				{
					// Body 1 should always be dynamic, body 2 may be static / kinematic
					JPH_ASSERT(mBody1->IsDynamic());
					JPH_ASSERT(!ShouldEarlyOut());

					// Test if we want to accept this hit
					if (mValidateBodyPair)
					{
						switch (mSystem->mContactManager.ValidateContactPoint(*mBody1, *mBody2, inResult))
						{
						case ValidateResult::AcceptContact:
							// We're just accepting this one, nothing to do
							break;

						case ValidateResult::AcceptAllContactsForThisBodyPair:
							// Accept and stop calling the validate callback
							mValidateBodyPair = false;
							break;

						case ValidateResult::RejectContact:
							// Skip this contact
							return;

						case ValidateResult::RejectAllContactsForThisBodyPair:
							// Skip this and early out
							ForceEarlyOut();
							return;
						}
					}

					// Determine contact points
					ContactManifold manifold;
					const PhysicsSettings &settings = mSystem->mPhysicsSettings;
					ManifoldBetweenTwoFaces(inResult.mContactPointOn1, inResult.mContactPointOn2, inResult.mPenetrationAxis, Square(settings.mSpeculativeContactDistance) + settings.mManifoldToleranceSq, inResult.mShape1Face, inResult.mShape2Face, manifold.mWorldSpaceContactPointsOn1, manifold.mWorldSpaceContactPointsOn2);

					// Calculate normal
					manifold.mWorldSpaceNormal = inResult.mPenetrationAxis.Normalized();

					// Store penetration depth
					manifold.mPenetrationDepth = inResult.mPenetrationDepth;
			
					// Prune if we have more than 4 points
					if (manifold.mWorldSpaceContactPointsOn1.size() > 4)
						PruneContactPoints(mBody1->GetCenterOfMassPosition(), manifold.mWorldSpaceNormal, manifold.mWorldSpaceContactPointsOn1, manifold.mWorldSpaceContactPointsOn2);

					// Set other properties
					manifold.mSubShapeID1 = inResult.mSubShapeID1;
					manifold.mSubShapeID2 = inResult.mSubShapeID2;

					// Actually add the contact points to the manager
					mConstraintCreated |= mSystem->mContactManager.AddContactConstraint(mContactAllocator, mBodyPairHandle, *mBody1, *mBody2, manifold);
				}

				PhysicsSystem *		mSystem;
				ContactAllocator &	mContactAllocator;
				Body *				mBody1;
				Body *				mBody2;
				ContactConstraintManager::BodyPairHandle mBodyPairHandle;
				bool				mValidateBodyPair = true;
				bool				mConstraintCreated = false;
			};
			NonReductionCollideShapeCollector collector(this, ioContactAllocator, body1, body2, body_pair_handle);

			// Perform collision detection between the two shapes
			SubShapeIDCreator part1, part2;
			CollisionDispatch::sCollideShapeVsShape(body1->GetShape(), body2->GetShape(), Vec3::sReplicate(1.0f), Vec3::sReplicate(1.0f), body1->GetCenterOfMassTransform(), body2->GetCenterOfMassTransform(), part1, part2, settings, collector);

			constraint_created = collector.mConstraintCreated;
		}
	}

	// If a contact constraint was created, we need to do some extra work
	if (constraint_created)
	{
		// Wake up sleeping bodies
		BodyID body_ids[2];
		int num_bodies = 0;
		if (body1->IsDynamic() && !body1->IsActive())
			body_ids[num_bodies++] = body1->GetID();
		if (body2->IsDynamic() && !body2->IsActive())
			body_ids[num_bodies++] = body2->GetID();
		if (num_bodies > 0)
			mBodyManager.ActivateBodies(body_ids, num_bodies);

		// Link the two bodies
		mIslandBuilder.LinkBodies(body1->GetIndexInActiveBodiesInternal(), body2->GetIndexInActiveBodiesInternal());
	}
}

void PhysicsSystem::JobFinalizeIslands(PhysicsUpdateContext *ioContext)
{
#ifdef JPH_ENABLE_ASSERTS
	// We only touch island data
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
#endif

	// Finish collecting the islands, at this point the active body list doesn't change so it's safe to access
	mIslandBuilder.Finalize(mBodyManager.GetActiveBodiesUnsafe(), mBodyManager.GetNumActiveBodies(), mContactManager.GetNumConstraints(), ioContext->mTempAllocator);
}

void PhysicsSystem::JobBodySetIslandIndex()
{
#ifdef JPH_ENABLE_ASSERTS
	// We only touch island data
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
#endif

	// Loop through the result and tag all bodies with an island index
	for (uint32 island_idx = 0, n = mIslandBuilder.GetNumIslands(); island_idx < n; ++island_idx)
	{
		BodyID *body_start, *body_end;
		mIslandBuilder.GetBodiesInIsland(island_idx, body_start, body_end);
		for (const BodyID *body = body_start; body < body_end; ++body)
			mBodyManager.GetBody(*body).GetMotionProperties()->SetIslandIndexInternal(island_idx);
	}
}

void PhysicsSystem::JobSolveVelocityConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We update velocities and need to read positions to do so
	BodyAccess::Grant grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);
#endif
	
	float delta_time = ioContext->mSubStepDeltaTime;
	float warm_start_impulse_ratio = ioContext->mWarmStartImpulseRatio;
	Constraint **active_constraints = ioContext->mActiveConstraints;

	bool first_sub_step = ioSubStep->mIsFirst;
	bool last_sub_step = ioSubStep->mIsLast;

	for (;;)
	{
		// Next island
		uint32 island_idx = ioSubStep->mSolveVelocityConstraintsNextIsland++;
		if (island_idx >= mIslandBuilder.GetNumIslands())
			break;

		JPH_PROFILE("Island");

		// Get iterators
		uint32 *constraints_begin, *constraints_end;
		bool has_constraints = mIslandBuilder.GetConstraintsInIsland(island_idx, constraints_begin, constraints_end);
		uint32 *contacts_begin, *contacts_end;
		bool has_contacts = mIslandBuilder.GetContactsInIsland(island_idx, contacts_begin, contacts_end);
		
		if (first_sub_step)
		{
			// If we don't have any contacts or constraints, we know that none of the following islands have any contacts or constraints
			// (because they're sorted by most constraints first). This means we're done.
			if (!has_contacts && !has_constraints)
			{
			#ifdef JPH_ENABLE_ASSERTS
				// Validate our assumption that the next islands don't have any constraints or contacts
				for (; island_idx < mIslandBuilder.GetNumIslands(); ++island_idx)
				{
					JPH_ASSERT(!mIslandBuilder.GetConstraintsInIsland(island_idx, constraints_begin, constraints_end));
					JPH_ASSERT(!mIslandBuilder.GetContactsInIsland(island_idx, contacts_begin, contacts_end));
				}
			#endif // JPH_ENABLE_ASSERTS
				return;
			}

			// Sort constraints to give a deterministic simulation
			ConstraintManager::sSortConstraints(active_constraints, constraints_begin, constraints_end);

			// Sort contacts to give a deterministic simulation
			mContactManager.SortContacts(contacts_begin, contacts_end);
		}
		else
		{
			{
				JPH_PROFILE("Apply Gravity");

				// Get bodies in this island
				BodyID *bodies_begin, *bodies_end;
				mIslandBuilder.GetBodiesInIsland(island_idx, bodies_begin, bodies_end);

				// Apply gravity. In the first step this is done in a separate job.
				for (const BodyID *body_id = bodies_begin; body_id < bodies_end; ++body_id)
				{
					Body &body = mBodyManager.GetBody(*body_id);
					if (body.IsDynamic())
						body.GetMotionProperties()->ApplyForceTorqueAndDragInternal(body.GetRotation(), mGravity, delta_time);
				}
			}

			// If we don't have any contacts or constraints, we don't need to run the solver, but we do need to process
			// the next island in order to apply gravity
			if (!has_contacts && !has_constraints)
				continue;

			// Prepare velocity constraints. In the first step this is done when adding the contact constraints.
			ConstraintManager::sSetupVelocityConstraints(active_constraints, constraints_begin, constraints_end, delta_time);
			mContactManager.SetupVelocityConstraints(contacts_begin, contacts_end, delta_time);
		}

		// Warm start
		ConstraintManager::sWarmStartVelocityConstraints(active_constraints, constraints_begin, constraints_end, warm_start_impulse_ratio);
		mContactManager.WarmStartVelocityConstraints(contacts_begin, contacts_end, warm_start_impulse_ratio);

		// Solve
		for (int velocity_step = 0; velocity_step < mPhysicsSettings.mNumVelocitySteps; ++velocity_step)
		{
			bool constraint_impulse = ConstraintManager::sSolveVelocityConstraints(active_constraints, constraints_begin, constraints_end, delta_time);
			bool contact_impulse = mContactManager.SolveVelocityConstraints(contacts_begin, contacts_end);
			if (!constraint_impulse && !contact_impulse)
				break;
		}

		// Save back the lambdas in the contact cache for the warm start of the next physics update
		if (last_sub_step)
			mContactManager.StoreAppliedImpulses(contacts_begin, contacts_end);
	}
}

void PhysicsSystem::JobPreIntegrateVelocity(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep) const
{
	// Reserve enough space for all bodies that may need a cast
	TempAllocator *temp_allocator = ioContext->mTempAllocator;
	JPH_ASSERT(ioSubStep->mCCDBodies == nullptr);
	ioSubStep->mCCDBodiesCapacity = mBodyManager.GetNumActiveCCDBodies();
	ioSubStep->mCCDBodies = (CCDBody *)temp_allocator->Allocate(ioSubStep->mCCDBodiesCapacity * sizeof(CCDBody));

	// Initialize the mapping table between active body and CCD body
	JPH_ASSERT(ioSubStep->mActiveBodyToCCDBody == nullptr);
	ioSubStep->mNumActiveBodyToCCDBody = mBodyManager.GetNumActiveBodies();
	ioSubStep->mActiveBodyToCCDBody = (int *)temp_allocator->Allocate(ioSubStep->mNumActiveBodyToCCDBody * sizeof(int));
}

void PhysicsSystem::JobIntegrateVelocity(const PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We update positions and need velocity to do so, we also clamp velocities so need to write to them
	BodyAccess::Grant grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::ReadWrite);
#endif

	float delta_time = ioContext->mSubStepDeltaTime;
	const BodyID *active_bodies = mBodyManager.GetActiveBodiesUnsafe();
	uint32 num_active_bodies = mBodyManager.GetNumActiveBodies();
	uint32 num_active_bodies_after_find_collisions = ioSubStep->mStep->mActiveBodyReadIdx;

	// We can move bodies that are not part of an island. In this case we need to notify the broadphase of the movement.
	static constexpr int cBodiesBatch = 64;
	BodyID *bodies_to_update_bounds = (BodyID *)JPH_STACK_ALLOC(cBodiesBatch * sizeof(BodyID));
	int num_bodies_to_update_bounds = 0;

	for (;;)
	{
		// Atomically fetch a batch of bodies
		uint32 active_body_idx = ioSubStep->mIntegrateVelocityReadIdx.fetch_add(cIntegrateVelocityBatchSize);
		if (active_body_idx >= num_active_bodies)
			break;

		// Calculate the end of the batch
		uint32 active_body_idx_end = min(num_active_bodies, active_body_idx + cIntegrateVelocityBatchSize);

		// Process the batch
		while (active_body_idx < active_body_idx_end)
		{
			// Update the positions using an Symplectic Euler step (which integrates using the updated velocity v1' rather
			// than the original velocity v1):
			// x1' = x1 + h * v1'
			// At this point the active bodies list does not change, so it is safe to access the array.
			BodyID body_id = active_bodies[active_body_idx];
			Body &body = mBodyManager.GetBody(body_id);
			MotionProperties *mp = body.GetMotionProperties();

			// Clamp velocities (not for kinematic bodies)
			if (body.IsDynamic())
			{
				mp->ClampLinearVelocity();
				mp->ClampAngularVelocity();
			}

			// Update the rotation of the body according to the angular velocity
			// For motion type discrete we need to do this anyway, for motion type linear cast we have multiple choices
			// 1. Rotate the body first and then sweep
			// 2. First sweep and then rotate the body at the end
			// 3. Pick some inbetween rotation (e.g. half way), then sweep and finally rotate the remainder
			// (1) has some clear advantages as when a long thin body hits a surface away from the center of mass, this will result in a large angular velocity and a limited reduction in linear velocity.
			// When simulation the rotation first before doing the translation, the body will be able to rotate away from the contact point allowing the center of mass to approach the surface. When using
			// approach (2) in this case what will happen is that we will immediately detect the same collision again (the body has not rotated and the body was already colliding at the end of the previous
			// time step) resulting in a lot of stolen time and the body appearing to be frozen in an unnatural pose (like it is glued at an angle to the surface). (2) obviously has some negative side effects
			// too as simulating the rotation first may cause it to tunnel through a small object that the linear cast might have otherwise dectected. In any case a linear cast is not good for detecting
			// tunneling due to angular rotation, so we don't care about that too much (you'd need a full cast to take angular effects into account).
			body.AddRotationStep(body.GetAngularVelocity() * delta_time);

			// Get delta position
			Vec3 delta_pos = body.GetLinearVelocity() * delta_time;

			// If the position should be updated (or if it is delayed because of CCD)
			bool update_position = true;

			switch (mp->GetMotionQuality())
			{
			case EMotionQuality::Discrete:
				// No additional collision checking to be done
				break;

			case EMotionQuality::LinearCast:
				if (body.IsDynamic() // Kinematic bodies cannot be stopped
					&& !body.IsSensor()) // We don't support CCD sensors
				{
					// Determine inner radius (the smallest sphere that fits into the shape)
					float inner_radius = body.GetShape()->GetInnerRadius();
					JPH_ASSERT(inner_radius > 0.0f, "The shape has no inner radius, this makes the shape unsuitable for the linear cast motion quality as we cannot move it without risking tunneling.");

					// Measure translation in this step and check if it above the treshold to perform a linear cast
					float linear_cast_threshold_sq = Square(mPhysicsSettings.mLinearCastThreshold * inner_radius);
					if (delta_pos.LengthSq() > linear_cast_threshold_sq)
					{
						// This body needs a cast
						uint32 ccd_body_idx = ioSubStep->mNumCCDBodies++;
						ioSubStep->mActiveBodyToCCDBody[active_body_idx] = ccd_body_idx;
						new (&ioSubStep->mCCDBodies[ccd_body_idx]) CCDBody(body_id, delta_pos, linear_cast_threshold_sq, min(mPhysicsSettings.mPenetrationSlop, mPhysicsSettings.mLinearCastMaxPenetration * inner_radius));

						update_position = false;
					}
				}
				break;
			}

			if (update_position)
			{
				// Move the body now
				body.AddPositionStep(delta_pos);

				// If the body was activated due to an earlier CCD step it will have an index in the active
				// body list that it higher than the highest one we processed during FindCollisions
				// which means it hasn't been assigned an island and will not be updated by an island
				// this means that we need to update its bounds manually
				if (mp->GetIndexInActiveBodiesInternal() >= num_active_bodies_after_find_collisions)
				{
					body.CalculateWorldSpaceBoundsInternal();
					bodies_to_update_bounds[num_bodies_to_update_bounds++] = body.GetID();
					if (num_bodies_to_update_bounds == cBodiesBatch)
					{
						// Buffer full, flush now
						mBroadPhase->NotifyBodiesAABBChanged(bodies_to_update_bounds, num_bodies_to_update_bounds);
						num_bodies_to_update_bounds = 0;
					}
				}

				// We did not create a CCD body
				ioSubStep->mActiveBodyToCCDBody[active_body_idx] = -1;
			}

			active_body_idx++;
		}
	}

	// Notify change bounds on requested bodies
	if (num_bodies_to_update_bounds > 0)
		mBroadPhase->NotifyBodiesAABBChanged(bodies_to_update_bounds, num_bodies_to_update_bounds, false);
}

void PhysicsSystem::JobPostIntegrateVelocity(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep) const
{
	// Validate that our reservations were correct
	JPH_ASSERT(ioSubStep->mNumCCDBodies <= mBodyManager.GetNumActiveCCDBodies());

	if (ioSubStep->mNumCCDBodies == 0)
	{
		// No continous collision detection jobs -> kick the next job ourselves
		if (ioSubStep->mIsLast)
			ioSubStep->mStep->mContactRemovedCallbacks.RemoveDependency();
	}
	else
	{
		// Run the continous collision detection jobs
		int num_continuous_collision_jobs = min(int(ioSubStep->mNumCCDBodies + cNumCCDBodiesPerJob - 1) / cNumCCDBodiesPerJob, ioContext->GetMaxConcurrency());
		ioSubStep->mResolveCCDContacts.AddDependency(num_continuous_collision_jobs);
		if (ioSubStep->mIsLast)
			ioSubStep->mStep->mContactRemovedCallbacks.AddDependency(num_continuous_collision_jobs - 1); // Already had 1 dependency
		for (int i = 0; i < num_continuous_collision_jobs; ++i)
		{
			JobHandle job = ioContext->mJobSystem->CreateJob("FindCCDContacts", cColorFindCCDContacts, [ioContext, ioSubStep]() 
			{
				ioContext->mPhysicsSystem->JobFindCCDContacts(ioContext, ioSubStep);

				ioSubStep->mResolveCCDContacts.RemoveDependency();
				if (ioSubStep->mIsLast)
					ioSubStep->mStep->mContactRemovedCallbacks.RemoveDependency();
			});
			ioContext->mBarrier->AddJob(job);
		}
	}
}

// Helper function to calculate the motion of a body during this CCD step
inline static Vec3 sCalculateBodyMotion(const Body &inBody, float inDeltaTime)
{
	// If the body is linear casting, the body has not yet moved so we need to calculate its motion
	if (inBody.IsDynamic() && inBody.GetMotionProperties()->GetMotionQuality() == EMotionQuality::LinearCast)
		return inDeltaTime * inBody.GetLinearVelocity();

	// Body has already moved, so we don't need to correct for anything
	return Vec3::sZero();
}

// Helper function that finds the CCD body corresponding to a body (if it exists)
inline static PhysicsUpdateContext::SubStep::CCDBody *sGetCCDBody(const Body &inBody, PhysicsUpdateContext::SubStep *inSubStep)
{
	// If the body has no motion properties it cannot have a CCD body
	const MotionProperties *motion_properties = inBody.GetMotionPropertiesUnchecked();
	if (motion_properties == nullptr)
		return nullptr;

	// If it is not active it cannot have a CCD body
	uint32 active_index = motion_properties->GetIndexInActiveBodiesInternal();
	if (active_index == Body::cInactiveIndex)
		return nullptr;

	// Check if the active body has a corresponding CCD body
	JPH_ASSERT(active_index < inSubStep->mNumActiveBodyToCCDBody); // Ensure that the body has a mapping to CCD body
	int ccd_index = inSubStep->mActiveBodyToCCDBody[active_index];
	if (ccd_index < 0)
		return nullptr;

	PhysicsUpdateContext::SubStep::CCDBody *ccd_body = &inSubStep->mCCDBodies[ccd_index];
	JPH_ASSERT(ccd_body->mBodyID1 == inBody.GetID(), "We found the wrong CCD body!");
	return ccd_body;
}

void PhysicsSystem::JobFindCCDContacts(const PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We only read positions, but the validate callback may read body positions and velocities
	BodyAccess::Grant grant(BodyAccess::EAccess::Read, BodyAccess::EAccess::Read);
#endif

	// Allocation context for allocating new contact points
	ContactAllocator contact_allocator(mContactManager.GetContactAllocator());

	// Settings
	ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
	settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;
	settings.mReturnDeepestPoint = true;
	settings.mCollectFacesMode = ECollectFacesMode::CollectFaces;
	settings.mActiveEdgeMode = mPhysicsSettings.mCheckActiveEdges? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;
										
	for (;;)
	{
		// Fetch the next body to cast
		uint32 idx = ioSubStep->mNextCCDBody++;
		if (idx >= ioSubStep->mNumCCDBodies)
			break;
		CCDBody &ccd_body = ioSubStep->mCCDBodies[idx];
		const Body &body = mBodyManager.GetBody(ccd_body.mBodyID1);

		// Filter out layers
		DefaultBroadPhaseLayerFilter broadphase_layer_filter = GetDefaultBroadPhaseLayerFilter(body.GetObjectLayer());
		DefaultObjectLayerFilter object_layer_filter = GetDefaultLayerFilter(body.GetObjectLayer());
				
	#ifdef JPH_DEBUG_RENDERER
		// Draw start and end shape of cast
		if (sDrawMotionQualityLinearCast)
		{
			Mat44 com = body.GetCenterOfMassTransform();
			body.GetShape()->Draw(DebugRenderer::sInstance, com, Vec3::sReplicate(1.0f), Color::sGreen, false, true);
			DebugRenderer::sInstance->DrawArrow(com.GetTranslation(), com.GetTranslation() + ccd_body.mDeltaPosition, Color::sGreen, 0.1f);
			body.GetShape()->Draw(DebugRenderer::sInstance, Mat44::sTranslation(ccd_body.mDeltaPosition) * com, Vec3::sReplicate(1.0f), Color::sRed, false, true);
		}
	#endif // JPH_DEBUG_RENDERER

		// Create a collector that will find the maximum distance allowed to travel while not penetrating more than 'max penetration'
		class CCDNarrowPhaseCollector : public CastShapeCollector
		{
		public:
										CCDNarrowPhaseCollector(const BodyManager &inBodyManager, ContactConstraintManager &inContactConstraintManager, CCDBody &inCCDBody, ShapeCastResult &inResult, float inDeltaTime) : 
				mBodyManager(inBodyManager),
				mContactConstraintManager(inContactConstraintManager),
				mCCDBody(inCCDBody),
				mResult(inResult),
				mDeltaTime(inDeltaTime)
			{ 
			}

			virtual void				AddHit(const ShapeCastResult &inResult) override
			{
				JPH_PROFILE_FUNCTION();

				// Check if this is a possible earlier hit than the one before
				float fraction = inResult.mFraction;
				if (fraction < mCCDBody.mFractionPlusSlop)
				{
					// Normalize normal
					Vec3 normal = inResult.mPenetrationAxis.Normalized();

					// Calculate how much we can add to the fraction to penetrate the collision point by mMaxPenetration.
					// Note that the normal is pointing towards body 2!
					// Let the extra distance that we can travel along delta_pos be 'dist': mMaxPenetration / dist = cos(angle between normal and delta_pos) = normal . delta_pos / |delta_pos|
					// <=> dist = mMaxPenetration * |delta_pos| / normal . delta_pos
					// Converting to a faction: delta_fraction = dist / |delta_pos| = mLinearCastTreshold / normal . delta_pos
					float denominator = normal.Dot(mCCDBody.mDeltaPosition);
					if (denominator > mCCDBody.mMaxPenetration) // Avoid dividing by zero, if extra hit fraction > 1 there's also no point in continuing
					{
						float fraction_plus_slop = fraction + mCCDBody.mMaxPenetration / denominator;
						if (fraction_plus_slop < mCCDBody.mFractionPlusSlop)
						{
							const Body &body2 = mBodyManager.GetBody(inResult.mBodyID2);

							// Check if we've already accepted all hits from this body
							if (mValidateBodyPair)
							{
								// Validate the contact result
								ValidateResult validate_result = mContactConstraintManager.ValidateContactPoint(mBodyManager.GetBody(mCCDBody.mBodyID1), body2, inResult);
								switch (validate_result)
								{
								case ValidateResult::AcceptContact:
									// Just continue
									break;

								case ValidateResult::AcceptAllContactsForThisBodyPair:
									// Accept this and all following contacts from this body
									mValidateBodyPair = false;
									break;

								case ValidateResult::RejectContact:
									return;

								case ValidateResult::RejectAllContactsForThisBodyPair:
									// Reject this and all following contacts from this body
									mRejectAll = true;
									ForceEarlyOut();
									return;
								}
							}

							// This is the earliest hit so far, store it
							mCCDBody.mContactNormal = normal;
							mCCDBody.mBodyID2 = inResult.mBodyID2;
							mCCDBody.mFraction = fraction;
							mCCDBody.mFractionPlusSlop = fraction_plus_slop;
							mResult = inResult;

							// Result was assuming body 2 is not moving, but it is, so we need to correct for it
							Vec3 movement2 = fraction * sCalculateBodyMotion(body2, mDeltaTime);
							if (!movement2.IsNearZero())
							{
								mResult.mContactPointOn1 += movement2;
								mResult.mContactPointOn2 += movement2;
								for (Vec3 &v : mResult.mShape1Face)
									v += movement2;
								for (Vec3 &v : mResult.mShape2Face)
									v += movement2;
							}

							// Update early out fraction
							CastShapeCollector::UpdateEarlyOutFraction(fraction_plus_slop);
						}
					}
				}
			}

			bool						mValidateBodyPair;				///< If we still have to call the ValidateContactPoint for this body pair
			bool						mRejectAll;						///< Reject all further contacts between this body pair

		private:
			const BodyManager &			mBodyManager;
			ContactConstraintManager &	mContactConstraintManager;
			CCDBody &					mCCDBody;
			ShapeCastResult &			mResult;
			float						mDeltaTime;
			BodyID						mAcceptedBodyID;
		};

		// Narrowphase collector
		ShapeCastResult cast_shape_result;
		CCDNarrowPhaseCollector np_collector(mBodyManager, mContactManager, ccd_body, cast_shape_result, ioContext->mSubStepDeltaTime);

		// This collector wraps the narrowphase collector and collects the closest hit
		class CCDBroadPhaseCollector : public CastShapeBodyCollector
		{
		public:
										CCDBroadPhaseCollector(const CCDBody &inCCDBody, const Body &inBody1, const ShapeCast &inShapeCast, ShapeCastSettings &inShapeCastSettings, CCDNarrowPhaseCollector &ioCollector, const BodyManager &inBodyManager, PhysicsUpdateContext::SubStep *inSubStep, float inDeltaTime) :
				mCCDBody(inCCDBody),
				mBody1(inBody1),
				mBody1Extent(inShapeCast.mShapeWorldBounds.GetExtent()),
				mShapeCast(inShapeCast),
				mShapeCastSettings(inShapeCastSettings),
				mCollector(ioCollector),
				mBodyManager(inBodyManager),
				mSubStep(inSubStep),
				mDeltaTime(inDeltaTime)
			{
			}

			virtual void				AddHit(const BroadPhaseCastResult &inResult) override
			{
				JPH_PROFILE_FUNCTION();

				JPH_ASSERT(inResult.mFraction <= GetEarlyOutFraction(), "This hit should not have been passed on to the collector");

				// Test if we're colliding with ourselves
				if (mBody1.GetID() == inResult.mBodyID)
					return;

				// Avoid treating duplicates, if both bodies are doing CCD then only consider collision if body ID < other body ID
				const Body &body2 = mBodyManager.GetBody(inResult.mBodyID);
				const CCDBody *ccd_body2 = sGetCCDBody(body2, mSubStep);
				if (ccd_body2 != nullptr && mCCDBody.mBodyID1 > ccd_body2->mBodyID1)
					return;

				// Test group filter
				if (!mBody1.GetCollisionGroup().CanCollide(body2.GetCollisionGroup()))
					return;

				// TODO: For now we ignore sensors
				if (body2.IsSensor())
					return;

				// Get relative movement of these two bodies
				Vec3 direction = mShapeCast.mDirection - sCalculateBodyMotion(body2, mDeltaTime);

				// Test if the remaining movement is less than our movement threshold
				if (direction.LengthSq() < mCCDBody.mLinearCastThresholdSq)
					return;

				// Get the bounds of 2, widen it by the extent of 1 and test a ray to see if it hits earlier than the current early out fraction
				AABox bounds = body2.GetWorldSpaceBounds();
				bounds.mMin -= mBody1Extent;
				bounds.mMax += mBody1Extent;
				float hit_fraction = RayAABox(mShapeCast.mCenterOfMassStart.GetTranslation(), RayInvDirection(direction), bounds.mMin, bounds.mMax);
				if (hit_fraction > max(FLT_MIN, GetEarlyOutFraction())) // If early out fraction <= 0, we have the possibility of finding a deeper hit so we need to clamp the early out fraction
					return;

				// Reset collector (this is a new body pair)
				mCollector.ResetEarlyOutFraction(GetEarlyOutFraction());
				mCollector.mValidateBodyPair = true;
				mCollector.mRejectAll = false;

				// Provide direction as hint for the active edges algorithm
				mShapeCastSettings.mActiveEdgeMovementDirection = direction;

				// Do narrow phase collision check
				ShapeCast relative_cast(mShapeCast.mShape, mShapeCast.mScale, mShapeCast.mCenterOfMassStart, direction, mShapeCast.mShapeWorldBounds);
				body2.GetTransformedShape().CastShape(relative_cast, mShapeCastSettings, mCollector);

				// Update early out fraction based on narrow phase collector
				if (!mCollector.mRejectAll)
					UpdateEarlyOutFraction(mCollector.GetEarlyOutFraction());
			}

			const CCDBody &				mCCDBody;
			const Body &				mBody1;
			Vec3						mBody1Extent;
			ShapeCast					mShapeCast;
			ShapeCastSettings &			mShapeCastSettings;
			CCDNarrowPhaseCollector &	mCollector;
			const BodyManager &			mBodyManager;
			PhysicsUpdateContext::SubStep *mSubStep;
			float						mDeltaTime;
		};

		// Check if we collide with any other body. Note that we use the non-locking interface as we know the broadphase cannot be modified at this point.
		ShapeCast shape_cast(body.GetShape(), Vec3::sReplicate(1.0f), body.GetCenterOfMassTransform(), ccd_body.mDeltaPosition);
		CCDBroadPhaseCollector bp_collector(ccd_body, body, shape_cast, settings, np_collector, mBodyManager, ioSubStep, ioContext->mSubStepDeltaTime);
		mBroadPhase->CastAABoxNoLock({ shape_cast.mShapeWorldBounds, shape_cast.mDirection }, bp_collector, broadphase_layer_filter, object_layer_filter);

		// Check if there was a hit
		if (ccd_body.mFractionPlusSlop < 1.0f)
		{
			const Body &body2 = mBodyManager.GetBody(ccd_body.mBodyID2);

			// Determine contact manifold
			ContactManifold manifold;
			ManifoldBetweenTwoFaces(cast_shape_result.mContactPointOn1, cast_shape_result.mContactPointOn2, cast_shape_result.mPenetrationAxis, mPhysicsSettings.mManifoldToleranceSq, cast_shape_result.mShape1Face, cast_shape_result.mShape2Face, manifold.mWorldSpaceContactPointsOn1, manifold.mWorldSpaceContactPointsOn2);
			manifold.mSubShapeID1 = cast_shape_result.mSubShapeID1;
			manifold.mSubShapeID2 = cast_shape_result.mSubShapeID2;
			manifold.mPenetrationDepth = cast_shape_result.mPenetrationDepth;
			manifold.mWorldSpaceNormal = ccd_body.mContactNormal;

			// Call contact point callbacks
			mContactManager.OnCCDContactAdded(contact_allocator, body, body2, manifold, ccd_body.mContactSettings);

			// Calculate the average position from the manifold (this will result in the same impulse applied as when we apply impulses to all contact points)
			if (manifold.mWorldSpaceContactPointsOn2.size() > 1)
			{
				Vec3 average_contact_point = Vec3::sZero();
				for (const Vec3 &v : manifold.mWorldSpaceContactPointsOn2)
					average_contact_point += v;
				average_contact_point /= (float)manifold.mWorldSpaceContactPointsOn2.size();
				ccd_body.mContactPointOn2 = average_contact_point;
			}
			else
				ccd_body.mContactPointOn2 = cast_shape_result.mContactPointOn2;
		}
	}

	// Atomically accumulate the number of found manifolds and body pairs
	ioSubStep->mStep->mNumBodyPairs += contact_allocator.mNumBodyPairs;
	ioSubStep->mStep->mNumManifolds += contact_allocator.mNumManifolds;
}

void PhysicsSystem::JobResolveCCDContacts(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// Read/write body access
	BodyAccess::Grant grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::ReadWrite);

	// We activate bodies that we collide with
	BodyManager::GrantActiveBodiesAccess grant_active(true, false);
#endif

	uint32 num_active_bodies_after_find_collisions = ioSubStep->mStep->mActiveBodyReadIdx;
	TempAllocator *temp_allocator = ioContext->mTempAllocator;

	// Check if there's anything to do
	uint num_ccd_bodies = ioSubStep->mNumCCDBodies;
	if (num_ccd_bodies > 0)
	{
		// Sort on fraction so that we process earliest collisions first
		// This is needed to make the simulation deterministic and also to be able to stop contact processing
		// between body pairs if an earlier hit was found involving the body by another CCD body 
		// (if it's body ID < this CCD body's body ID - see filtering logic in CCDBroadPhaseCollector)
		CCDBody **sorted_ccd_bodies = (CCDBody **)temp_allocator->Allocate(num_ccd_bodies * sizeof(CCDBody *));
		{
			JPH_PROFILE("Sort");

			// We don't want to copy the entire struct (it's quite big), so we create a pointer array first
			CCDBody *src_ccd_bodies = ioSubStep->mCCDBodies;
			CCDBody **dst_ccd_bodies = sorted_ccd_bodies;
			CCDBody **dst_ccd_bodies_end = dst_ccd_bodies + num_ccd_bodies;
			while (dst_ccd_bodies < dst_ccd_bodies_end)
				*(dst_ccd_bodies++) = src_ccd_bodies++;

			// Which we then sort
			sort(sorted_ccd_bodies, sorted_ccd_bodies + num_ccd_bodies, [](const CCDBody *inBody1, const CCDBody *inBody2) 
				{ 
					if (inBody1->mFractionPlusSlop != inBody2->mFractionPlusSlop)
						return inBody1->mFractionPlusSlop < inBody2->mFractionPlusSlop;

					return inBody1->mBodyID1 < inBody2->mBodyID1;
				});
		}

		// We can collide with bodies that are not active, we track them here so we can activate them in one go at the end.
		// This is also needed because we can't modify the active body array while we iterate it.
		static constexpr int cBodiesBatch = 64;
		BodyID *bodies_to_activate = (BodyID *)JPH_STACK_ALLOC(cBodiesBatch * sizeof(BodyID));
		int num_bodies_to_activate = 0;

		// We can move bodies that are not part of an island. In this case we need to notify the broadphase of the movement.
		BodyID *bodies_to_update_bounds = (BodyID *)JPH_STACK_ALLOC(cBodiesBatch * sizeof(BodyID));
		int num_bodies_to_update_bounds = 0;

		for (uint i = 0; i < num_ccd_bodies; ++i)
		{
			const CCDBody *ccd_body = sorted_ccd_bodies[i];
			Body &body1 = mBodyManager.GetBody(ccd_body->mBodyID1);
			MotionProperties *body_mp = body1.GetMotionProperties();

			// If there was a hit
			if (!ccd_body->mBodyID2.IsInvalid())
			{
				Body &body2 = mBodyManager.GetBody(ccd_body->mBodyID2);

				// Determine if the other body has a CCD body
				CCDBody *ccd_body2 = sGetCCDBody(body2, ioSubStep);
				if (ccd_body2 != nullptr)
				{
					JPH_ASSERT(ccd_body2->mBodyID2 != ccd_body->mBodyID1, "If we collided with another body, that other body should have ignored collisions with us!");

					// Check if the other body found a hit that is further away
					if (ccd_body2->mFraction > ccd_body->mFraction)
					{
						// Reset the colliding body of the other CCD body. The other body will shorten its distance travelled and will not do any collision response (we'll do that).
						// This means that at this point we have triggered a contact point add/persist for our further hit by accident for the other body.
						// We accept this as calling the contact point callbacks here would require persisting the manifolds up to this point and doing the callbacks single threaded.
						ccd_body2->mBodyID2 = BodyID();
						ccd_body2->mFractionPlusSlop = ccd_body->mFraction;
					}
				}

				// If the other body moved less than us before hitting something, we're not colliding with it so we again have triggered contact point add/persist callbacks by accident.
				// We'll just move to the collision position anyway (as that's the last position we know is good), but we won't do any collision response.
				if (ccd_body2 == nullptr || ccd_body2->mFraction >= ccd_body->mFraction)
				{
					// Calculate contact points relative to center of mass of both bodies
					Vec3 r1_plus_u = ccd_body->mContactPointOn2 - (body1.GetCenterOfMassPosition() + ccd_body->mFraction * ccd_body->mDeltaPosition);
					Vec3 r2 = ccd_body->mContactPointOn2 - body2.GetCenterOfMassPosition();

					// Calculate velocity of collision points
					Vec3 v1 = body1.GetPointVelocityCOM(r1_plus_u); 
					Vec3 v2 = body2.GetPointVelocityCOM(r2);
					Vec3 relative_velocity = v2 - v1;
					float normal_velocity = relative_velocity.Dot(ccd_body->mContactNormal);

					// Calculate velocity bias due to restitution
					float normal_velocity_bias;
					if (ccd_body->mContactSettings.mCombinedRestitution > 0.0f && normal_velocity < -mPhysicsSettings.mMinVelocityForRestitution)
						normal_velocity_bias = ccd_body->mContactSettings.mCombinedRestitution * normal_velocity;
					else
						normal_velocity_bias = 0.0f;

					// Solve contact constraint
					AxisConstraintPart contact_constraint;
					contact_constraint.CalculateConstraintProperties(ioContext->mSubStepDeltaTime, body1, r1_plus_u, body2, r2, ccd_body->mContactNormal, normal_velocity_bias);
					contact_constraint.SolveVelocityConstraint(body1, body2, ccd_body->mContactNormal, -FLT_MAX, FLT_MAX);

					// Apply friction
					if (ccd_body->mContactSettings.mCombinedFriction > 0.0f)
					{
						Vec3 tangent1 = ccd_body->mContactNormal.GetNormalizedPerpendicular();
						Vec3 tangent2 = ccd_body->mContactNormal.Cross(tangent1);

						float max_lambda_f = ccd_body->mContactSettings.mCombinedFriction * contact_constraint.GetTotalLambda();

						AxisConstraintPart friction1;
						friction1.CalculateConstraintProperties(ioContext->mSubStepDeltaTime, body1, r1_plus_u, body2, r2, tangent1, 0.0f);
						friction1.SolveVelocityConstraint(body1, body2, tangent1, -max_lambda_f, max_lambda_f);

						AxisConstraintPart friction2;
						friction2.CalculateConstraintProperties(ioContext->mSubStepDeltaTime, body1, r1_plus_u, body2, r2, tangent2, 0.0f);
						friction2.SolveVelocityConstraint(body1, body2, tangent2, -max_lambda_f, max_lambda_f);
					}

					// Clamp velocities
					body_mp->ClampLinearVelocity();
					body_mp->ClampAngularVelocity();

					if (body2.IsDynamic())
					{
						MotionProperties *body2_mp = body2.GetMotionProperties();
						body2_mp->ClampLinearVelocity();
						body2_mp->ClampAngularVelocity();

						// Activate the body if it is not already active
						if (!body2.IsActive())
						{
							bodies_to_activate[num_bodies_to_activate++] = ccd_body->mBodyID2;
							if (num_bodies_to_activate == cBodiesBatch)
							{
								// Batch is full, activate now
								mBodyManager.ActivateBodies(bodies_to_activate, num_bodies_to_activate);
								num_bodies_to_activate = 0;
							}
						}
					}

				#ifdef JPH_DEBUG_RENDERER
					if (sDrawMotionQualityLinearCast)
					{
						// Draw the collision location
						Mat44 collision_transform = Mat44::sTranslation(ccd_body->mFraction * ccd_body->mDeltaPosition) * body1.GetCenterOfMassTransform();
						body1.GetShape()->Draw(DebugRenderer::sInstance, collision_transform, Vec3::sReplicate(1.0f), Color::sYellow, false, true);

						// Draw the collision location + slop
						Mat44 collision_transform_plus_slop = Mat44::sTranslation(ccd_body->mFractionPlusSlop * ccd_body->mDeltaPosition) * body1.GetCenterOfMassTransform();
						body1.GetShape()->Draw(DebugRenderer::sInstance, collision_transform_plus_slop, Vec3::sReplicate(1.0f), Color::sOrange, false, true);

						// Draw contact normal
						DebugRenderer::sInstance->DrawArrow(ccd_body->mContactPointOn2, ccd_body->mContactPointOn2 - ccd_body->mContactNormal, Color::sYellow, 0.1f);

						// Draw post contact velocity
						DebugRenderer::sInstance->DrawArrow(collision_transform.GetTranslation(), collision_transform.GetTranslation() + body1.GetLinearVelocity(), Color::sOrange, 0.1f);
						DebugRenderer::sInstance->DrawArrow(collision_transform.GetTranslation(), collision_transform.GetTranslation() + body1.GetAngularVelocity(), Color::sPurple, 0.1f);
					}
				#endif // JPH_DEBUG_RENDERER
				}
			}

			// Update body position
			body1.AddPositionStep(ccd_body->mDeltaPosition * ccd_body->mFractionPlusSlop);

			// If the body was activated due to an earlier CCD step it will have an index in the active
			// body list that it higher than the highest one we processed during FindCollisions
			// which means it hasn't been assigned an island and will not be updated by an island
			// this means that we need to update its bounds manually
			if (body_mp->GetIndexInActiveBodiesInternal() >= num_active_bodies_after_find_collisions)
			{
				body1.CalculateWorldSpaceBoundsInternal();
				bodies_to_update_bounds[num_bodies_to_update_bounds++] = body1.GetID();
				if (num_bodies_to_update_bounds == cBodiesBatch)
				{
					// Buffer full, flush now
					mBroadPhase->NotifyBodiesAABBChanged(bodies_to_update_bounds, num_bodies_to_update_bounds);
					num_bodies_to_update_bounds = 0;
				}
			}
		}

		// Activate the requested bodies
		if (num_bodies_to_activate > 0)
			mBodyManager.ActivateBodies(bodies_to_activate, num_bodies_to_activate);

		// Notify change bounds on requested bodies
		if (num_bodies_to_update_bounds > 0)
			mBroadPhase->NotifyBodiesAABBChanged(bodies_to_update_bounds, num_bodies_to_update_bounds, false);

		// Free the sorted ccd bodies
		temp_allocator->Free(sorted_ccd_bodies, num_ccd_bodies * sizeof(CCDBody *));
	}

	// Ensure we free the CCD bodies array now, will not call the destructor!
	temp_allocator->Free(ioSubStep->mActiveBodyToCCDBody, ioSubStep->mNumActiveBodyToCCDBody * sizeof(int));
	ioSubStep->mActiveBodyToCCDBody = nullptr;
	ioSubStep->mNumActiveBodyToCCDBody = 0;
	temp_allocator->Free(ioSubStep->mCCDBodies, ioSubStep->mCCDBodiesCapacity * sizeof(CCDBody));
	ioSubStep->mCCDBodies = nullptr;
	ioSubStep->mCCDBodiesCapacity = 0;
}

void PhysicsSystem::JobContactRemovedCallbacks(const PhysicsUpdateContext::Step *ioStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We don't touch any bodies
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
#endif

	// Reset the Body::EFlags::InvalidateContactCache flag for all bodies
	mBodyManager.ValidateContactCacheForAllBodies();

	// Trigger all contact removed callbacks by looking at last step contact points that have not been flagged as reused
	mContactManager.ContactPointRemovedCallbacks();

	// Finalize the contact cache (this swaps the read and write versions of the contact cache)
	mContactManager.FinalizeContactCache(ioStep->mNumBodyPairs, ioStep->mNumManifolds);
}

void PhysicsSystem::JobSolvePositionConstraints(PhysicsUpdateContext *ioContext, PhysicsUpdateContext::SubStep *ioSubStep)
{
#ifdef JPH_ENABLE_ASSERTS
	// We fix up position errors
	BodyAccess::Grant grant(BodyAccess::EAccess::None, BodyAccess::EAccess::ReadWrite);

	// Can only deactivate bodies
	BodyManager::GrantActiveBodiesAccess grant_active(false, true);
#endif

	float delta_time = ioContext->mSubStepDeltaTime;
	Constraint **active_constraints = ioContext->mActiveConstraints;

	for (;;)
	{
		// Next island
		uint32 island_idx = ioSubStep->mSolvePositionConstraintsNextIsland++;
		if (island_idx >= mIslandBuilder.GetNumIslands())
			break;

		JPH_PROFILE("Island");

		// Get iterators for this island
		BodyID *bodies_begin, *bodies_end;
		mIslandBuilder.GetBodiesInIsland(island_idx, bodies_begin, bodies_end);
		uint32 *constraints_begin, *constraints_end;
		bool has_constraints = mIslandBuilder.GetConstraintsInIsland(island_idx, constraints_begin, constraints_end);
		uint32 *contacts_begin, *contacts_end;
		bool has_contacts = mIslandBuilder.GetContactsInIsland(island_idx, contacts_begin, contacts_end);

		// Correct positions
		if (has_contacts || has_constraints)
		{
			float baumgarte = mPhysicsSettings.mBaumgarte;
			for (int position_step = 0; position_step < mPhysicsSettings.mNumPositionSteps; ++position_step)
			{
				bool constraint_impulse = ConstraintManager::sSolvePositionConstraints(active_constraints, constraints_begin, constraints_end, delta_time, baumgarte);
				bool contact_impulse = mContactManager.SolvePositionConstraints(contacts_begin, contacts_end);
				if (!constraint_impulse && !contact_impulse)
					break;
			}
		}

		// Only check sleeping in the last sub step of the last step
		// Also resets force and torque used during the apply gravity phase
		if (ioSubStep->mIsLastOfAll)
		{
			JPH_PROFILE("Check Sleeping");

			static_assert(int(Body::ECanSleep::CannotSleep) == 0 && int(Body::ECanSleep::CanSleep) == 1, "Loop below makes this assumption");
			int all_can_sleep = mPhysicsSettings.mAllowSleeping? int(Body::ECanSleep::CanSleep) : int(Body::ECanSleep::CannotSleep);

			float time_before_sleep = mPhysicsSettings.mTimeBeforeSleep;
			float max_movement = mPhysicsSettings.mPointVelocitySleepThreshold * time_before_sleep;

			for (const BodyID *body_id = bodies_begin; body_id < bodies_end; ++body_id)
			{
				Body &body = mBodyManager.GetBody(*body_id);

				// Update bounding box
				body.CalculateWorldSpaceBoundsInternal();

				// Update sleeping
				all_can_sleep &= int(body.UpdateSleepStateInternal(ioContext->mSubStepDeltaTime, max_movement, time_before_sleep));

				// Reset force and torque
				body.GetMotionProperties()->ResetForceAndTorqueInternal();
			}

			// If all bodies indicate they can sleep we can deactivate them
			if (all_can_sleep == int(Body::ECanSleep::CanSleep))
				mBodyManager.DeactivateBodies(bodies_begin, int(bodies_end - bodies_begin));
		}
		else
		{
			JPH_PROFILE("Update Bounds");

			// Update bounding box only for all other sub steps
			for (const BodyID *body_id = bodies_begin; body_id < bodies_end; ++body_id)
			{
				Body &body = mBodyManager.GetBody(*body_id);
				body.CalculateWorldSpaceBoundsInternal();
			}
		}

		// Notify broadphase of changed objects (find ccd contacts can do linear casts in the next step, so
		// we need to do this every sub step)
		// Note: Shuffles the BodyID's around!!!
		mBroadPhase->NotifyBodiesAABBChanged(bodies_begin, int(bodies_end - bodies_begin), false);
	}
}

void PhysicsSystem::SaveState(StateRecorder &inStream) const
{
	JPH_PROFILE_FUNCTION();

	inStream.Write(mPreviousSubStepDeltaTime);
	inStream.Write(mGravity);

	mBodyManager.SaveState(inStream);

	mContactManager.SaveState(inStream);

	mConstraintManager.SaveState(inStream);
}

bool PhysicsSystem::RestoreState(StateRecorder &inStream)
{
	JPH_PROFILE_FUNCTION();

	inStream.Read(mPreviousSubStepDeltaTime);
	inStream.Read(mGravity);

	if (!mBodyManager.RestoreState(inStream))
		return false;

	if (!mContactManager.RestoreState(inStream))
		return false;

	if (!mConstraintManager.RestoreState(inStream))
		return false;

	// Update bounding boxes for all bodies in the broadphase
	vector<BodyID> bodies;
	for (const Body *b : mBodyManager.GetBodies())
		if (BodyManager::sIsValidBodyPointer(b) && b->IsInBroadPhase())
			bodies.push_back(b->GetID());
	if (!bodies.empty())
		mBroadPhase->NotifyBodiesAABBChanged(&bodies[0], (int)bodies.size());

	return true;
}

JPH_NAMESPACE_END

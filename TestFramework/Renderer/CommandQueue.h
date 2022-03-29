// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/FatalErrorIfFailed.h>

/// Holds a number of DirectX operations with logic to wait for completion
class CommandQueue
{
public:
	/// Destructor
                                        ~CommandQueue()
    {
        WaitUntilFinished();

        if (mFenceEvent != INVALID_HANDLE_VALUE)
            CloseHandle(mFenceEvent);
    }

    /// Initialize the queue
    void                                Initialize(ID3D12Device *inDevice)
    {
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        FatalErrorIfFailed(inDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&mCommandQueue)));

        FatalErrorIfFailed(inDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));

        // Create the command list
        FatalErrorIfFailed(inDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

        // Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it now
        FatalErrorIfFailed(mCommandList->Close());

        // Create synchronization object
        FatalErrorIfFailed(inDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

		// Increment fence value so we don't skip waiting the first time a command list is executed
		mFenceValue++;

        // Create an event handle to use for frame synchronization
        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (mFenceEvent == nullptr)
            FatalErrorIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    /// Start the command list (requires waiting until the previous one is finished)
    ID3D12GraphicsCommandList *         Start()
    {
	    // Reset the allocator
        FatalErrorIfFailed(mCommandAllocator->Reset());

	    // Reset the command list
        FatalErrorIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	    return mCommandList.Get();
    }

    /// Execute accumulated command list
    void                                Execute()
    {
        JPH_ASSERT(!mIsExecuting);

	    // Close the command list
        FatalErrorIfFailed(mCommandList->Close());

	    // Execute the command list
        ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Schedule a Signal command in the queue
        FatalErrorIfFailed(mCommandQueue->Signal(mFence.Get(), mFenceValue));

        // Mark that we're executing
        mIsExecuting = true;
    }

    /// After executing, this waits until execution is done
    void                                WaitUntilFinished()
    {
        // Check if we've been started
        if (mIsExecuting)
        {
            if (mFence->GetCompletedValue() < mFenceValue)
            {
                // Wait until the fence has been processed
                FatalErrorIfFailed(mFence->SetEventOnCompletion(mFenceValue, mFenceEvent));
                WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
            }

			// Increment the fence value
            mFenceValue++;

            // Done executing
            mIsExecuting = false;
        }
    }

    /// Execute and wait for the command list to finish
    void                                ExecuteAndWait()
    {
        Execute();
        WaitUntilFinished();
    }

private:
    ComPtr<ID3D12CommandQueue>		    mCommandQueue;								///< The command queue that will hold command lists
    ComPtr<ID3D12CommandAllocator>	    mCommandAllocator;							///< Allocator that holds the memory for the commands
    ComPtr<ID3D12GraphicsCommandList>   mCommandList;								///< The command list that will hold the render commands / state changes
    HANDLE							    mFenceEvent = INVALID_HANDLE_VALUE;			///< Fence event, used to wait for rendering to complete
    ComPtr<ID3D12Fence>				    mFence;										///< Fence object, used to signal the fence event
	UINT64							    mFenceValue = 0;							///< Current fence value, each time we need to wait we will signal the fence with this value, wait for it and then increase the value
    bool                                mIsExecuting = false;						///< If a commandlist is currently executing on the queue
};

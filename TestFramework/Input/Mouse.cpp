// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Mouse.h>
#include <Renderer/Renderer.h>
#include <Jolt/Core/Profiler.h>

// Used libraries
#pragma comment ( lib, "dxguid.lib" )
#pragma comment ( lib, "dinput8.lib" )

Mouse::Mouse()
{
	Reset();
}

Mouse::~Mouse()
{
	Shutdown();
}

void 
Mouse::Reset()
{
	mDI = nullptr;
	mMouse = nullptr;
	mMousePos.x = 0;
	mMousePos.y = 0;

	ResetMouse();	
}

void Mouse::ResetMouse()
{
	memset(&mMouseState, 0, sizeof(mMouseState));
	memset(&mDOD, 0, sizeof(mDOD));
	mDODLength = 0;
	mTimeLeftButtonLastReleased = 0;
	mLeftButtonDoubleClicked = false;
}

bool Mouse::Initialize(Renderer *inRenderer)
#ifdef JPH_COMPILER_CLANG
	// DIPROP_BUFFERSIZE is a pointer to 1 which causes UBSan: runtime error: reference binding to misaligned address 0x000000000001
	__attribute__((no_sanitize("alignment")))
#endif
{
	// Store renderer
	mRenderer = inRenderer;

	// Create direct input interface
	if (FAILED(CoCreateInstance(CLSID_DirectInput8, nullptr, CLSCTX_INPROC_SERVER, IID_IDirectInput8W, (void **)&mDI)))
	{
		Trace("Unable to create DirectInput interface, DirectX 8.0 is required");
		return false;
	}
	
	// Initialize direct input interface
	if (FAILED(mDI->Initialize((HINSTANCE)GetModuleHandle(nullptr), DIRECTINPUT_VERSION)))
	{
		Trace("Unable to initialize DirectInput interface, DirectX 8.0 is required");
		return false;
	}

	// Create Mouse device
	if (FAILED(mDI->CreateDevice(GUID_SysMouse, &mMouse, nullptr)))
	{
		Trace("Unable to get DirectInputDevice interface, DirectX 8.0 is required");
		return false;
	}
	
	// Set cooperative level for Mouse
	SetExclusive(false);
	
	// Set data format
	if (FAILED(mMouse->SetDataFormat(&c_dfDIMouse)))
	{
		Trace("Unable to set data format to mouse");
		return false;
	}

	// Create a mouse buffer
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = BUFFERSIZE;
    if (FAILED(mMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		Trace("Unable to set mouse buffer size");
		return false;
	}

	return true;
}

void Mouse::Shutdown()
{
	if (mMouse) 
	{
		mMouse->Unacquire();
		mMouse = nullptr;
	}

	mDI = nullptr;

	Reset();
}

void Mouse::Poll()
{
	JPH_PROFILE_FUNCTION();

	// Get mouse position using the standard window call
	if (!GetCursorPos(&mMousePos))
	{
		ResetMouse();
		return;
	}

	// Convert to window space
	if (!ScreenToClient(mRenderer->GetWindowHandle(), &mMousePos))
	{
		ResetMouse();
		return;
	}
 
	// Get relative movement
	if (FAILED(mMouse->GetDeviceState(sizeof(mMouseState), &mMouseState)))
	{
		// Mouse input was lost, reacquire		
		mMouse->Acquire();
		
		if (FAILED(mMouse->GetDeviceState(sizeof(mMouseState), &mMouseState)))
		{
			ResetMouse();
			return;
		}
	}
	
	// Get the state in a buffer for checking doubleclicks	
	if (FAILED(mMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), mDOD, &mDODLength, 0)))
	{
		// We lost mMouse input, reacquire		
		mMouse->Acquire();
		
		if (FAILED(mMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), mDOD, &mDODLength, 0)))
		{
			// Unable to reacquire, reset button info			
			mTimeLeftButtonLastReleased = 0;
			mLeftButtonDoubleClicked = false;			
			return;
		}
	}
	
	// Check for double clicks
    for (DWORD d = 0; d < mDODLength; d++)
    {
		// Check if this means left button is pressed		
		if (mDOD[d].dwOfs == DIMOFS_BUTTON0)
		{
			if (mDOD[d].dwData & 0x80)
			{
				if (mDOD[d].dwTimeStamp - mTimeLeftButtonLastReleased <= DCLICKTIME)
				{
					// This is a double click				
					mTimeLeftButtonLastReleased = 0;
					mLeftButtonDoubleClicked = true;
				}
			}
			else // Remember last time button was released
				mTimeLeftButtonLastReleased = mDOD[d].dwTimeStamp;
		}
	}
}

void Mouse::HideCursor()
{
	::ShowCursor(false);
}

void Mouse::ShowCursor()
{
	::ShowCursor(true);
}

void Mouse::SetExclusive(bool inExclusive)
{
	// Set cooperative level for Mouse
	if (FAILED(mMouse->SetCooperativeLevel(mRenderer->GetWindowHandle(), (inExclusive? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND)))
		Trace("Failed to set cooperative level for mouse");
}

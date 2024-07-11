// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Mouse.h>
#include <Renderer/Renderer.h>
#include <Jolt/Core/Profiler.h>

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
	mMousePosInitialized = false;
	memset(&mDOD, 0, sizeof(mDOD));
	mDODLength = 0;
	mTimeLeftButtonLastReleased = 0;
	mLeftButtonDoubleClicked = false;

}

void Mouse::DetectParsecRunning()
{
	mIsParsecRunning = false;

	if (SC_HANDLE manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT))
	{
		if (SC_HANDLE service = OpenServiceA(manager, "Parsec", SERVICE_QUERY_STATUS))
		{
			SERVICE_STATUS status;
			if (QueryServiceStatus(service, &status))
			{
				mIsParsecRunning = status.dwCurrentState == SERVICE_RUNNING;
			}
			CloseServiceHandle(service);
		}
		CloseServiceHandle(manager);
	}
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

	// Check if the parsec service is running
	DetectParsecRunning();

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

	// Remember last position
	POINT old_mouse_pos = mMousePos;

	// Get mouse position using the standard window call
	if (!GetCursorPos(&mMousePos))
	{
		ResetMouse();
		return;
	}

	// If we lost mouse before, we need to reset the old mouse pos to the current one
	if (!mMousePosInitialized)
	{
		old_mouse_pos = mMousePos;
		mMousePosInitialized = true;
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

	// If we're connected through remote desktop or Parsec then GetDeviceState returns faulty data for lX and lY so we need to use a fallback
	if (GetSystemMetrics(SM_REMOTESESSION) || mIsParsecRunning)
	{
		// Just use the delta between the current and last mouse position.
		// Note that this has the disadvantage that you can no longer rotate any further if you're at the edge of the screen,
		// but unfortunately a RDP session doesn't allow capturing the mouse so there doesn't seem to be a workaround for this.
		mMouseState.lX = mMousePos.x - old_mouse_pos.x;
		mMouseState.lY = mMousePos.y - old_mouse_pos.y;
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

// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Win/MouseWin.h>
#include <Window/ApplicationWindowWin.h>
#include <Jolt/Core/Profiler.h>

MouseWin::MouseWin()
{
	Reset();
}

MouseWin::~MouseWin()
{
	Shutdown();
}

void MouseWin::Reset()
{
	mDI = nullptr;
	mMouse = nullptr;
	mMousePos.x = 0;
	mMousePos.y = 0;

	ResetMouse();
}

void MouseWin::ResetMouse()
{
	memset(&mMouseState, 0, sizeof(mMouseState));
	mMousePosInitialized = false;
}

void MouseWin::DetectParsecRunning()
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

bool MouseWin::Initialize(ApplicationWindow *inWindow)
#ifdef JPH_COMPILER_CLANG
	// DIPROP_BUFFERSIZE is a pointer to 1 which causes UBSan: runtime error: reference binding to misaligned address 0x000000000001
	__attribute__((no_sanitize("alignment")))
#endif
{
	// Store window
	mWindow = static_cast<ApplicationWindowWin *>(inWindow);

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
	if (FAILED(mMouse->SetCooperativeLevel(mWindow->GetWindowHandle(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		Trace("Failed to set cooperative level for mouse");

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

void MouseWin::Shutdown()
{
	if (mMouse)
	{
		mMouse->Unacquire();
		mMouse = nullptr;
	}

	mDI = nullptr;

	Reset();
}

void MouseWin::Poll()
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
	if (!ScreenToClient(mWindow->GetWindowHandle(), &mMousePos))
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
}

void MouseWin::HideCursor()
{
	::ShowCursor(false);
}

void MouseWin::ShowCursor()
{
	::ShowCursor(true);
}

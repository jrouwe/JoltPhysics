// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Keyboard.h>
#include <Renderer/Renderer.h>
#include <Jolt/Core/Profiler.h>

class Renderer;

Keyboard::Keyboard()
{
	Reset();
}

Keyboard::~Keyboard()
{
	Shutdown();
}

void Keyboard::Reset()
{
	mDI = nullptr;
	mKeyboard = nullptr;

	ResetKeyboard();
}

void Keyboard::ResetKeyboard()
{
	memset(&mKeyPressed, 0, sizeof(mKeyPressed));
	memset(&mTimeKeyLastReleased, 0, sizeof(mTimeKeyLastReleased));
	memset(&mKeyDoubleClicked, 0, sizeof(mKeyDoubleClicked));
	memset(&mDOD, 0, sizeof(mDOD));
	mDODLength = 0;
	mCurrentPosition = 0;
	GetKeyboardState(mCurrentWUIState);
	memcpy(mPreviousWUIState, mCurrentWUIState, sizeof(mPreviousWUIState));
}

bool Keyboard::Initialize(Renderer *inRenderer)
#ifdef JPH_COMPILER_CLANG
	// DIPROP_BUFFERSIZE is a pointer to 1 which causes UBSan: runtime error: reference binding to misaligned address 0x000000000001
	__attribute__((no_sanitize("alignment")))
#endif
{
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

	// Create keyboard device
	if (FAILED(mDI->CreateDevice(GUID_SysKeyboard, &mKeyboard, nullptr)))
	{
		Trace("Unable to get DirectInputDevice interface, DirectX 8.0 is required");
		return false;
	}

	// Set cooperative level for keyboard
	if (FAILED(mKeyboard->SetCooperativeLevel(inRenderer->GetWindowHandle(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
	{
		Trace("Unable to set cooperative level for keyboard");
		return false;
	}

	// Set data format
	if (FAILED(mKeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		Trace("Unable to set data format to keyboard");
		return false;
	}

	// Create a keyboard buffer
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = BUFFERSIZE;
	if (FAILED(mKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		Trace("Unable to set keyboard buffer size");
		return false;
	}

	// Get keyboard layout
	mKeyboardLayout = GetKeyboardLayout(0);

	return true;
}

void Keyboard::Shutdown()
{
	if (mKeyboard)
	{
		mKeyboard->Unacquire();
		mKeyboard = nullptr;
	}

	mDI = nullptr;

	Reset();
}

void Keyboard::Poll()
{
	JPH_PROFILE_FUNCTION();

	// Get the state of the keyboard
	if (FAILED(mKeyboard->GetDeviceState(sizeof(mKeyPressed), mKeyPressed)))
	{
		mKeyboard->Acquire();

		if (FAILED(mKeyboard->GetDeviceState(sizeof(mKeyPressed), mKeyPressed)))
		{
			ResetKeyboard();
			return;
		}
	}

	// Get the state in a buffer
	mDODLength = BUFFERSIZE;
	mCurrentPosition = 0;
	if (FAILED(mKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), mDOD, &mDODLength, 0)))
	{
		mKeyboard->Acquire();

		if (FAILED(mKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), mDOD, &mDODLength, 0)))
		{
			ResetKeyboard();
			return;
		}
	}

	// Check double clicks
	for (DWORD d = 0; d < mDODLength; d++)
	{
		// Check if this means a button is pressed
		if (mDOD[d].dwData & 0x80)
		{
			if (mDOD[d].dwTimeStamp - mTimeKeyLastReleased[mDOD[d].dwOfs] <= DCLICKTIME)
			{
				// This is a double click
				mTimeKeyLastReleased[mDOD[d].dwOfs] = 0;
				mKeyDoubleClicked[mDOD[d].dwOfs] = TRUE;
			}
		}
		else // Remember last time this key was released
			mTimeKeyLastReleased[mDOD[d].dwOfs] = mDOD[d].dwTimeStamp;
	}

	// Get Windows User Interface state, copy current state to previous state and get new current state
	memcpy(mPreviousWUIState, mCurrentWUIState, sizeof(mPreviousWUIState));
	GetKeyboardState(mCurrentWUIState);
}

int	Keyboard::GetFirstKey()
{
	mCurrentPosition = 0;
	memcpy(mTrackedWUIState, mPreviousWUIState, sizeof(mTrackedWUIState));

	return GetNextKey();
}

static void sPress(BYTE &ioValue)
{
	ioValue |= 0x80;
	ioValue ^= 0x01;
}

static void sRelease(BYTE &ioValue)
{
	ioValue &= 0x7f;
}

int
Keyboard::GetNextKey()
{
	while (mCurrentPosition < mDODLength)
	{
		// Get next key
		const DIDEVICEOBJECTDATA &current = mDOD[mCurrentPosition];
		mCurrentPosition++;

		// Check special keys and update current state accordingly
		if (current.dwData & 0x80)
			switch (current.dwOfs)
			{
			case DIK_LSHIFT:		sPress(mTrackedWUIState[VK_LSHIFT]);		sPress(mTrackedWUIState[VK_SHIFT]);			break;
			case DIK_RSHIFT:		sPress(mTrackedWUIState[VK_RSHIFT]);		sPress(mTrackedWUIState[VK_SHIFT]);			break;
			case DIK_LCONTROL:		sPress(mTrackedWUIState[VK_LCONTROL]);		sPress(mTrackedWUIState[VK_CONTROL]);		break;
			case DIK_RCONTROL:		sPress(mTrackedWUIState[VK_RCONTROL]);		sPress(mTrackedWUIState[VK_CONTROL]);		break;
			case DIK_LALT:			sPress(mTrackedWUIState[VK_LMENU]);			sPress(mTrackedWUIState[VK_MENU]);			break;
			case DIK_RALT:			sPress(mTrackedWUIState[VK_RMENU]);			sPress(mTrackedWUIState[VK_MENU]);			break;
			case DIK_CAPSLOCK:		sPress(mTrackedWUIState[VK_CAPITAL]);													break;
			}
		else
			switch (current.dwOfs)
			{
			case DIK_LSHIFT:		sRelease(mTrackedWUIState[VK_LSHIFT]);		sRelease(mTrackedWUIState[VK_SHIFT]);		break;
			case DIK_RSHIFT:		sRelease(mTrackedWUIState[VK_RSHIFT]);		sRelease(mTrackedWUIState[VK_SHIFT]);		break;
			case DIK_LCONTROL:		sRelease(mTrackedWUIState[VK_LCONTROL]);	sRelease(mTrackedWUIState[VK_CONTROL]);		break;
			case DIK_RCONTROL:		sRelease(mTrackedWUIState[VK_RCONTROL]);	sRelease(mTrackedWUIState[VK_CONTROL]);		break;
			case DIK_LALT:			sRelease(mTrackedWUIState[VK_LMENU]);		sRelease(mTrackedWUIState[VK_MENU]);		break;
			case DIK_RALT:			sRelease(mTrackedWUIState[VK_RMENU]);		sRelease(mTrackedWUIState[VK_MENU]);		break;
			case DIK_CAPSLOCK:		sRelease(mTrackedWUIState[VK_CAPITAL]);													break;
			}

		// Return it
		if (current.dwData & 0x80)
			return current.dwOfs;
	}

	return 0;
}

uint Keyboard::GetVKValue()
{
	JPH_ASSERT(mCurrentPosition > 0, "First call GetFirstKey() to get the first key, then you can convert it to a VK code");
	int key = mDOD[mCurrentPosition - 1].dwOfs;
	return MapVirtualKeyEx(key, 1, mKeyboardLayout);
}

char Keyboard::GetASCIIValue()
{
	WORD result;

	JPH_ASSERT(mCurrentPosition > 0, "First call GetFirstKey() to get the first key, then you can convert it to an ASCII code");
	int key = mDOD[mCurrentPosition - 1].dwOfs;
	if (ToAsciiEx(GetVKValue(), key, mTrackedWUIState, &result, 0, mKeyboardLayout) != 1)
		return 0;

	return (char)result;
}

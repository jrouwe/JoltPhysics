// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Win/KeyboardWin.h>
#include <Renderer/Renderer.h>
#include <Window/ApplicationWindowWin.h>
#include <Jolt/Core/Profiler.h>

KeyboardWin::KeyboardWin()
{
	Reset();
}

KeyboardWin::~KeyboardWin()
{
	Shutdown();
}

void KeyboardWin::Reset()
{
	mDI = nullptr;
	mKeyboard = nullptr;

	ResetKeyboard();
}

void KeyboardWin::ResetKeyboard()
{
	memset(&mKeyPressed, 0, sizeof(mKeyPressed));
	memset(&mDOD, 0, sizeof(mDOD));
	mDODLength = 0;
	mCurrentPosition = 0;
}

bool KeyboardWin::Initialize(ApplicationWindow *inWindow)
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
	if (FAILED(mKeyboard->SetCooperativeLevel(static_cast<ApplicationWindowWin *>(inWindow)->GetWindowHandle(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
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

	return true;
}

void KeyboardWin::Shutdown()
{
	if (mKeyboard)
	{
		mKeyboard->Unacquire();
		mKeyboard = nullptr;
	}

	mDI = nullptr;

	Reset();
}

void KeyboardWin::Poll()
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
}

EKey KeyboardWin::GetFirstKey()
{
	mCurrentPosition = 0;

	return GetNextKey();
}

EKey KeyboardWin::GetNextKey()
{
	while (mCurrentPosition < mDODLength)
	{
		// Get next key
		const DIDEVICEOBJECTDATA &current = mDOD[mCurrentPosition];
		mCurrentPosition++;

		// Return it
		if (current.dwData & 0x80)
			return ToKey(current.dwOfs);
	}

	return EKey::Invalid;
}

EKey KeyboardWin::ToKey(int inValue) const
{
	switch (inValue)
	{
	case DIK_A: return EKey::A;
	case DIK_B: return EKey::B;
	case DIK_C: return EKey::C;
	case DIK_D: return EKey::D;
	case DIK_E: return EKey::E;
	case DIK_F: return EKey::F;
	case DIK_G: return EKey::G;
	case DIK_H: return EKey::H;
	case DIK_I: return EKey::I;
	case DIK_J: return EKey::J;
	case DIK_K: return EKey::K;
	case DIK_L: return EKey::L;
	case DIK_M: return EKey::M;
	case DIK_N: return EKey::N;
	case DIK_O: return EKey::O;
	case DIK_P: return EKey::P;
	case DIK_Q: return EKey::Q;
	case DIK_R: return EKey::R;
	case DIK_S: return EKey::S;
	case DIK_T: return EKey::T;
	case DIK_U: return EKey::U;
	case DIK_V: return EKey::V;
	case DIK_W: return EKey::W;
	case DIK_X: return EKey::X;
	case DIK_Y: return EKey::Y;
	case DIK_Z: return EKey::Z;
	case DIK_0: return EKey::Num0;
	case DIK_1: return EKey::Num1;
	case DIK_2: return EKey::Num2;
	case DIK_3: return EKey::Num3;
	case DIK_4: return EKey::Num4;
	case DIK_5: return EKey::Num5;
	case DIK_6: return EKey::Num6;
	case DIK_7: return EKey::Num7;
	case DIK_8: return EKey::Num8;
	case DIK_9: return EKey::Num9;
	case DIK_SPACE: return EKey::Space;
	case DIK_COMMA: return EKey::Comma;
	case DIK_PERIOD: return EKey::Period;
	case DIK_ESCAPE: return EKey::Escape;
	case DIK_LSHIFT: return EKey::LShift;
	case DIK_RSHIFT: return EKey::RShift;
	case DIK_LCONTROL: return EKey::LControl;
	case DIK_RCONTROL: return EKey::RControl;
	case DIK_LALT: return EKey::LAlt;
	case DIK_RALT: return EKey::RAlt;
	case DIK_LEFT: return EKey::Left;
	case DIK_RIGHT: return EKey::Right;
	case DIK_UP: return EKey::Up;
	case DIK_DOWN: return EKey::Down;
	case DIK_RETURN: return EKey::Return;
	default: return EKey::Unknown;
	}
}

int KeyboardWin::FromKey(EKey inKey) const
{
	switch (inKey)
	{
	case EKey::A: return DIK_A;
	case EKey::B: return DIK_B;
	case EKey::C: return DIK_C;
	case EKey::D: return DIK_D;
	case EKey::E: return DIK_E;
	case EKey::F: return DIK_F;
	case EKey::G: return DIK_G;
	case EKey::H: return DIK_H;
	case EKey::I: return DIK_I;
	case EKey::J: return DIK_J;
	case EKey::K: return DIK_K;
	case EKey::L: return DIK_L;
	case EKey::M: return DIK_M;
	case EKey::N: return DIK_N;
	case EKey::O: return DIK_O;
	case EKey::P: return DIK_P;
	case EKey::Q: return DIK_Q;
	case EKey::R: return DIK_R;
	case EKey::S: return DIK_S;
	case EKey::T: return DIK_T;
	case EKey::U: return DIK_U;
	case EKey::V: return DIK_V;
	case EKey::W: return DIK_W;
	case EKey::X: return DIK_X;
	case EKey::Y: return DIK_Y;
	case EKey::Z: return DIK_Z;
	case EKey::Num0: return DIK_0;
	case EKey::Num1: return DIK_1;
	case EKey::Num2: return DIK_2;
	case EKey::Num3: return DIK_3;
	case EKey::Num4: return DIK_4;
	case EKey::Num5: return DIK_5;
	case EKey::Num6: return DIK_6;
	case EKey::Num7: return DIK_7;
	case EKey::Num8: return DIK_8;
	case EKey::Num9: return DIK_9;
	case EKey::Space: return DIK_SPACE;
	case EKey::Comma: return DIK_COMMA;
	case EKey::Period: return DIK_PERIOD;
	case EKey::Escape: return DIK_ESCAPE;
	case EKey::LShift: return DIK_LSHIFT;
	case EKey::RShift: return DIK_RSHIFT;
	case EKey::LControl: return DIK_LCONTROL;
	case EKey::RControl: return DIK_RCONTROL;
	case EKey::LAlt: return DIK_LALT;
	case EKey::RAlt: return DIK_RALT;
	case EKey::Left: return DIK_LEFT;
	case EKey::Right: return DIK_RIGHT;
	case EKey::Up: return DIK_UP;
	case EKey::Down: return DIK_DOWN;
	case EKey::Return: return DIK_RETURN;
	case EKey::Invalid:
	case EKey::Unknown:
	default:
		return 0;
	}
}

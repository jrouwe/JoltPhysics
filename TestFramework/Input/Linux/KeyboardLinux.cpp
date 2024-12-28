// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/Linux/KeyboardLinux.h>
#include <Renderer/Renderer.h>

KeyboardLinux::~KeyboardLinux()
{
	Shutdown();
}

bool KeyboardLinux::Initialize(Renderer *inRenderer)
{
	mRenderer = inRenderer;
	inRenderer->SetEventListener([this](const XEvent &inEvent) { HandleEvent(inEvent); });

	return true;
}

void KeyboardLinux::Shutdown()
{
	if (mRenderer != nullptr)
	{
		mRenderer->SetEventListener({});
		mRenderer = nullptr;
	}
}

void KeyboardLinux::Poll()
{
	// Reset the keys pressed
	memset(mKeysPressed, 0, sizeof(mKeysPressed));

	Display *display = mRenderer->GetDisplay();

	// Get pressed keys
	char keymap[32];
	XQueryKeymap(display, keymap);
	for (int i = 0; i < 32; ++i)
	{
		// Iterate 8 bits at a time
		uint keycode = i << 3;
		uint32 value = uint8(keymap[i]);
		while (value != 0)
		{
			// Get the next bit
			uint lz = CountTrailingZeros(value);
			keycode += lz;

			// Convert to key
			KeySym keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
			EKey key = ToKey(keysym);
			if (key != EKey::Unknown)
				mKeysPressed[(int)key] = true;

			// Skip this bit
			keycode++;
			value >>= lz + 1;
		}
	}

	// Make the pending buffer the active buffer
	mKeyBuffer = mPendingKeyBuffer;
	mPendingKeyBuffer.clear();
}

EKey KeyboardLinux::GetFirstKey()
{
	mCurrentKey = 0;

	return GetNextKey();
}

EKey KeyboardLinux::GetNextKey()
{
	if (mCurrentKey < mKeyBuffer.size())
		return mKeyBuffer[mCurrentKey++];
	return EKey::Invalid;
}

void KeyboardLinux::HandleEvent(const XEvent &inEvent)
{
	// If this is a key press event and the buffer is not yet full
	if (inEvent.type == KeyPress && mPendingKeyBuffer.size() < mPendingKeyBuffer.capacity())
	{
		// Convert to key
		KeySym keysym = XkbKeycodeToKeysym(mRenderer->GetDisplay(), inEvent.xkey.keycode, 0, 0);
		EKey key = ToKey(keysym);
		if (key != EKey::Unknown)
			mPendingKeyBuffer.push_back(key);
	}
}

EKey KeyboardLinux::ToKey(int inValue) const
{
	switch (inValue)
	{
	case XK_a: return EKey::A;
	case XK_b: return EKey::B;
	case XK_c: return EKey::C;
	case XK_d: return EKey::D;
	case XK_e: return EKey::E;
	case XK_f: return EKey::F;
	case XK_g: return EKey::G;
	case XK_h: return EKey::H;
	case XK_i: return EKey::I;
	case XK_j: return EKey::J;
	case XK_k: return EKey::K;
	case XK_l: return EKey::L;
	case XK_m: return EKey::M;
	case XK_n: return EKey::N;
	case XK_o: return EKey::O;
	case XK_p: return EKey::P;
	case XK_q: return EKey::Q;
	case XK_r: return EKey::R;
	case XK_s: return EKey::S;
	case XK_t: return EKey::T;
	case XK_u: return EKey::U;
	case XK_v: return EKey::V;
	case XK_w: return EKey::W;
	case XK_x: return EKey::X;
	case XK_y: return EKey::Y;
	case XK_z: return EKey::Z;
	case XK_0: return EKey::Num0;
	case XK_1: return EKey::Num1;
	case XK_2: return EKey::Num2;
	case XK_3: return EKey::Num3;
	case XK_4: return EKey::Num4;
	case XK_5: return EKey::Num5;
	case XK_6: return EKey::Num6;
	case XK_7: return EKey::Num7;
	case XK_8: return EKey::Num8;
	case XK_9: return EKey::Num9;
	case XK_space: return EKey::Space;
	case XK_comma: return EKey::Comma;
	case XK_period: return EKey::Period;
	case XK_Escape: return EKey::Escape;
	case XK_Shift_L: return EKey::LShift;
	case XK_Shift_R: return EKey::RShift;
	case XK_Control_L: return EKey::LControl;
	case XK_Control_R: return EKey::RControl;
	case XK_Alt_L: return EKey::LAlt;
	case XK_Alt_R: return EKey::RAlt;
	case XK_Left: return EKey::Left;
	case XK_Right: return EKey::Right;
	case XK_Up: return EKey::Up;
	case XK_Down: return EKey::Down;
	case XK_Return: return EKey::Return;
	default: return EKey::Unknown;
	}
}

int KeyboardLinux::FromKey(EKey inKey) const
{
	switch (inKey)
	{
	case EKey::A: return XK_a;
	case EKey::B: return XK_b;
	case EKey::C: return XK_c;
	case EKey::D: return XK_d;
	case EKey::E: return XK_e;
	case EKey::F: return XK_f;
	case EKey::G: return XK_g;
	case EKey::H: return XK_h;
	case EKey::I: return XK_i;
	case EKey::J: return XK_j;
	case EKey::K: return XK_k;
	case EKey::L: return XK_l;
	case EKey::M: return XK_m;
	case EKey::N: return XK_n;
	case EKey::O: return XK_o;
	case EKey::P: return XK_p;
	case EKey::Q: return XK_q;
	case EKey::R: return XK_r;
	case EKey::S: return XK_s;
	case EKey::T: return XK_t;
	case EKey::U: return XK_u;
	case EKey::V: return XK_v;
	case EKey::W: return XK_w;
	case EKey::X: return XK_x;
	case EKey::Y: return XK_y;
	case EKey::Z: return XK_z;
	case EKey::Num0: return XK_0;
	case EKey::Num1: return XK_1;
	case EKey::Num2: return XK_2;
	case EKey::Num3: return XK_3;
	case EKey::Num4: return XK_4;
	case EKey::Num5: return XK_5;
	case EKey::Num6: return XK_6;
	case EKey::Num7: return XK_7;
	case EKey::Num8: return XK_8;
	case EKey::Num9: return XK_9;
	case EKey::Space: return XK_space;
	case EKey::Comma: return XK_comma;
	case EKey::Period: return XK_period;
	case EKey::Escape: return XK_Escape;
	case EKey::LShift: return XK_Shift_L;
	case EKey::RShift: return XK_Shift_R;
	case EKey::LControl: return XK_Control_L;
	case EKey::RControl: return XK_Control_R;
	case EKey::LAlt: return XK_Alt_L;
	case EKey::RAlt: return XK_Alt_R;
	case EKey::Left: return XK_Left;
	case EKey::Right: return XK_Right;
	case EKey::Up: return XK_Up;
	case EKey::Down: return XK_Down;
	case EKey::Return: return XK_Return;
	case EKey::Invalid:
	case EKey::Unknown:
	default:
		return 0;
	}
}

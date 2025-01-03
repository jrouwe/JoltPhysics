// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class ApplicationWindow;

enum class EKey
{
	Invalid,
	Unknown,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Num0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Space,
	Comma,
	Period,
	Escape,
	LShift,
	RShift,
	LControl,
	RControl,
	LAlt,
	RAlt,
	Left,
	Right,
	Up,
	Down,
	Return,
	NumKeys,
};

/// Keyboard interface class which keeps track on the status of all keys and keeps track of the list of keys pressed.
class Keyboard
{
public:
	/// Constructor
									Keyboard() = default;
	virtual							~Keyboard() = default;

	/// Initialization / shutdown
	virtual bool					Initialize(ApplicationWindow *inWindow) = 0;
	virtual void					Shutdown() = 0;

	/// Update the keyboard state
	virtual void					Poll() = 0;

	/// Checks if a key is pressed or not
	virtual bool					IsKeyPressed(EKey inKey) const = 0;

	/// Checks if a key is pressed and was not pressed the last time this function was called (state is stored in ioPrevState)
	bool							IsKeyPressedAndTriggered(EKey inKey, bool &ioPrevState) const
	{
		bool prev_state = ioPrevState;
		ioPrevState = IsKeyPressed(inKey);
		return ioPrevState && !prev_state;
	}

	/// Buffered keyboard input, returns EKey::Invalid for none
	virtual EKey					GetFirstKey() = 0;
	virtual EKey					GetNextKey() = 0;
};

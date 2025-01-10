// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/KeyboardMacOS.h>
#import <GameController/GameController.h>

static EKey sToKey(GCKeyCode inValue)
{
	if (inValue == GCKeyCodeKeyA) return EKey::A;
	if (inValue == GCKeyCodeKeyB) return EKey::B;
	if (inValue == GCKeyCodeKeyC) return EKey::C;
	if (inValue == GCKeyCodeKeyD) return EKey::D;
	if (inValue == GCKeyCodeKeyE) return EKey::E;
	if (inValue == GCKeyCodeKeyF) return EKey::F;
	if (inValue == GCKeyCodeKeyG) return EKey::G;
	if (inValue == GCKeyCodeKeyH) return EKey::H;
	if (inValue == GCKeyCodeKeyI) return EKey::I;
	if (inValue == GCKeyCodeKeyJ) return EKey::J;
	if (inValue == GCKeyCodeKeyK) return EKey::K;
	if (inValue == GCKeyCodeKeyL) return EKey::L;
	if (inValue == GCKeyCodeKeyM) return EKey::M;
	if (inValue == GCKeyCodeKeyN) return EKey::N;
	if (inValue == GCKeyCodeKeyO) return EKey::O;
	if (inValue == GCKeyCodeKeyP) return EKey::P;
	if (inValue == GCKeyCodeKeyQ) return EKey::Q;
	if (inValue == GCKeyCodeKeyR) return EKey::R;
	if (inValue == GCKeyCodeKeyS) return EKey::S;
	if (inValue == GCKeyCodeKeyT) return EKey::T;
	if (inValue == GCKeyCodeKeyU) return EKey::U;
	if (inValue == GCKeyCodeKeyV) return EKey::V;
	if (inValue == GCKeyCodeKeyW) return EKey::W;
	if (inValue == GCKeyCodeKeyX) return EKey::X;
	if (inValue == GCKeyCodeKeyY) return EKey::Y;
	if (inValue == GCKeyCodeKeyZ) return EKey::Z;
	if (inValue == GCKeyCodeZero) return EKey::Num0;
	if (inValue == GCKeyCodeOne) return EKey::Num1;
	if (inValue == GCKeyCodeTwo) return EKey::Num2;
	if (inValue == GCKeyCodeThree) return EKey::Num3;
	if (inValue == GCKeyCodeFour) return EKey::Num4;
	if (inValue == GCKeyCodeFive) return EKey::Num5;
	if (inValue == GCKeyCodeSix) return EKey::Num6;
	if (inValue == GCKeyCodeSeven) return EKey::Num7;
	if (inValue == GCKeyCodeEight) return EKey::Num8;
	if (inValue == GCKeyCodeNine) return EKey::Num9;
	if (inValue == GCKeyCodeSpacebar) return EKey::Space;
	if (inValue == GCKeyCodeComma) return EKey::Comma;
	if (inValue == GCKeyCodePeriod) return EKey::Period;
	if (inValue == GCKeyCodeEscape) return EKey::Escape;
	if (inValue == GCKeyCodeLeftShift) return EKey::LShift;
	if (inValue == GCKeyCodeRightShift) return EKey::RShift;
	if (inValue == GCKeyCodeLeftControl) return EKey::LControl;
	if (inValue == GCKeyCodeRightControl) return EKey::RControl;
	if (inValue == GCKeyCodeLeftAlt) return EKey::LAlt;
	if (inValue == GCKeyCodeRightAlt) return EKey::RAlt;
	if (inValue == GCKeyCodeLeftArrow) return EKey::Left;
	if (inValue == GCKeyCodeRightArrow) return EKey::Right;
	if (inValue == GCKeyCodeUpArrow) return EKey::Up;
	if (inValue == GCKeyCodeDownArrow) return EKey::Down;
	if (inValue == GCKeyCodeReturnOrEnter) return EKey::Return;
	return EKey::Unknown;
}

// This class receives keyboard connect callbacks
@interface KeyboardDelegate : NSObject
@end

@implementation KeyboardDelegate
{
	KeyboardMacOS *mKeyboard;
}

- (KeyboardDelegate *)init:(KeyboardMacOS *)Keyboard
{
	mKeyboard = Keyboard;

	[NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent *(NSEvent *event) {
		// Ignore all keystrokes except Command-Q (Quit).
		if ((event.modifierFlags & NSEventModifierFlagCommand) && [event.charactersIgnoringModifiers isEqual:@"q"])
			return event;
		else
			return nil;
	}];

	return self;
}

- (void)keyboardDidConnect:(NSNotification *)notification
{
	GCKeyboard *keyboard = (GCKeyboard *)notification.object;
	if (!keyboard)
		return;

	__block KeyboardDelegate *weakSelf = self;
	keyboard.keyboardInput.keyChangedHandler = ^(GCKeyboardInput *keyboard, GCControllerButtonInput *key, GCKeyCode keyCode, BOOL pressed) {
		KeyboardDelegate *strongSelf = weakSelf;
		if (strongSelf == nil)
			return;

		EKey ekey = sToKey(keyCode);
		if (ekey != EKey::Invalid)
			strongSelf->mKeyboard->OnKeyPressed(ekey, pressed);
	};
}
	
@end

bool KeyboardMacOS::Initialize(ApplicationWindow *inWindow)
{
	KeyboardDelegate *delegate = [[KeyboardDelegate alloc] init: this];
	[NSNotificationCenter.defaultCenter addObserver: delegate selector: @selector(keyboardDidConnect:) name: GCKeyboardDidConnectNotification object: nil];
	return true;
}

void KeyboardMacOS::Poll()
{
	// Make the pending buffer the active buffer
	mKeyBuffer = mPendingKeyBuffer;
	mPendingKeyBuffer.clear();
}

EKey KeyboardMacOS::GetFirstKey()
{
	mCurrentKey = 0;
	return GetNextKey();
}

EKey KeyboardMacOS::GetNextKey()
{
	if (mCurrentKey < mKeyBuffer.size())
		return mKeyBuffer[mCurrentKey++];
	return EKey::Invalid;
}

void KeyboardMacOS::OnKeyPressed(EKey inKey, bool inPressed)
{
	if (inPressed && mPendingKeyBuffer.size() < mPendingKeyBuffer.capacity())
		mPendingKeyBuffer.push_back(inKey);
	
	mKeyPressed[(int)inKey] = inPressed;
}

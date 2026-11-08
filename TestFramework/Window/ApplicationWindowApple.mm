// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowApple.h>

#import <MetalKit/MetalKit.h>
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

ApplicationWindowApple::~ApplicationWindowApple()
{
	if (mKeyboardConnectObserver != nullptr)
	{
		[NSNotificationCenter.defaultCenter removeObserver: (id)mKeyboardConnectObserver];

		// Remove the handler as it references this window
		GCKeyboard.coalescedKeyboard.keyboardInput.keyChangedHandler = nil;
	}

	[mMetalView release];
}

void ApplicationWindowApple::SetMetalView(MTKView *inView)
{
	inView.clearColor = MTLClearColorMake(0.098f, 0.098f, 0.439f, 1.000f);
	inView.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
	inView.clearDepth = 0.0f;
	mMetalView = inView;
}

void ApplicationWindowApple::ListenForKeyboards()
{
	ApplicationWindowApple *window = this;
	mKeyboardConnectObserver = [NSNotificationCenter.defaultCenter addObserverForName: GCKeyboardDidConnectNotification object: nil queue: nil usingBlock: ^(NSNotification *notification) {
		GCKeyboardInput *keyboard_input = ((GCKeyboard *)notification.object).keyboardInput;
		if (keyboard_input == nil)
			return;

		keyboard_input.keyChangedHandler = ^(GCKeyboardInput *, GCControllerButtonInput *, GCKeyCode inKeyCode, BOOL inPressed) {
			EKey key = sToKey(inKeyCode);
			if (key != EKey::Invalid)
				window->OnKey(key, inPressed);
		};
	}];
}

CAMetalLayer *ApplicationWindowApple::GetMetalLayer() const
{
	return (CAMetalLayer *)mMetalView.layer;
}

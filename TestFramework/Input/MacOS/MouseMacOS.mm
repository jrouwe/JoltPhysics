// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/MouseMacOS.h>

#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>

// This class receives mouse connect callbacks
@interface MouseDelegate : NSObject
@end

@implementation MouseDelegate
{
	MouseMacOS *mMouse;
}

- (MouseDelegate *)init:(MouseMacOS *)mouse
{
	mMouse = mouse;
	return self;
}

- (void)mouseDidConnect:(NSNotification *)notification
{
	GCMouse *mouse = (GCMouse *)notification.object;
	if (mouse == nil)
		return;

	GCMouseInput *mouseInput = mouse.mouseInput;
	if (mouseInput == nil)
		return;

	__block MouseDelegate *weakSelf = self;
	mouseInput.mouseMovedHandler = ^(GCMouseInput *mouse, float deltaX, float deltaY) {
		MouseDelegate *strongSelf = weakSelf;
		if (strongSelf == nil)
			return;

		strongSelf->mMouse->OnMouseMoved(deltaX, -deltaY);
	};

	mouseInput.leftButton.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
		MouseDelegate *strongSelf = weakSelf;
		if (strongSelf == nil)
			return;

		strongSelf->mMouse->SetLeftPressed(pressed);
	};

	mouseInput.rightButton.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
		MouseDelegate *strongSelf = weakSelf;
		if (strongSelf == nil)
			return;

		strongSelf->mMouse->SetRightPressed(pressed);
	};

	mouseInput.middleButton.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
		MouseDelegate *strongSelf = weakSelf;
		if (strongSelf == nil)
			return;

		strongSelf->mMouse->SetMiddlePressed(pressed);
	};
}
	
@end

bool MouseMacOS::Initialize(ApplicationWindow *inWindow)
{
	MouseDelegate *delegate = [[MouseDelegate alloc] init: this];
	[NSNotificationCenter.defaultCenter addObserver: delegate selector: @selector(mouseDidConnect:) name: GCMouseDidConnectNotification object:nil];
	return true;
}

void MouseMacOS::Poll()
{
	mX += mDeltaXAcc;
	mY += mDeltaYAcc;
	mDeltaX = mDeltaXAcc;
	mDeltaY = mDeltaYAcc;
	
	mDeltaXAcc = 0;
	mDeltaYAcc = 0;
}

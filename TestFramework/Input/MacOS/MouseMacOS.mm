// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Input/MacOS/MouseMacOS.h>
#include <Window/ApplicationWindowMacOS.h>

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

		strongSelf->mMouse->OnMouseDelta(deltaX, -deltaY);
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
	mWindow = static_cast<ApplicationWindowMacOS *>(inWindow);

	// Install listener for mouse move callbacks
	mWindow->SetMouseMovedCallback([this](int inX, int inY) { OnMouseMoved(inX, inY); });
	
	// Install listener for mouse delta callbacks (will work also when mouse is outside the window or at the edge of the screen)
	MouseDelegate *delegate = [[MouseDelegate alloc] init: this];
	[NSNotificationCenter.defaultCenter addObserver: delegate selector: @selector(mouseDidConnect:) name: GCMouseDidConnectNotification object:nil];
	return true;
}

void MouseMacOS::Shutdown()
{
	if (mWindow != nullptr)
	{
		mWindow->SetMouseMovedCallback({});
		mWindow = nullptr;
	}
}

void MouseMacOS::Poll()
{
	mDeltaX = mDeltaXAcc;
	mDeltaY = mDeltaYAcc;
	
	mDeltaXAcc = 0;
	mDeltaYAcc = 0;
}

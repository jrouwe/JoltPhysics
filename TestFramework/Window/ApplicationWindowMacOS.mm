// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Window/ApplicationWindowMacOS.h>

#import <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

void ApplicationWindowMacOS::Initialize()
{
	@autoreleasepool
	{
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		sWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, mWindowWidth, mWindowHeight) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[sWindow cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
		[sWindow setTitle: @"TestFramework"];
		[sWindow makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];
		
		NSView *content_view = [sWindow contentView];
		mMetalLayer = [CAMetalLayer layer];
		[content_view setLayer:sMetalLayer];
		[content_view setWantsLayer:YES];
		//		[NSApp run];
	}
}

void ApplicationWindow::MainLoop(RenderCallback inRenderCallback)
{
	@autoreleasepool
	{
		for (;;)
		{
//			if (![NSApp isRunning])
//				return false;
			for (;;)
			{
				NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
				if (event == nullptr)
					break;
				[NSApp sendEvent:event];
//				[NSApp updateWindows];
			}

			// Call the render callback
			if (!inRenderCallback())
				return;
		}
	}
}

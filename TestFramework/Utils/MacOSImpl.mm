// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#import <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

static id sWindow = nullptr;
static id sMetalLayer = nullptr;

void MacOSInit(int inWindowWidth, int inWindowHeight)
{
	@autoreleasepool
	{
		[NSApplication sharedApplication];
		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		sWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, inWindowWidth, inWindowHeight) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[sWindow cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
		[sWindow setTitle: @"TestFramework"];
		[sWindow makeKeyAndOrderFront:nil];
		[NSApp activateIgnoringOtherApps:YES];
		
		NSView *content_view = [sWindow contentView];
		sMetalLayer = [CAMetalLayer layer];
		[content_view setLayer:sMetalLayer];
		[content_view setWantsLayer:YES];
		//		[NSApp run];
	}
}

bool MacOSUpdate()
{
	@autoreleasepool
	{
		for (;;)
		{
//			if (![NSApp isRunning])
//				return false;
			NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
			if (event == nullptr)
				return true;
			[NSApp sendEvent:event];
//			[NSApp updateWindows];
		}
	}
}	

void *MacOSGetMetalLayer()
{
	return sMetalLayer;
}

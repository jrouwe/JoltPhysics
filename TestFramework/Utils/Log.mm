// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/Log.h>
#include <cstdarg>

#ifdef JPH_PLATFORM_IOS
	#import <UIKit/UIKit.h>
#else
	#import <Cocoa/Cocoa.h>
#endif

// Show a message box, if inFatal is true the application will exit after the user closes the message box
static void sShowAlert(const char *inMessage, bool inFatal)
{
	NSString *message = [NSString stringWithCString: inMessage encoding: NSUTF8StringEncoding];

#ifdef JPH_PLATFORM_IOS
	// Find the view controller of the key window
	UIViewController *view_controller = nil;
	for (UIScene *scene in UIApplication.sharedApplication.connectedScenes)
		if ([scene isKindOfClass: [UIWindowScene class]])
			for (UIWindow *window in ((UIWindowScene *)scene).windows)
				if (window.isKeyWindow)
					view_controller = window.rootViewController;

	// If the view controller is already presenting something (e.g. another alert), present on top of that
	while (view_controller.presentedViewController != nil)
		view_controller = view_controller.presentedViewController;

	// If the application is not running yet, we cannot show the alert
	if (view_controller != nil)
	{
		__block bool closed = false;
		UIAlertController *alert = [UIAlertController alertControllerWithTitle: inFatal? @"Fatal Error" : @"Alert" message: message preferredStyle: UIAlertControllerStyleAlert];
		[alert addAction: [UIAlertAction actionWithTitle: @"OK" style: UIAlertActionStyleDefault handler: ^(UIAlertAction *action) { closed = true; }]];
		[view_controller presentViewController: alert animated: YES completion: nil];

		// For a fatal error, keep processing events until the user closes the alert
		if (inFatal)
			while (!closed)
				[[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate distantFuture]];
	}
#else
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	alert.messageText = message;
	[alert runModal];
#endif
}

// Trace to TTY
void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Log to the console
	printf("%s\n", buffer);
}

void Alert(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Trace("Alert: %s", buffer);

	sShowAlert(buffer, false);
}

void FatalError [[noreturn]] (const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Trace("Fatal Error: %s", buffer);

	sShowAlert(buffer, true);

	exit(1);
}

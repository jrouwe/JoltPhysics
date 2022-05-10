// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <cstdarg>
#include <Jolt/Core/FPException.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#ifdef JPH_PLATFORM_ANDROID
#include <Jolt/Core/Color.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#endif // JPH_PLATFORM_ANDROID

using namespace JPH;

// Emit everything needed for the main function
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_WINDOWS_SEH

#include "doctest.h"

using namespace doctest;

// Disable common warnings triggered by Jolt
JPH_SUPPRESS_WARNINGS

// Callback for traces
static void TraceImpl(const char *inFMT, ...)
{ 
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);

	// Forward to doctest
	MESSAGE(buffer);
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{ 
	// Format message
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s:%u: (%s) %s", inFile, inLine, inExpression, inMessage != nullptr? inMessage : "");

	// Forward to doctest
	FAIL_CHECK(buffer);

	// No breakpoint
	return false;
}

#endif // JPH_ENABLE_ASSERTS

#ifdef JPH_PLATFORM_WINDOWS_UWP

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

#ifdef _DEBUG
	// Enable leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Enable floating point exceptions
	FPExceptionsEnable enable_exceptions;
	JPH_UNUSED(enable_exceptions);

	// Create a factory
	Factory::sInstance = new Factory();

	// Register physics types
	RegisterTypes();

	int rv = Context().run();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	return rv;
}

#elif !defined(JPH_PLATFORM_ANDROID)

// Generic entry point
int main(int argc, char** argv)
{
	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

#if defined(JPH_PLATFORM_WINDOWS) && defined(_DEBUG)
	// Enable leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Enable floating point exceptions
	FPExceptionsEnable enable_exceptions;
	JPH_UNUSED(enable_exceptions);

	// Create a factory
	Factory::sInstance = new Factory();

	// Register physics types
	RegisterTypes();

	int rv = Context(argc, argv).run(); 

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

	return rv;
}

#else // !JPH_PLATFORM_ANDROID

// Reporter that writes logs to the Android log
struct LogReporter : public ConsoleReporter
{
	LogReporter(const ContextOptions &inOptions) : 
		ConsoleReporter(inOptions, mStream) 
	{
	}

#define REPORTER_OVERRIDE(func, type, arg)						\
	void func(type arg) override								\
	{															\
		ConsoleReporter::func(arg);								\
		const char *str = mStream.str().c_str();				\
		if (str[0] != 0)										\
			__android_log_write(ANDROID_LOG_INFO, "Jolt", str);	\
		mStream.str("");										\
	}

	REPORTER_OVERRIDE(test_run_start, DOCTEST_EMPTY, DOCTEST_EMPTY)
	REPORTER_OVERRIDE(test_run_end, const TestRunStats &, in)
	REPORTER_OVERRIDE(test_case_start, const TestCaseData &, in)
	REPORTER_OVERRIDE(test_case_reenter, const TestCaseData &, in)
	REPORTER_OVERRIDE(test_case_end, const CurrentTestCaseStats &, in)
	REPORTER_OVERRIDE(test_case_exception, const TestCaseException &, in)
	REPORTER_OVERRIDE(subcase_start, const SubcaseSignature &, in)
	REPORTER_OVERRIDE(subcase_end, DOCTEST_EMPTY, DOCTEST_EMPTY)
	REPORTER_OVERRIDE(log_assert, const AssertData &, in)
	REPORTER_OVERRIDE(log_message, const MessageData &, in)
	REPORTER_OVERRIDE(test_case_skipped, const TestCaseData &, in)

private:
	thread_local static std::ostringstream mStream;
};

thread_local std::ostringstream LogReporter::mStream;

DOCTEST_REGISTER_REPORTER("android_log", 0, LogReporter);

void AndroidInitialize(android_app *inApp)
{
	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Enable floating point exceptions
	FPExceptionsEnable enable_exceptions;
	JPH_UNUSED(enable_exceptions);

	// Create a factory
	Factory::sInstance = new Factory();

	// Register physics types
	RegisterTypes();

	// Run all tests
	Context context;
	context.addFilter("reporters", "android_log");
	int return_value = context.run();

	// Color the screen according to the test result
	JPH::Color color = return_value == 0? JPH::Color::sGreen : JPH::Color::sRed;
	ANativeWindow_acquire(inApp->window);
	ANativeWindow_Buffer buffer;
	ARect bounds;
	ANativeWindow_lock(inApp->window, &buffer, &bounds);
	switch (buffer.format)
	{
		case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
		case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
		{
			uint32 color_u32 = color.GetUInt32();
			for (int y = 0; y < buffer.height; ++y)
			{
				uint32 *dest = (uint32 *)((uint8 *)buffer.bits + y * buffer.stride * sizeof(uint32));
				for (int x = 0; x < buffer.width; ++x)
					*dest++ = color_u32;
			}
			break;
		}

		default:
			// TODO implement
			break;
	}
	ANativeWindow_unlockAndPost(inApp->window);
	ANativeWindow_release(inApp->window);

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

// Handle callback from Android
void AndroidHandleCommand(android_app *inApp, int32_t inCmd)
{
	switch (inCmd)
	{
	case APP_CMD_INIT_WINDOW:
		AndroidInitialize(inApp);
		break;
	}
}

// Main entry point for android
void android_main(struct android_app *ioApp)
{
	ioApp->onAppCmd = AndroidHandleCommand;

	int events;
	android_poll_source *source;
	do
	{
		if (ALooper_pollAll(1, nullptr, &events, (void **)&source) >= 0 && source != nullptr)
			source->process(ioApp, source);
	} while (ioApp->destroyRequested == 0);
}

#endif // JPH_PLATFORM_ANDROID

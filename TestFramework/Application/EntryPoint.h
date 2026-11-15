// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/FPException.h>

#if defined(JPH_PLATFORM_WINDOWS)

#define ENTRY_POINT(AppName, RegisterAllocator)																\
																											\
int WINAPI wWinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)		\
{																											\
	RegisterAllocator();																					\
																											\
	JPH_PROFILE_START("Main");																				\
																											\
	FPExceptionsEnable enable_exceptions;																	\
	JPH_UNUSED(enable_exceptions);																			\
																											\
	{																										\
		AppName app(GetCommandLineA());																		\
		app.Run();																							\
	}																										\
																											\
	JPH_PROFILE_END();																						\
																											\
	return 0;																								\
}																											\
																											\
int __cdecl main(int inArgC, char **inArgV)																	\
{																											\
	RegisterAllocator();																					\
																											\
	JPH_PROFILE_START("Main");																				\
																											\
	FPExceptionsEnable enable_exceptions;																	\
	JPH_UNUSED(enable_exceptions);																			\
																											\
	{																										\
		AppName app(Application::sCreateCommandLine(inArgC, inArgV));											\
		app.Run();																							\
	}																										\
																											\
	JPH_PROFILE_END();																						\
																											\
	return 0;																								\
}

#elif defined(JPH_PLATFORM_ANDROID)

#include <Window/ApplicationWindowAndroid.h>
#include <Utils/AssetStream.h>
#include <android_native_app_glue.h>

#define ENTRY_POINT(AppName, RegisterAllocator)																\
																											\
void android_main(struct android_app *ioApp)																\
{																											\
	if (!ApplicationWindowAndroid::sAcquireApp(ioApp))														\
		return;																								\
																											\
	RegisterAllocator();																					\
	AssetStream::sSetAssetManager(ioApp->activity->assetManager);											\
																											\
	JPH_PROFILE_START("Main");																				\
																											\
	{																										\
		AppName app(ApplicationWindowAndroid::sGetCommandLine());											\
		app.Run();																							\
	}																										\
																											\
	JPH_PROFILE_END();																						\
																											\
	ApplicationWindowAndroid::sReleaseApp(ioApp);															\
}

#else

#define ENTRY_POINT(AppName, RegisterAllocator)																\
																											\
int main(int inArgC, char **inArgV)																			\
{																											\
	RegisterAllocator();																					\
																											\
	JPH_PROFILE_START("Main");																				\
																											\
	FPExceptionsEnable enable_exceptions;																	\
	JPH_UNUSED(enable_exceptions);																			\
																											\
	{																										\
		AppName app(Application::sCreateCommandLine(inArgC, inArgV));											\
		app.Run();																							\
	}																										\
																											\
	JPH_PROFILE_END();																						\
																											\
	return 0;																								\
}

#endif

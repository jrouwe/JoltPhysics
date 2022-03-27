// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/FPException.h>

#if defined(JPH_PLATFORM_WINDOWS)

#define ENTRY_POINT(AppName)																				\
																											\
int WINAPI wWinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)		\
{																											\
	JPH_PROFILE_THREAD_START("Main");																		\
																											\
	FPExceptionsEnable enable_exceptions;																	\
	JPH_UNUSED(enable_exceptions);																			\
																											\
	AppName app;																							\
	app.Run();																								\
																											\
	JPH_PROFILE_THREAD_END();																				\
																											\
	return 0;																								\
}																											\
																											\
int __cdecl main(int inArgC, char **inArgV)																	\
{																											\
	JPH_PROFILE_THREAD_START("Main");																		\
																											\
	FPExceptionsEnable enable_exceptions;																	\
	JPH_UNUSED(enable_exceptions);																			\
																											\
	AppName app;																							\
	app.Run();																								\
																											\
	JPH_PROFILE_THREAD_END();																				\
																											\
	return 0;																								\
}

#else
#error Undefined
#endif

// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/FatalErrorIfFailed.h>
#include <Jolt/Core/StringTools.h>
#include <Utils/Log.h>

void FatalErrorIfFailed(HRESULT inHResult)
{
	if (FAILED(inHResult))
		FatalError("DirectX exception thrown: %s", ConvertToString(inHResult).c_str());
}

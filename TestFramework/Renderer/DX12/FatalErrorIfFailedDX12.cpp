// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <system_error>

#include <Renderer/DX12/FatalErrorIfFailedDX12.h>
#include <Jolt/Core/StringTools.h>
#include <Utils/Log.h>

void FatalErrorIfFailed(HRESULT inHResult)
{
	if (FAILED(inHResult))
	{
		string message = system_category().message(inHResult);
		FatalError("DirectX error returned: %s (%s)", ConvertToString(inHResult).c_str(), message.c_str());
	}
}

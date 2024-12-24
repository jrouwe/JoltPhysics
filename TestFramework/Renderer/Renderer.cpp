// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <Utils/Log.h>

#include <shellscalingapi.h>

static Renderer *sRenderer = nullptr;

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		if (sRenderer != nullptr)
			sRenderer->OnWindowResize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void Renderer::Initialize()
{
	// Prevent this window from auto scaling
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = TEXT("TestFrameworkClass");
	wcex.hIconSm = nullptr;
	if (!RegisterClassEx(&wcex))
		FatalError("Failed to register window class");

	// Create window
	RECT rc = { 0, 0, mWindowWidth, mWindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	mhWnd = CreateWindow(TEXT("TestFrameworkClass"), TEXT("TestFramework"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);
	if (!mhWnd)
		FatalError("Failed to create window");

	// Show window
	ShowWindow(mhWnd, SW_SHOW);

	// Store global renderer now that we're done initializing
	sRenderer = this;
}

void Renderer::OnWindowResize()
{
	JPH_ASSERT(!mInFrame);

	// Get new window size
	RECT rc;
	GetClientRect(mhWnd, &rc);
	mWindowWidth = max<LONG>(rc.right - rc.left, 8);
	mWindowHeight = max<LONG>(rc.bottom - rc.top, 8);
}

static Mat44 sPerspectiveInfiniteReverseZ(float inFovY, float inAspect, float inNear, float inYSign)
{
	float height = 1.0f / Tan(0.5f * inFovY);
	float width = height / inAspect;

	return Mat44(Vec4(width, 0.0f, 0.0f, 0.0f), Vec4(0.0f, inYSign * height, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, -1.0f), Vec4(0.0f, 0.0f, inNear, 0.0f));
}

void Renderer::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	// Mark that we're in the frame
	JPH_ASSERT(!mInFrame);
	mInFrame = true;

	// Store state
	mCameraState = inCamera;

	// Light properties
	Vec3 light_pos = inWorldScale * Vec3(250, 250, 250);
	Vec3 light_tgt = Vec3::sZero();
	Vec3 light_up = Vec3(0, 1, 0);
	Vec3 light_fwd = (light_tgt - light_pos).Normalized();
	float light_fov = DegreesToRadians(20.0f);
	float light_near = 1.0f;

	// Camera properties
	Vec3 cam_pos = Vec3(inCamera.mPos - mBaseOffset);
	float camera_fovy = inCamera.mFOVY;
	float camera_aspect = static_cast<float>(GetWindowWidth()) / GetWindowHeight();
	float camera_fovx = 2.0f * ATan(camera_aspect * Tan(0.5f * camera_fovy));
	float camera_near = 0.01f * inWorldScale;

	// Calculate camera frustum
	mCameraFrustum = Frustum(cam_pos, inCamera.mForward, inCamera.mUp, camera_fovx, camera_fovy, camera_near);

	// Calculate light frustum
	mLightFrustum = Frustum(light_pos, light_fwd, light_up, light_fov, light_fov, light_near);

	// Camera projection and view
	mVSBuffer.mProjection = sPerspectiveInfiniteReverseZ(camera_fovy, camera_aspect, camera_near, mPerspectiveYSign);
	Vec3 tgt = cam_pos + inCamera.mForward;
	mVSBuffer.mView = Mat44::sLookAt(cam_pos, tgt, inCamera.mUp);

	// Light projection and view
	mVSBuffer.mLightProjection = sPerspectiveInfiniteReverseZ(light_fov, 1.0f, light_near, mPerspectiveYSign);
	mVSBuffer.mLightView = Mat44::sLookAt(light_pos, light_tgt, light_up);

	// Camera ortho projection and view
	mVSBufferOrtho.mProjection = Mat44(Vec4(2.0f / mWindowWidth, 0.0f, 0.0f, 0.0f), Vec4(0.0f, -mPerspectiveYSign * 2.0f / mWindowHeight, 0.0f, 0.0f), Vec4(0.0f, 0.0f, -1.0f, 0.0f), Vec4(-1.0f, mPerspectiveYSign * 1.0f, 0.0f, 1.0f));
	mVSBufferOrtho.mView = Mat44::sIdentity();

	// Light projection and view are unused in ortho mode
	mVSBufferOrtho.mLightView = Mat44::sIdentity();
	mVSBufferOrtho.mLightProjection = Mat44::sIdentity();

	// Set constants for pixel shader
	mPSBuffer.mCameraPos = Vec4(cam_pos, 0);
	mPSBuffer.mLightPos = Vec4(light_pos, 0);
}

void Renderer::EndFrame()
{
	// Mark that we're no longer in the frame
	JPH_ASSERT(mInFrame);
	mInFrame = false;
}

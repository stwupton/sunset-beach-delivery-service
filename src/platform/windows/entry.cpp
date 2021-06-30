#pragma once

#include <cstdio>

#include <Windows.h>

#include "dx3d_renderer.cpp"
#include "types.hpp"
#include "platform/windows/utils.cpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static Dx3dRenderer renderer;

LRESULT CALLBACK handle(
	HWND windowHandle, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
) {
	INT result = 0;
	switch (message) {
		case WM_CREATE: {
			renderer.initialise(windowHandle);
		} break;

		case WM_PAINT: {
			// TODO(steven):  			
		} break;

		case WM_CLOSE: {
			shouldClose = true;
			DestroyWindow(windowHandle);
		} break;

		default: {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
	}
	return result;
}

INT createWin32Window(HINSTANCE instanceHandle, INT showFlag) {
	const LPCSTR className = "SBDS";

	WNDCLASS windowClass = {};
	windowClass.lpfnWndProc = handle;
	windowClass.hInstance = instanceHandle;
	windowClass.lpszClassName = className;
	windowClass.style = CS_OWNDC;
	RegisterClass(&windowClass);

	const HWND windowHandle = CreateWindowEx(
		0,
		className,
		"Sunset Beach Delivery Service",
		WS_OVERLAPPEDWINDOW,
		0,
		200,
		800,
		600,
		NULL,
		NULL,
		instanceHandle,
		NULL
	);

	if (windowHandle == NULL) {
		return 0;
	}

	ShowWindow(windowHandle, showFlag);

	return 0;
}

INT WINAPI wWinMain(
	HINSTANCE instanceHandle, 
	HINSTANCE prevInstanceHandle, 
	PWSTR cmdArgs, 
	INT showFlag
) {
	createWin32Window(instanceHandle, showFlag);

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}
}
#pragma once

#include <cstdio>

#include <Windows.h>

#include "dx3d_renderer.cpp"
#include "dx3d_sprite_loader.cpp"
#include "types.hpp"
#include "platform/windows/utils.cpp"
#include "platform/windows/window_config.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static Dx3dRenderer *renderer = new Dx3dRenderer();
static Dx3dSpriteLoader *loader = new Dx3dSpriteLoader();

LRESULT CALLBACK eventHandler(
	HWND windowHandle, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
) {
	INT result = 0;
	switch (message) {
		case WM_CREATE: {
			renderer->initialise(windowHandle);
			loader->initialise(renderer->device);
		} break;

		case WM_CLOSE: {
			shouldClose = true;
			DestroyWindow(windowHandle);
		} break;

		case WM_DESTROY: {
			delete renderer;
			PostQuitMessage(0);
			return 0;
		} break;

		default: {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
	}
	return result;
}

INT createWin32Window(HINSTANCE instanceHandle, INT showFlag) {
	const LPCWSTR className = L"SBDS";

	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = eventHandler;
	windowClass.hInstance = instanceHandle;
	windowClass.lpszClassName = className;
	windowClass.style = CS_OWNDC;
	RegisterClassEx(&windowClass);

	const HWND windowHandle = CreateWindowEx(
		0,
		className,
		L"Sunset Beach Delivery Service",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		screenWidth,
		screenHeight,
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
	CoInitialize(NULL);
	createWin32Window(instanceHandle, showFlag);

	Dx3dSpriteInfo sprites[] = {
		loader->load(L"assets/img/starry_background.jpg"),
	};
	
	const u8 spriteLength = sizeof(sprites) / sizeof(Dx3dSpriteInfo);
	
	MSG message = {};
	while (!shouldClose) {
		renderer->testRender(sprites, spriteLength);
		
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	loader->unload(sprites, spriteLength);
}
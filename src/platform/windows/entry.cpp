#pragma once

#include <cstdio>

#include <Windows.h>

#include "common/game_data.hpp"
#include "directx_renderer.cpp"
#include "dx3d_sprite_loader.cpp"
#include "game/game.cpp"
#include "platform/windows/input_processor.cpp"
#include "platform/windows/utils.cpp"
#include "platform/windows/window_config.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static DirectXRenderer *renderer = new DirectXRenderer();
static Dx3dSpriteLoader *loader = new Dx3dSpriteLoader();
static InputProcessor *inputProcessor = new InputProcessor();
static f32 delta;

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

			CREATESTRUCT *createStruct = (CREATESTRUCT*)lParam;
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
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

		case WM_LBUTTONDOWN: {
			GameData *gameData = (GameData*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
			gameData->input.primaryButtonDown = true;
		} break;

		case WM_LBUTTONUP: {
			GameData *gameData = (GameData*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);
			gameData->input.primaryButtonDown = false;
		} break;

		default: {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
	}
	return result;
}

INT createWin32Window(HINSTANCE instanceHandle, INT showFlag, GameData *gameData) {
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
		(void*)gameData
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
	HRESULT result = CoInitialize(NULL);
	ASSERT_HRESULT(result)
	
	GameData *gameData = new GameData {};

	createWin32Window(instanceHandle, showFlag, gameData);

	Game game;
	game.load(loader);
	game.setup();

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		inputProcessor->process(&gameData->input);

		u8 spriteLength = 0;
		Sprite *spriteBuffer = nullptr;
		game.update(&spriteBuffer, &spriteLength, delta);

		renderer->drawSprtes(spriteBuffer, spriteLength);

		u8 fps = 1 / delta;
		WCHAR text[100] = {};
		swprintf_s(text, L"FPS: %d", fps);
		renderer->drawText(text, 30.0f, 0.0f, 0.0f, 300.0f, 40.0f);

		swprintf_s(text, L"Mouse Down: %d", gameData->input.primaryButtonDown);
		renderer->drawText(text, 30.0f, 0.0f, 40.0f, 300.0f, 40.0f);

		renderer->finish();

		// TODO(steven): Move elsewhere, maybe a gameloop class?
		{
			static LARGE_INTEGER previousCounter;
			LARGE_INTEGER counter;
			LARGE_INTEGER frequency;
			
			QueryPerformanceCounter(&counter);
			QueryPerformanceFrequency(&frequency);

			if (previousCounter.QuadPart == 0) {
				previousCounter = counter;
				continue;
			}

			const f32 diff = counter.QuadPart - previousCounter.QuadPart;
			previousCounter = counter;

			delta = diff / frequency.QuadPart;
		}
	}

	loader->unload();
}
#pragma once

#include <cstdio>

#include <Windows.h>

#include "common/game_state.hpp"
#include "game/game.cpp"
#include "platform/windows/directx_renderer.cpp"
#include "platform/windows/dx3d_sprite_loader.cpp"
#include "platform/windows/input_processor.cpp"
#include "platform/windows/utils.cpp"
#include "platform/windows/window_config.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static DirectXResources *directXResources = new DirectXResources {};
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
			renderer->initialise(windowHandle, directXResources);
			inputProcessor->initialise(windowHandle);
			loader->initialise(directXResources);

			CREATESTRUCT *createStruct = (CREATESTRUCT*)lParam;
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
		} break;

		case WM_CLOSE: {
			shouldClose = true;
			DestroyWindow(windowHandle);
		} break;

		case WM_DESTROY: {
			PostQuitMessage(0);
		} break;

		case WM_KEYDOWN: { 
			if (wParam == VK_ESCAPE) {
				PostMessage(windowHandle, WM_CLOSE, NULL, NULL);
			}
		} break;

		default: {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
	}
	return result;
}

INT createWin32Window(HINSTANCE instanceHandle, INT showFlag, GameState *gameState) {
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
		WS_POPUP | WS_VISIBLE,
		// TODO(steven): Get actual monitor size
		0,
		0,
		screenWidth,
		screenHeight,
		NULL,
		NULL,
		instanceHandle,
		(void*)gameState
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
	
	GameState *gameState = new GameState {};

	createWin32Window(instanceHandle, showFlag, gameState);

	Game game;
	game.load(gameState);
	game.setup(gameState);

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		inputProcessor->process(&gameState->input);
		game.update(gameState, delta);

		loader->load(&gameState->loadQueue);
		renderer->drawSprites(gameState->sprites.data, gameState->sprites.length);
		renderer->drawUI(gameState->uiElements.data, gameState->uiElements.length);
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

	delete loader;
	delete renderer;
	delete inputProcessor;

	delete directXResources;
	delete gameState;
}
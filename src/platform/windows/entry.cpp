#pragma once

#include <cstdio>

#include <Windows.h>

#include "common/game_state.hpp"
#include "directx_renderer.cpp"
#include "SoundManager.cpp"
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
static SoundManager *soundManager = new SoundManager();
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
			inputProcessor->initialise(windowHandle);
			loader->initialise(renderer->device);
			soundManager->Initialise();
			//soundManager->PlaySoundW(L"assets/music/sound1.wav");

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
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
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
	game.load(loader);
	game.setup();

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		inputProcessor->process(&gameState->input);
		game.update(gameState, delta);

		// TODO(ross): Move this into game state at some point
		if (gameState->input.primaryButton.down) {
			soundManager->PlaySound(L"assets/music/sound1.wav");
		}

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
	delete soundManager;
	delete inputProcessor;
}
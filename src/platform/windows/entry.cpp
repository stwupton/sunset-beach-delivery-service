#pragma once

#include <cstdio>

#include <Windows.h>

#include "dx3d_renderer.cpp"
#include "dx3d_sprite_loader.cpp"
#include "input.hpp"
#include "sprite.cpp"
#include "types.hpp"
#include "platform/windows/input_processor.cpp"
#include "platform/windows/utils.cpp"
#include "platform/windows/window_config.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static Dx3dRenderer *renderer = new Dx3dRenderer();
static Dx3dSpriteLoader *loader = new Dx3dSpriteLoader();
static Input *input = new Input();
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
			input->primaryButtonDown = true;
		} break;

		case WM_LBUTTONUP: {
			input->primaryButtonDown = false;
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

	const Dx3dSpriteResource starryBackgroundTexture = loader->load(L"assets/img/starry_background.jpg");
	const Dx3dSpriteResource shipTexture = loader->load(L"assets/img/ship.png");

	Sprite ship;
	ship.textureReference = (void*)&shipTexture;
	ship.position = Vec3<f32>(200.0f, 200.0f, 0.4f);
	ship.scale = Vec2<f32>(0.5f, 0.5f);
	ship.angle = -95.0f;

	Sprite background;
	background.textureReference = (void*)&starryBackgroundTexture;
	background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
	background.scale = Vec2<f32>(1.3f, 1.3f);

	Sprite sprites[] = { background, ship };
	const u8 spriteLength = sizeof(sprites) / sizeof(Sprite);

	MSG message = {};
	while (!shouldClose) {
		renderer->renderSprites(sprites, spriteLength);
		
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		inputProcessor->process(input);

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

		sprites[1].position.x -= 100.0f * delta;

		const u8 fps = 1.0 / delta;
		LOG(L"FPS: %u\n", fps);
	}

	Dx3dSpriteResource toUnload[] = { starryBackgroundTexture, shipTexture };
	loader->unload(toUnload, 2);
}
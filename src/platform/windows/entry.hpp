#pragma once

#include <cstdio>
#include <Windows.h>

#include "common/game_state.hpp"
#include "common/window_config.hpp"
#include "editor/editor.hpp"
#include "frame_timing.hpp"
#include "game/game.hpp"
#include "platform/windows/directx_renderer.hpp"
#include "platform/windows/dx3d_sprite_loader.hpp"
#include "platform/windows/file_saver.hpp"
#include "platform/windows/input_processor.hpp"
#include "platform/windows/template_loader.hpp"
#include "platform/windows/sound_manager.hpp"
#include "platform/windows/utils.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static bool editorOpen = false;
static bool inFocus = true;
static DirectXResources *directXResources = new DirectXResources{};
static DirectXRenderer *renderer = new DirectXRenderer();
static Dx3dSpriteLoader *loader = new Dx3dSpriteLoader();
static SoundManager *soundManager = new SoundManager();
static InputProcessor *inputProcessor = new InputProcessor();
static FrameTiming timings = {};

#ifdef DEBUG
struct FrameProfile {
	String16<64> name;
	f32 time = 0;
};

static Array<FrameProfile, 32> profiles = {};

#define PROFILE(_name, _call)                          \
{                                                      \
	LARGE_INTEGER start;                                 \
	LARGE_INTEGER end;                                   \
                                                       \
	QueryPerformanceCounter(&start);                     \
	_call;                                               \
	QueryPerformanceCounter(&end);                       \
	                                                     \
	const f32 _diff = end.QuadPart - start.QuadPart;     \
	profiles.push({ _name, _diff / timings.frequency }); \
}
#else
#define PROFILE(_name, _call) _call;
#endif

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
			soundManager->initialise();

			CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)createStruct->lpCreateParams);
		} break;

		case WM_SETFOCUS: {
			inFocus = true;
		} break;

		case WM_KILLFOCUS: {
			inFocus = false;
		} break;

		case WM_CLOSE: {
			shouldClose = true;
			DestroyWindow(windowHandle);
		} break;

		case WM_DESTROY: {
			PostQuitMessage(0);
		} break;

		case WM_KEYDOWN: {
			GameState *gameState = (GameState *)GetWindowLongPtr(windowHandle, GWLP_USERDATA);

			if (wParam == VK_ESCAPE) {
				PostMessage(windowHandle, WM_CLOSE, NULL, NULL);
			}

			gameState->input.keyDown = wParam;

#ifdef DEBUG
			if (wParam == 'E') {
				// Reset to menu whenever we click the E key
				gameState->editorState.mode = EditorMode::menu;

				editorOpen = !editorOpen;
			}

			if (wParam == VK_OEM_PLUS) {
				gameState->gameSpeed++;
			} else if (wParam == VK_OEM_MINUS) {
				gameState->gameSpeed--;
			}
#endif
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
		(void *)gameState
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

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	timings.frequency = frequency.QuadPart;

	GameState *gameState = new GameState {};
	createWin32Window(instanceHandle, showFlag, gameState);

	Game::setup(gameState);

	loadTemplates(gameState);

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		loader->load(&gameState->textureLoadQueue);

		if (inFocus) {
			PROFILE(L"Process Input", inputProcessor->process(&gameState->input))
		}

		if (editorOpen) {
			Editor::update(gameState);

			SaveData &saveData = gameState->editorState.saveData;
			if (saveData.pending) {
				save(saveData.path.data, saveData.buffer, saveData.size);
				saveData.pending = false;
			}
		} else {
			PROFILE(L"Game Update", Game::update(gameState, timings.delta))
		}

		PROFILE(L"Sound", soundManager->process(&gameState->soundLoadQueue, &gameState->pendingMusicItem))

		PROFILE(L"Render Start", renderer->start())
		PROFILE(L"  Draw Starfield", renderer->drawStarfield())
		PROFILE(L"  Draw Sprites", renderer->drawSprites(gameState->sprites.data, gameState->sprites.length))
		PROFILE(L"  Draw UI", renderer->drawUI(gameState->uiElements.data, gameState->uiElements.length))
		PROFILE(L"Render Finish", renderer->finish())

		// Reset render buffers
		gameState->sprites.clear();
		gameState->uiElements.clear();

		inputProcessor->updateCursor(gameState->input.cursor);
		gameState->input.cursor = Cursor::arrow;
		gameState->input.keyDown = '\0';

#ifdef DEBUG
		UITextData text = {};
		text.font = L"consolas";
		text.fontSize = 16.0f;
		text.width = 300.0f;
		text.height = 20.0f;
		text.position = Vec2(screenWidth - text.width, 300.0f);
		text.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);

		wchar_t textBuffer[128] = {};

		for (const FrameProfile profile : profiles) {
			swprintf_s(textBuffer, L"%s: %.5fs", profile.name.data, profile.time);
			text.text = textBuffer;
			text.position.y += text.height;

			gameState->uiElements.push(text);
		}

		profiles.clear();
#endif

		// Update delta
		{
			LARGE_INTEGER counter;

			QueryPerformanceCounter(&counter);
			if (timings.previousTime == 0) {
				timings.previousTime = counter.QuadPart;
				continue;
			}

			const f32 diff = counter.QuadPart - timings.previousTime;
			timings.previousTime = counter.QuadPart;

			timings.delta = diff / timings.frequency;
		}
	}

	delete loader;
	delete renderer;
	delete soundManager;
	delete inputProcessor;

	delete directXResources;
	delete gameState;
}
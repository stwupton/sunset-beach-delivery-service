#define _USE_MATH_DEFINES 1

#include <cmath>

#include <cstdio>
#include <Windows.h>

#include "common/asset_definitions.hpp"
#include "common/input.hpp"
#include "common/window_config.hpp"
#include "platform/windows/input_processor.hpp"
#include "platform/windows/utils.hpp"
#include "types/array.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

#include "ecs_common.hpp"
#include "ecs_renderer.hpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static DirectXResources *directXResources = new DirectXResources();
static Dx3dSpriteLoader *loader = new Dx3dSpriteLoader();
static DirectXRenderer *renderer = new DirectXRenderer();
static InputProcessor *inputProcessor = new InputProcessor();
static Input *input = new Input {};
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
			loader->initialise(directXResources);
			inputProcessor->initialise(windowHandle);

			CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
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

			input->keyDown = wParam;
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
		WS_POPUP | WS_VISIBLE,
		// TODO(steven): Get actual monitor size
		0,
		0,
		screenWidth,
		screenHeight,
		NULL,
		NULL,
		instanceHandle,
		nullptr
	);

	if (windowHandle == NULL) {
		return 0;
	}

	ShowWindow(windowHandle, showFlag);

	return 0;
}

void updateVelocity(Entity_State<MAX_ENTITY_COUNT> *state, f32 delta) {
	for (Entity entity : state->entities) {
		if (!entity.has<Velocity, Transform>()) {
			continue;
		}

		const Velocity &velocity = state->get_component<Velocity>(entity.uid.id);
		Transform &transform = state->get_component<Transform>(entity.uid.id);

		transform.position = Vec3(
			transform.position.x + velocity.direction.x * delta * 50,
			transform.position.y + velocity.direction.y * delta * 50
		);
	}
}

INT WINAPI wWinMain(
	HINSTANCE instanceHandle,
	HINSTANCE prevInstanceHandle,
	PWSTR cmdArgs,
	INT showFlag
) {
	HRESULT result = CoInitialize(NULL);
	ASSERT_HRESULT(result)

	createWin32Window(instanceHandle, showFlag);

	TextureLoadQueue loadQueue;
	loadQueue.push(TextureAssetId::ship);
	loadQueue.push(TextureAssetId::enemyShip);
	loader->load(&loadQueue);

	Entity_State<MAX_ENTITY_COUNT> *state = new Entity_State<MAX_ENTITY_COUNT> {};
	state->add_component_pool<Transform>();
	state->add_component_pool<Sprite>();
	state->add_component_pool<Velocity>();

	for (size_t i = 0; i < 256; i++) {
		Entity_Id id = state->create_entity();

		Transform transform = {};
		transform.position = Vec3<f32>(
			rand() % 1920 - 1920 * 0.5f, 
			rand() % 1080 - 1080 * 0.5f
		);
		transform.scale = Vec2<f32>(
			(f32)rand() / RAND_MAX * 0.3, 
			(f32)rand() / RAND_MAX * 0.3
		);
		transform.angle = rand() % 360;
		state->add_component<Transform>(id, transform);

		// const TextureAssetId asset = rand() % 2 == 0 ? TextureAssetId::ship : TextureAssetId::enemyShip;
		state->add_component<Sprite>(id, { TextureAssetId::ship });

		// if (rand() % 2 == 0) {
			state->add_component<Velocity>(id, { Vec2<f32>(
				rand() % 20 - 10, 
				rand() % 20 - 10
			) });
		// }
	}

	MSG message = {};
	while (!shouldClose) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		inputProcessor->process(input);
		updateVelocity(state, delta);

		renderer->drawSprites(state);
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

	delete renderer;
	delete loader;
	delete inputProcessor;
	delete state;
	delete directXResources;
}
#pragma once

#include <cstdio>
#include <malloc.h>

#include <Windows.h>
#include <d3d11.h>

#include "types.hpp"
#include "platform/windows/utils.cpp"

// TODO(steven): Move elsewhere
static bool shouldClose = false;
static RECT clientRect;

LRESULT CALLBACK handle(
	HWND windowHandle, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
) {
	INT result = 0;
	switch (message) {
		case WM_CREATE: {
			GetClientRect(windowHandle, &clientRect);

			D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_1;

			DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
			swapChainDescription.BufferCount = 1;
			swapChainDescription.BufferDesc.Width = clientRect.right - clientRect.left;
			swapChainDescription.BufferDesc.Height = clientRect.bottom - clientRect.top;
			swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.OutputWindow = windowHandle;
			swapChainDescription.SampleDesc.Count = 1;
			swapChainDescription.SampleDesc.Quality = 0;
			swapChainDescription.Windowed = true;

			// TODO(steven): Printing the adapters for now. Come back and check if we
			// have any use for them.
			{
				IDXGIFactory *factory = nullptr;
				CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

				IDXGIAdapter *adapter;
				for (u8 i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
					DXGI_ADAPTER_DESC adapterDescription;
					adapter->GetDesc(&adapterDescription);

					const u16 descriptionLength = wcslen(adapterDescription.Description) + 1;
					const size_t descriptionSize = sizeof(WCHAR) * descriptionLength;
					WCHAR *buffer = (WCHAR*)alloca(descriptionSize);
					swprintf_s(buffer, descriptionLength, L"%ls\n", adapterDescription.Description);

					OutputDebugStringW(buffer);
				}
			}

			IDXGISwapChain *swapChain;
			ID3D11Device *device;
			D3D_FEATURE_LEVEL featureLevel;
			ID3D11DeviceContext *immediateContext;

			D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_REFERENCE,
				NULL,
				0,
				&featureLevels,
				1,
				D3D11_SDK_VERSION,
				&swapChainDescription,
				&swapChain,
				&device,
				&featureLevel,
				&immediateContext
			);
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
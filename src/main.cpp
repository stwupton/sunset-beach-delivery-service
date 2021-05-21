#include <Windows.h>
#include <d2d1.h>

// TODO(steven): Remove
static bool shouldClose = false;
static ID2D1HwndRenderTarget *renderTarget = NULL;
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
			ID2D1Factory *factory;
			D2D1CreateFactory(
				D2D1_FACTORY_TYPE_SINGLE_THREADED,
				__uuidof(ID2D1Factory),
				(void**)&factory
			);

			GetClientRect(windowHandle, &clientRect);

			// Create a Direct2D render target          
			HRESULT result = factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(
					windowHandle,
					D2D1::SizeU(
						clientRect.right - clientRect.left,
						clientRect.bottom - clientRect.top
					)
				),
				&renderTarget
			);
		} break;

		case WM_PAINT: {
			ID2D1SolidColorBrush *blackBrush = NULL;
			renderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&blackBrush
			); 

			ID2D1SolidColorBrush *whiteBrush = NULL;
			renderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White),
				&whiteBrush
			); 

			renderTarget->BeginDraw();

			renderTarget->FillRectangle(
				D2D1::RectF(
					clientRect.left,
					clientRect.top,
					clientRect.right,
					clientRect.bottom
				),
				whiteBrush
			);

			renderTarget->FillRectangle(
				D2D1::RectF(
					clientRect.left + 100.0f,
					clientRect.top + 100.0f,
					clientRect.right - 100.0f,
					clientRect.bottom - 100.0f
				),
				blackBrush
			);

			renderTarget->EndDraw(); 
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
	const LPCSTR className = "OpenGL Example Window";

	WNDCLASS windowClass = {};
	windowClass.lpfnWndProc = handle;
	windowClass.hInstance = instanceHandle;
	windowClass.lpszClassName = className;
	windowClass.style = CS_OWNDC;
	RegisterClass(&windowClass);

	HWND windowHandle = CreateWindowEx(
		0,
		className,
		"OpenGL Window",
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
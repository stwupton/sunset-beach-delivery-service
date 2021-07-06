#pragma once

#include <cstdio>

#include <Windows.h>
#include <malloc.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "types.hpp"

class Dx3dRenderer {
protected:
	IDXGISwapChain *swapChain;
	ID3D11Device *device;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11DeviceContext *deviceContext;
	RECT clientRect;

	// TODO(steven): delete
	ID3D11Buffer *vertexBuffer;

public:
	~Dx3dRenderer() {
		this->swapChain->Release();
		this->device->Release();
		this->deviceContext->Release();
	}

	void initialise(HWND windowHandle) {
		GetClientRect(windowHandle, &this->clientRect);

		DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
		swapChainDescription.BufferCount = 1;
		swapChainDescription.BufferDesc.Width = this->clientRect.right - this->clientRect.left;
		swapChainDescription.BufferDesc.Height = this->clientRect.bottom - this->clientRect.top;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.OutputWindow = windowHandle;
		swapChainDescription.SampleDesc.Count = 4;
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

				// NOTE(steven): + 2 for new line & null terminator
				const size_t descriptionLength = 
					sizeof(adapterDescription.Description) / 
					sizeof(WCHAR) + 2;

				WCHAR buffer[descriptionLength];
				swprintf_s(buffer, descriptionLength, L"%ls\n", adapterDescription.Description);

				OutputDebugString(buffer);
			}
		}

		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			NULL,
			NULL,
			D3D11_SDK_VERSION,
			&swapChainDescription,
			&this->swapChain,
			&this->device,
			&this->featureLevel,
			&this->deviceContext
		);

		// TODO(steven): delete
		// Creating a test vertex buffer
		{
			struct SimpleVertexCombined {
				DirectX::XMFLOAT3 position;
				DirectX::XMFLOAT3 colour;
			};

			SimpleVertexCombined vertexData[] = {
				DirectX::XMFLOAT3(.0f, .5f, .5f),
				DirectX::XMFLOAT3(.0f, .0f, .5f),
				DirectX::XMFLOAT3(.5f, -.5f, .5f),
				DirectX::XMFLOAT3(.5f, .0f, .0f),
				DirectX::XMFLOAT3(-.5f, -.5f, .5f),
				DirectX::XMFLOAT3(.0f, .5f, .0f),
			};

			D3D11_BUFFER_DESC bufferDescription;
			bufferDescription.Usage = D3D11_USAGE_DEFAULT;
			bufferDescription.ByteWidth = sizeof(SimpleVertexCombined) * 3;
			bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDescription.CPUAccessFlags = 0;
			bufferDescription.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = vertexData;
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;

			this->device->CreateBuffer(
				&bufferDescription, 
				&subresourceData, 
				&this->vertexBuffer
			);
		}
	}

	// TODO(steven): delete
	void testRender() const {
	}
};
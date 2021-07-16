#pragma once

#include <cstdio>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <malloc.h>
#include <Windows.h>

#include "types.hpp"
#include "platform/windows/window_config.hpp"
#include "platform/windows/utils.cpp"

// TODO(steven): Delete
struct Vertex {
	Vec3<f32> position;
	Color color;
};

class Dx3dRenderer {
protected:
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
	ID3D11RenderTargetView *renderView; 
	IDXGISwapChain *swapChain;

	// TODO(steven): delete
	ID3D11Buffer *vertexBuffer;
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *vertexBufferLayout;

	void compileShaders() {
		ID3D10Blob *vertexShaderBlob; 
		ID3D10Blob *pixelShaderBlob;

		D3DCompileFromFile(L"assets/shaders/shaders.hlsl", 0, 0, "vertex", "vs_4_0", 0, 0, &vertexShaderBlob, 0); 
		D3DCompileFromFile(L"assets/shaders/shaders.hlsl", 0, 0, "pixel", "ps_4_0", 0, 0, &pixelShaderBlob, 0); 

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(), 
			vertexShaderBlob->GetBufferSize(), 
			NULL, 
			&this->vertexShader
		);

		this->device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(), 
			pixelShaderBlob->GetBufferSize(), 
			NULL, 
			&this->pixelShader
		);

		this->deviceContext->VSSetShader(this->vertexShader, 0, 0);
		this->deviceContext->PSSetShader(this->pixelShader, 0, 0);

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		this->device->CreateInputLayout(
			inputElementDescriptions,
			2,
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			&this->vertexBufferLayout
		);
		this->deviceContext->IASetInputLayout(this->vertexBufferLayout);
	}

	void createDeviceAndSwapChain(HWND windowHandle) {
		DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
		swapChainDescription.BufferCount = 1;
		swapChainDescription.BufferDesc.Width = screenWidth;
		swapChainDescription.BufferDesc.Height = screenHeight;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.OutputWindow = windowHandle;
		swapChainDescription.SampleDesc.Count = 4;
		swapChainDescription.SampleDesc.Quality = 0;
		swapChainDescription.Windowed = true;
		swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// TODO(steven): Printing the adapters for now. Come back and check if we
		// have any use for them.
		{
			IDXGIFactory *factory = nullptr;
			CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

			IDXGIAdapter *adapter;
			for (u8 i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
				DXGI_ADAPTER_DESC adapterDescription;
				adapter->GetDesc(&adapterDescription);
				LOG(L"%ls\n", adapterDescription.Description)
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
			NULL,
			&this->deviceContext
		);

		ID3D11Texture2D *backBuffer;
		this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		this->device->CreateRenderTargetView(backBuffer, NULL, &this->renderView);
		this->deviceContext->OMSetRenderTargets(1, &this->renderView, NULL);
		backBuffer->Release();

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = screenWidth;
		viewport.Height = screenHeight;
		this->deviceContext->RSSetViewports(1, &viewport);
	}

	void createVertexBuffer() {
		Vertex exampleVertices[] = {
			{ Vec3(0.0f, 0.5f, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
			{ Vec3(0.45f, -0.5f, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
			{ Vec3(-0.45f, -0.5f, 0.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },
		};

		D3D11_BUFFER_DESC bufferDescription = {};
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(Vertex) * 3;
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = exampleVertices;

		this->device->CreateBuffer(
			&bufferDescription, 
			&subresourceData, 
			&this->vertexBuffer
		);
	}

public:
	~Dx3dRenderer() {
		// Direct3D is incapable of closing down in full screen mode, so we ensure 
		// that it's in windowed mode here.
		this->swapChain->SetFullscreenState(false, NULL);

		RELEASE_COM_OBJ(this->vertexShader)
		RELEASE_COM_OBJ(this->pixelShader)
		RELEASE_COM_OBJ(this->vertexBuffer)
		RELEASE_COM_OBJ(this->vertexBufferLayout)
		RELEASE_COM_OBJ(this->swapChain)
		RELEASE_COM_OBJ(this->device)
		RELEASE_COM_OBJ(this->deviceContext)
		RELEASE_COM_OBJ(this->renderView)
	}

	void initialise(HWND windowHandle) {
		this->createDeviceAndSwapChain(windowHandle);
		this->compileShaders();
		this->createVertexBuffer();
	}

	// TODO(steven): delete
	void testRender() const {
		const Color clearColor(0.0f, 0.2f, 0.4f, 1.0f);
		this->deviceContext->ClearRenderTargetView(this->renderView, (f32*)&clearColor);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		this->deviceContext->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);

		this->deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->deviceContext->Draw(3, 0);
		this->swapChain->Present(0, 0);
	}
};
#pragma once

#include <cstdio>

#include <cmath>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <malloc.h>
#include <Windows.h>
#include <combaseapi.h>
#include <wincodec.h>

#include "sprite.cpp"
#include "types.hpp"
#include "platform/windows/dx3d_sprite_loader.cpp"
#include "platform/windows/window_config.hpp"
#include "platform/windows/utils.cpp"
#include "platform/windows/sprite_vertex.hpp"

// TODO(steven): Move somewhere else and rename
struct ConstantBuffer {
	Mat4x4<f32> projection;
};

struct SpriteInfoBuffer {
	Mat4x4<f32> transform;
};

class Dx3dRenderer {
public:
	ID3D11Device *device;

protected:
	ID3D11DepthStencilState *depthStencilState;
	ID3D11DepthStencilView *depthStencilView;
	ID3D11DeviceContext *deviceContext;
	ID3D11RenderTargetView *renderView; 
	IDXGISwapChain *swapChain;

	// TODO(steven): delete
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *vertexBufferLayout;
	ID3D11BlendState *blendState;
	ID3D11Buffer *spriteInfoBuffer;
	ID3D11Buffer *constantBuffer;

public:
	~Dx3dRenderer() {
		// Direct3D is incapable of closing down in full screen mode, so we ensure 
		// that it's in windowed mode here.
		this->swapChain->SetFullscreenState(false, NULL);

		RELEASE_COM_OBJ(this->vertexShader)
		RELEASE_COM_OBJ(this->pixelShader)
		RELEASE_COM_OBJ(this->vertexBufferLayout)
		RELEASE_COM_OBJ(this->swapChain)
		RELEASE_COM_OBJ(this->device)
		RELEASE_COM_OBJ(this->deviceContext)
		RELEASE_COM_OBJ(this->renderView)
		RELEASE_COM_OBJ(this->depthStencilState)
		RELEASE_COM_OBJ(this->depthStencilView)
	}

	void initialise(HWND windowHandle) {
		this->createDeviceAndSwapChain(windowHandle);
		this->compileShaders();
		this->createBlendState();
		this->createDepthBuffer();
		this->createConstantBuffers();
	}

	void renderSprites(Sprite *sprites, UINT bufferLength) const {
		const Rgba clearColor(0.0f, 0.2f, 0.4f, 1.0f);
		this->deviceContext->ClearRenderTargetView(this->renderView, (f32*)&clearColor);

		this->deviceContext->ClearDepthStencilView(
			this->depthStencilView, 
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
			1.0f, 
			0
		);

		this->deviceContext->VSSetConstantBuffers(0, 1, &this->constantBuffer);
		this->deviceContext->VSSetConstantBuffers(1, 1, &this->spriteInfoBuffer);

		this->deviceContext->OMSetBlendState(this->blendState, 0, 0xffffffff);
		this->deviceContext->OMSetDepthStencilState(this->depthStencilState, NULL);
		this->deviceContext->OMSetRenderTargets(1, &this->renderView, this->depthStencilView);

		this->deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		for (UINT i = 0; i < bufferLength; i++) {
			const Sprite &sprite = sprites[i];
			const Dx3dSpriteResource *info = (Dx3dSpriteResource*)sprite.textureReference;

			const UINT stride = sizeof(SpriteVertex);
			const UINT offset = 0;
			this->deviceContext->IASetVertexBuffers(0, 1, &info->vertexBuffer, &stride, &offset);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			this->deviceContext->Map(this->spriteInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			SpriteInfoBuffer *buffer = (SpriteInfoBuffer*)mappedResource.pData;

			Mat4x4<f32> transform;
			transform = transform.translate(sprite.position.x, sprite.position.y, sprite.position.z);
			transform = transform.scale(sprite.scale.x, sprite.scale.y);
			transform = transform.rotate(-sprite.angle * M_PI / 180);

			buffer->transform = transform;
			this->deviceContext->Unmap(this->spriteInfoBuffer, 0);

			this->deviceContext->PSSetShaderResources(0, 1, &info->texture2dView);
			this->deviceContext->Draw(4, 0);
		}

		this->swapChain->Present(2, 0);
	}

protected:
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
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	void createBlendState() {
		D3D11_BLEND_DESC blendDescription = {};
		blendDescription.RenderTarget[0].BlendEnable = true;
		blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		this->device->CreateBlendState(&blendDescription, &this->blendState);
	}

	void createConstantBuffers() {
		{
			ConstantBuffer buffer = {};

			const f32 left = -((f32)screenWidth / 2);
			const f32 right = (f32)screenWidth / 2;
			const f32 top = (f32)screenHeight / 2;
			const f32 bottom = -((f32)screenHeight / 2);
			const f32 front = -1.0f;
			const f32 back = 1.0f;

			const f32 x0 = 2.0f / (right - left);
			const f32 y1 = 2.0f / (top - bottom);
			const f32 z2 = 2.0f / (back - front);
			const f32 x3 = -(right + left) / (right - left);
			const f32 y3 = -(top + bottom) / (top - bottom);
			const f32 z3 = -(back + front) / (back - front);

			buffer.projection = Mat4x4<f32>(
				x0, 0.0f, 0.0f, x3,
				0.0f, y1, 0.0f, y3,
				0.0f, 0.0f, z2, z3,
				0.0f, 0.0f, 0.0f, 1.0f
			);

			D3D11_BUFFER_DESC bufferDescription = {};
			bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
			bufferDescription.ByteWidth = sizeof(buffer);
			bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pSysMem = &buffer;

			this->device->CreateBuffer(
				&bufferDescription, 
				&subresourceData, 
				&this->constantBuffer
			);
		}

		{
			SpriteInfoBuffer buffer = {};

			D3D11_BUFFER_DESC bufferDescription = {};
			bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
			bufferDescription.ByteWidth = sizeof(buffer);
			bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pSysMem = &buffer;

			this->device->CreateBuffer(
				&bufferDescription, 
				&subresourceData, 
				&this->spriteInfoBuffer
			);
		}
	}

	void createDepthBuffer() {
		D3D11_TEXTURE2D_DESC depthStencilResourceDescription = {};
		depthStencilResourceDescription.Width = screenWidth;
		depthStencilResourceDescription.Height = screenHeight;
		depthStencilResourceDescription.MipLevels = 1;
		depthStencilResourceDescription.ArraySize = 1;
		depthStencilResourceDescription.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilResourceDescription.SampleDesc.Count = 4;
		depthStencilResourceDescription.SampleDesc.Quality = 0;
		depthStencilResourceDescription.Usage = D3D11_USAGE_DEFAULT;
		depthStencilResourceDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ID3D11Texture2D *depthStencil;
		this->device->CreateTexture2D(&depthStencilResourceDescription, NULL, &depthStencil);

		D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
		depthStencilDescription.DepthEnable = true;
		depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;

		this->device->CreateDepthStencilState(&depthStencilDescription, &this->depthStencilState);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription = {};
		depthStencilViewDescription.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDescription.Texture2D.MipSlice = 0;

		this->device->CreateDepthStencilView(
			depthStencil,
			&depthStencilViewDescription, 
			&this->depthStencilView
		);

		RELEASE_COM_OBJ(depthStencil)
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
			D3D11_CREATE_DEVICE_DEBUG,
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
		RELEASE_COM_OBJ(backBuffer)

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = screenWidth;
		viewport.Height = screenHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		this->deviceContext->RSSetViewports(1, &viewport);
	}
};
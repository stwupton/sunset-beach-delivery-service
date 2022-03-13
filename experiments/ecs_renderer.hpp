#pragma once

#include <cstdio>
#include <cmath>

#include <d2d1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dwrite.h>
#include <dxgi.h>
#include <Windows.h>
#include <combaseapi.h>
#include <wincodec.h>

#include "common/window_config.hpp"
#include "platform/windows/utils.hpp"
#include "platform/windows/sprite_vertex.hpp"
#include "types/core.hpp"
#include "types/matrix.hpp"
#include "types/vector.hpp"

#include "ecs_loader.hpp"
#include "ecs_common.hpp"

// TODO(steven): Move somewhere else and rename
struct ConstantBuffer {
	Mat4x4<f32> projection;
};

struct SpriteInfoBuffer {
	Mat4x4<f32> transform;
};

class DirectXRenderer {
protected:
	ID3D11DepthStencilState *depthStencilState;
	ID3D11DepthStencilView *depthStencilView;
	ID3D11DeviceContext *deviceContext;
	ID3D11RenderTargetView *renderView; 
	DirectXResources *resources;
	IDXGISwapChain *swapChain;
	ID2D1Factory *d2dFactory;
	ID2D1RenderTarget *d2dRenderTarget;
	ID2D1SolidColorBrush *d2dSolidBrush;
	IDWriteFactory *dWriteFactory;

	// TODO(steven): delete
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *vertexBufferLayout;
	ID3D11BlendState *blendState;
	ID3D11Buffer *spriteInfoBuffer;
	ID3D11Buffer *constantBuffer;

public:
	~DirectXRenderer() {
		RELEASE_COM_OBJ(this->vertexShader)
		RELEASE_COM_OBJ(this->pixelShader)
		RELEASE_COM_OBJ(this->vertexBufferLayout)
		RELEASE_COM_OBJ(this->swapChain)
		RELEASE_COM_OBJ(this->deviceContext)
		RELEASE_COM_OBJ(this->renderView)
		RELEASE_COM_OBJ(this->depthStencilState)
		RELEASE_COM_OBJ(this->depthStencilView)
		RELEASE_COM_OBJ(this->d2dFactory)
		RELEASE_COM_OBJ(this->d2dRenderTarget)
		RELEASE_COM_OBJ(this->d2dSolidBrush)
		RELEASE_COM_OBJ(this->dWriteFactory)
		RELEASE_COM_OBJ(this->blendState)
		RELEASE_COM_OBJ(this->spriteInfoBuffer)
		RELEASE_COM_OBJ(this->constantBuffer)

#ifdef DEBUG
		ID3D11Debug *debug = nullptr;
		this->resources->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
		debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		RELEASE_COM_OBJ(debug)
#endif

		RELEASE_COM_OBJ(this->resources->device)
	}

	void initialise(HWND windowHandle, DirectXResources *resources) {
		this->resources = resources;

		this->createDeviceAndSwapChain(windowHandle);
		this->compileShaders();
		this->createBlendState();
		this->createDepthBuffer();
		this->createConstantBuffers();
		this->create2dTarget();
	}

	void create2dTarget() {
		IDXGISurface *backBuffer;
		HRESULT result = this->swapChain->GetBuffer(0, __uuidof(IDXGISurface), (LPVOID*)&backBuffer);
		ASSERT_HRESULT(result)

		result = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, 
			{ D2D1_DEBUG_LEVEL_INFORMATION }, 
			&this->d2dFactory
		);
		ASSERT_HRESULT(result)

		D2D1_RENDER_TARGET_PROPERTIES renderProperties = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_HARDWARE,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
		);

		result = this->d2dFactory->CreateDxgiSurfaceRenderTarget(
			backBuffer,
			renderProperties,
			&this->d2dRenderTarget
		);
		ASSERT_HRESULT(result)

		RELEASE_COM_OBJ(backBuffer)

		result = this->d2dRenderTarget->CreateSolidColorBrush({}, &this->d2dSolidBrush);
		ASSERT_HRESULT(result)

		result = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED, 
			__uuidof(IDWriteFactory), 
			(IUnknown**)&this->dWriteFactory
		);
		ASSERT_HRESULT(result)
	}

	void drawSprites(Entity_State<MAX_ENTITY_COUNT> *state) const {
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

		const Dx3dSpriteResource &textureReference = this->resources->spriteResources[(size_t)TextureAssetId::ship];
		const UINT stride = sizeof(SpriteVertex);
		const UINT offset = 0;
		this->deviceContext->IASetVertexBuffers(0, 1, &textureReference.vertexBuffer, &stride, &offset);
		this->deviceContext->PSSetShaderResources(0, 1, &textureReference.texture2dView);

		for (const Entity &entity : state->entities) {
			if (!entity.has<Sprite, Transform>()) {
				continue;
			}

			// const Sprite &sprite = state->get_component<Sprite>(entity.uid.id);
			const Transform &transform = state->get_component<Transform>(entity.uid.id);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT result = this->deviceContext->Map(
				this->spriteInfoBuffer, 
				NULL, 
				D3D11_MAP_WRITE_DISCARD, 
				NULL, 
				&mappedResource
			);
			ASSERT_HRESULT(result)

			Mat4x4<f32> transform_result;
			transform_result = transform_result.translate(transform.position.x, transform.position.y, transform.position.z);
			transform_result = transform_result.rotate(-transform.angle * M_PI / 180);
			transform_result = transform_result.scale(transform.scale.x, transform.scale.y);
			
			SpriteInfoBuffer *buffer = (SpriteInfoBuffer*)mappedResource.pData;
			buffer->transform = transform_result;

			this->deviceContext->Unmap(this->spriteInfoBuffer, 0);

			this->deviceContext->Draw(4, 0);
		}
	}

	void finish() const {
		// TODO(steven): SyncInterval results in different framerate on different monitors
		HRESULT result = this->swapChain->Present(2, 0);
		ASSERT_HRESULT(result)
	}

protected:
	void compileShaders() {
		ID3D10Blob *vertexShaderBlob; 
		ID3D10Blob *pixelShaderBlob;

		HRESULT result = D3DCompileFromFile(
			L"assets/shaders/shaders.hlsl", 
			nullptr, 
			nullptr, 
			"vertex", 
			"vs_4_0", 
			NULL, 
			NULL, 
			&vertexShaderBlob, 
			nullptr
		); 
		ASSERT_HRESULT(result)

		result = D3DCompileFromFile(
			L"assets/shaders/shaders.hlsl", 
			nullptr, 
			nullptr, 
			"pixel", 
			"ps_4_0", 
			NULL, 
			NULL, 
			&pixelShaderBlob, 
			nullptr
		);
		ASSERT_HRESULT(result)

		result = this->resources->device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(), 
			vertexShaderBlob->GetBufferSize(), 
			NULL, 
			&this->vertexShader
		);
		ASSERT_HRESULT(result)

		result = this->resources->device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(), 
			pixelShaderBlob->GetBufferSize(), 
			NULL, 
			&this->pixelShader
		);
		ASSERT_HRESULT(result)

		this->deviceContext->VSSetShader(this->vertexShader, 0, 0);
		this->deviceContext->PSSetShader(this->pixelShader, 0, 0);

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		result = this->resources->device->CreateInputLayout(
			inputElementDescriptions,
			2,
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			&this->vertexBufferLayout
		);
		ASSERT_HRESULT(result)

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

		HRESULT result = this->resources->device->CreateBlendState(&blendDescription, &this->blendState);
		ASSERT_HRESULT(result)
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

			HRESULT result = this->resources->device->CreateBuffer(
				&bufferDescription, 
				&subresourceData, 
				&this->constantBuffer
			);
			ASSERT_HRESULT(result)
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

			HRESULT result = this->resources->device->CreateBuffer(
				&bufferDescription, 
				&subresourceData, 
				&this->spriteInfoBuffer
			);
			ASSERT_HRESULT(result)
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
		HRESULT result = this->resources->device->CreateTexture2D(&depthStencilResourceDescription, NULL, &depthStencil);
		ASSERT_HRESULT(result)

		D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
		depthStencilDescription.DepthEnable = true;
		depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		result = this->resources->device->CreateDepthStencilState(&depthStencilDescription, &this->depthStencilState);
		ASSERT_HRESULT(result)

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription = {};
		depthStencilViewDescription.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilViewDescription.Texture2D.MipSlice = 0;

		result = this->resources->device->CreateDepthStencilView(
			depthStencil,
			&depthStencilViewDescription, 
			&this->depthStencilView
		);
		ASSERT_HRESULT(result)

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
		swapChainDescription.Flags = 0; 

		// TODO(steven): Printing the adapters for now. Come back and check if we
		// have any use for them.
		{
			IDXGIFactory *factory = nullptr;
			HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
			ASSERT_HRESULT(result)

			IDXGIAdapter *adapter;
			for (u8 i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
				DXGI_ADAPTER_DESC adapterDescription;

				result = adapter->GetDesc(&adapterDescription);
				ASSERT_HRESULT(result)

				LOG(L"%ls\n", adapterDescription.Description)
			}
		}
		
		D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
		HRESULT result = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			NULL,
			NULL,
			D3D11_SDK_VERSION,
			&swapChainDescription,
			&this->swapChain,
			&this->resources->device,
			NULL,
			&this->deviceContext
		);
		ASSERT_HRESULT(result)

		ID3D11Texture2D *backBuffer;
		result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		ASSERT_HRESULT(result)

		result = this->resources->device->CreateRenderTargetView(backBuffer, NULL, &this->renderView);
		ASSERT_HRESULT(result)

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
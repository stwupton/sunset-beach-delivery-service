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

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/window_config.hpp"
#include "common/ui_element.hpp"
#include "platform/windows/dx3d_sprite_loader.hpp"
#include "platform/windows/utils.hpp"
#include "platform/windows/sprite_vertex.hpp"
#include "types/core.hpp"
#include "types/matrix.hpp"
#include "types/vector.hpp"

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
	WCHAR d2dLocaleName[LOCALE_NAME_MAX_LENGTH];
	ID2D1RenderTarget *d2dRenderTarget;
	ID2D1SolidColorBrush *d2dSolidBrush;
	IDWriteFactory *dWriteFactory;

	// TODO(steven): delete
	struct {
		ID3D11VertexShader *vertexShader;
		ID3D11PixelShader *pixelShader;
		ID3D11InputLayout *vertexBufferLayout;
		ID3D11Buffer *infoBuffer;
	} spriteShader;

	struct {
		ID3D11VertexShader *vertexShader;
		ID3D11InputLayout *vertexBufferLayout;
		ID3D11Buffer *vertexBuffer;
		ID3D11PixelShader *pixelShader;
	} starfieldShader;

	ID3D11BlendState *blendState;
	ID3D11Buffer *constantBuffer;

public:
	~DirectXRenderer() {
		RELEASE_COM_OBJ(this->spriteShader.vertexShader)
		RELEASE_COM_OBJ(this->spriteShader.pixelShader)
		RELEASE_COM_OBJ(this->spriteShader.vertexBufferLayout)
		RELEASE_COM_OBJ(this->spriteShader.infoBuffer)
		RELEASE_COM_OBJ(this->starfieldShader.vertexShader)
		RELEASE_COM_OBJ(this->starfieldShader.pixelShader)
		RELEASE_COM_OBJ(this->starfieldShader.vertexBufferLayout)
		RELEASE_COM_OBJ(this->starfieldShader.vertexBuffer)
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

		GetUserDefaultLocaleName(this->d2dLocaleName, LOCALE_NAME_MAX_LENGTH);
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

	void drawUI(UIElement *uiElementBuffer, UINT bufferLength) const {
		// Horizontal text alignment
		DWRITE_TEXT_ALIGNMENT textAlignments[] = { 
			DWRITE_TEXT_ALIGNMENT_LEADING, 
			DWRITE_TEXT_ALIGNMENT_CENTER, 
			DWRITE_TEXT_ALIGNMENT_TRAILING 
		};

		// Vertical text alignment
		DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignments[] = {
			DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
			DWRITE_PARAGRAPH_ALIGNMENT_CENTER,
			DWRITE_PARAGRAPH_ALIGNMENT_FAR
		};

		IDWriteTextFormat *textFormat = nullptr;
		ID2D1StrokeStyle *strokeStyle = nullptr;
		ID2D1PathGeometry *geometry = nullptr;
		ID2D1GeometrySink *sink = nullptr;

		for (UINT i = 0; i < bufferLength; i++) {
			const UIElement &element = uiElementBuffer[i];
			this->d2dRenderTarget->BeginDraw();

			this->d2dSolidBrush->SetColor({ 
				element.common.color.r, 
				element.common.color.g, 
				element.common.color.b, 
				element.common.color.a 
			});

			if (element.type == UIType::text) {
				const UITextData &text = element.text;

				// TODO(steven): Re-use text formats
				HRESULT result = this->dWriteFactory->CreateTextFormat(
					text.font.data, 
					nullptr, 
					DWRITE_FONT_WEIGHT_REGULAR, 
					DWRITE_FONT_STYLE_NORMAL, 
					DWRITE_FONT_STRETCH_MEDIUM, 
					text.fontSize, 
					this->d2dLocaleName, 
					&textFormat
				);
				ASSERT_HRESULT(result)

				DWRITE_TEXT_ALIGNMENT textAlignment = textAlignments[(size_t)text.horizontalAlignment]; 
				result = textFormat->SetTextAlignment(textAlignment);
				ASSERT_HRESULT(result)

				DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment = paragraphAlignments[(size_t)text.verticalAlignment];
				textFormat->SetParagraphAlignment(paragraphAlignment);
				ASSERT_HRESULT(result)

				D2D1_RECT_F layoutRect = { 
					text.position.x, 
					text.position.y, 
					text.position.x + text.width, 
					text.position.y + text.height 
				};

				this->d2dRenderTarget->DrawText(
					text.text.data, 
					wcslen(text.text.data), 
					textFormat, 
					layoutRect, 
					this->d2dSolidBrush, 
					D2D1_DRAW_TEXT_OPTIONS_NO_SNAP, 
					DWRITE_MEASURING_MODE_NATURAL
				);

				RELEASE_COM_OBJ(textFormat)
			} else if (element.type == UIType::line) {
				const UILineData &line = element.line;

				this->d2dRenderTarget->DrawLine(
					{ line.start.x, line.start.y },
					{ line.end.x, line.end.y },
					this->d2dSolidBrush,
					line.thickness
				);
			} else if (element.type == UIType::circle) {
				const UICircleData &circle = element.circle;

				D2D1_ELLIPSE ellipse = {
					{ circle.position.x, circle.position.y },
					circle.radius,
					circle.radius
				};

				this->d2dRenderTarget->FillEllipse(ellipse, this->d2dSolidBrush);

				D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties = {};
				strokeStyleProperties.dashStyle = circle.strokeStyle == UIStrokeStyle::solid ? 
					D2D1_DASH_STYLE_SOLID : 
					D2D1_DASH_STYLE_DASH;
	
				HRESULT result = this->d2dFactory->CreateStrokeStyle(strokeStyleProperties, nullptr, 0, &strokeStyle);
				ASSERT_HRESULT(result)

				this->d2dSolidBrush->SetColor((const D2D1_COLOR_F*)&circle.strokeColor);
				this->d2dRenderTarget->DrawEllipse(ellipse, this->d2dSolidBrush, circle.strokeWidth, strokeStyle);

				RELEASE_COM_OBJ(strokeStyle)
			} else if (element.type == UIType::traingle) {
				HRESULT result = this->d2dFactory->CreatePathGeometry(&geometry);
				ASSERT_HRESULT(result)

				result = geometry->Open(&sink);
				ASSERT_HRESULT(result)
				
				D2D1_POINT_2F point = { element.triangle.points[0].x, element.triangle.points[0].y };
				sink->BeginFigure(point, D2D1_FIGURE_BEGIN_FILLED);

				point = { element.triangle.points[1].x, element.triangle.points[1].y };
				sink->AddLine(point);

				point = { element.triangle.points[2].x, element.triangle.points[2].y };
				sink->AddLine(point);

				sink->EndFigure(D2D1_FIGURE_END_CLOSED);

				result = sink->Close();
				ASSERT_HRESULT(result)

				this->d2dRenderTarget->FillGeometry(geometry, this->d2dSolidBrush);

				RELEASE_COM_OBJ(sink)
				RELEASE_COM_OBJ(geometry)
			} else if (element.type == UIType::rectangle) {
				const UIRectangleData &rectangle = element.rectangle;

				D2D1_ROUNDED_RECT rect = { 
					rectangle.position.x, 
					rectangle.position.y, 
					rectangle.position.x + rectangle.width, 
					rectangle.position.y + rectangle.height,
					rectangle.cornerRadius, 
					rectangle.cornerRadius 
				};

				this->d2dRenderTarget->FillRoundedRectangle(rect, this->d2dSolidBrush);

				D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties = {};
				strokeStyleProperties.dashStyle = rectangle.strokeStyle == UIStrokeStyle::solid ? 
					D2D1_DASH_STYLE_SOLID : 
					D2D1_DASH_STYLE_DASH;

				HRESULT result = this->d2dFactory->CreateStrokeStyle(strokeStyleProperties, nullptr, 0, &strokeStyle);
				ASSERT_HRESULT(result)

				this->d2dSolidBrush->SetColor((const D2D1_COLOR_F*)&rectangle.strokeColor);
				this->d2dRenderTarget->DrawRoundedRectangle(rect, this->d2dSolidBrush, rectangle.strokeWidth, strokeStyle);

				RELEASE_COM_OBJ(strokeStyle)
			}

			HRESULT result = this->d2dRenderTarget->EndDraw();
			ASSERT_HRESULT(result);
		}
	}

	void drawSprites(Sprite *sprites, UINT bufferLength) const {
		this->deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		this->deviceContext->IASetInputLayout(this->spriteShader.vertexBufferLayout);

		this->deviceContext->VSSetShader(this->spriteShader.vertexShader, nullptr, 0);
		this->deviceContext->VSSetConstantBuffers(0, 1, &this->constantBuffer);
		this->deviceContext->VSSetConstantBuffers(1, 1, &this->spriteShader.infoBuffer);

		this->deviceContext->PSSetShader(this->spriteShader.pixelShader, nullptr, 0);

		for (UINT i = 0; i < bufferLength; i++) {
			const Sprite &sprite = sprites[i];
			const Dx3dSpriteResource &textureReference = this->resources->spriteResources[(size_t)sprite.assetId];

			const UINT stride = sizeof(SpriteVertex);
			const UINT offset = 0;
			this->deviceContext->IASetVertexBuffers(0, 1, &textureReference.vertexBuffer, &stride, &offset);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT result = this->deviceContext->Map(
				this->spriteShader.infoBuffer, 
				NULL, 
				D3D11_MAP_WRITE_DISCARD, 
				NULL, 
				&mappedResource
			);
			ASSERT_HRESULT(result)

			Mat4x4<f32> transform;
			transform = transform.translate(sprite.position.x, sprite.position.y, sprite.position.z);
			transform = transform.rotate(-sprite.angle * M_PI / 180);
			transform = transform.scale(sprite.scale.x, sprite.scale.y);
			
			SpriteInfoBuffer *buffer = (SpriteInfoBuffer*)mappedResource.pData;
			buffer->transform = transform;

			this->deviceContext->Unmap(this->spriteShader.infoBuffer, 0);

			this->deviceContext->PSSetShaderResources(0, 1, &textureReference.texture2dView);
			this->deviceContext->Draw(4, 0);
		}
	}

	void drawStarfield() const {
		this->deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		this->deviceContext->IASetInputLayout(this->starfieldShader.vertexBufferLayout);

		const UINT stride = sizeof(Vec3<f32>);
		const UINT offset = 0;
		this->deviceContext->IASetVertexBuffers(0, 1, &this->starfieldShader.vertexBuffer, &stride, &offset);

		this->deviceContext->VSSetShader(this->starfieldShader.vertexShader, nullptr, 0);
		this->deviceContext->PSSetShader(this->starfieldShader.pixelShader, nullptr, 0);

		this->deviceContext->Draw(4, 0);
	}

	void finish() const {
		// TODO(steven): SyncInterval results in different framerate on different monitors
		HRESULT result = this->swapChain->Present(1, 0);
		ASSERT_HRESULT(result)
	}

	void start() const {
		const Rgba clearColor(0.0f, 0.2f, 0.4f, 1.0f);
		this->deviceContext->ClearRenderTargetView(this->renderView, (f32*)&clearColor);

		this->deviceContext->ClearDepthStencilView(
			this->depthStencilView, 
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
			1.0f, 
			0
		);

		this->deviceContext->OMSetBlendState(this->blendState, 0, 0xffffffff);
		this->deviceContext->OMSetDepthStencilState(this->depthStencilState, NULL);
		this->deviceContext->OMSetRenderTargets(1, &this->renderView, this->depthStencilView);
	}

protected:
	void compileSpriteShaders() {
		ID3D10Blob *blob;
		ID3D10Blob *errorBlob;

		const wchar_t *fileName = GET_ASSET_PATH("shaders/sprite_shader.hlsl");
		HRESULT result = D3DCompileFromFile(
			fileName, 
			nullptr, 
			nullptr, 
			"vertex", 
			"vs_4_0", 
			NULL, 
			NULL, 
			&blob, 
			&errorBlob
		); 
		DETAILED_ASSERT_HRESULT(result, L"%hs\n", errorBlob->GetBufferPointer())

		result = this->resources->device->CreateVertexShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), 
			NULL, 
			&this->spriteShader.vertexShader
		);
		ASSERT_HRESULT(result)

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		result = this->resources->device->CreateInputLayout(
			inputElementDescriptions,
			2,
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			&this->spriteShader.vertexBufferLayout
		);
		ASSERT_HRESULT(result)

		result = D3DCompileFromFile(
			fileName, 
			nullptr, 
			nullptr, 
			"pixel", 
			"ps_4_0", 
			NULL, 
			NULL, 
			&blob,
			&errorBlob
		);
		DETAILED_ASSERT_HRESULT(result, L"%hs\n", errorBlob->GetBufferPointer())

		result = this->resources->device->CreatePixelShader(
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			NULL,
			&this->spriteShader.pixelShader
		);
		ASSERT_HRESULT(result)
	}

	void compileStarfieldShaders() {
		ID3D10Blob *blob;
		ID3DBlob *errorBlob;

		const wchar_t *fileName = GET_ASSET_PATH("shaders/starfield_shader.hlsl");
		HRESULT result = D3DCompileFromFile(
			fileName, 
			nullptr, 
			nullptr, 
			"vertex", 
			"vs_4_0", 
			NULL, 
			NULL, 
			&blob, 
			&errorBlob
		); 
		DETAILED_ASSERT_HRESULT(result, L"%hs\n", errorBlob->GetBufferPointer())

		result = this->resources->device->CreateVertexShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), 
			NULL, 
			&this->starfieldShader.vertexShader
		);
		ASSERT_HRESULT(result)

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		result = this->resources->device->CreateInputLayout(
			inputElementDescriptions,
			1,
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			&this->starfieldShader.vertexBufferLayout
		);
		ASSERT_HRESULT(result)

		const Vec3<f32> vertices[] = { 
			{ -1.0f, -1.0f },
			{ -1.0f, 1.0f }, 
			{ 1.0f, -1.0f }, 
			{ 1.0f, 1.0f } 
		};
		D3D11_BUFFER_DESC bufferDescription = {};
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(vertices);
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = vertices;

		result = this->resources->device->CreateBuffer(
			&bufferDescription, 
			&subresourceData, 
			&this->starfieldShader.vertexBuffer
		);
		ASSERT_HRESULT(result)

		result = D3DCompileFromFile(
			fileName, 
			nullptr, 
			nullptr, 
			"pixel", 
			"ps_4_0", 
			NULL, 
			NULL, 
			&blob, 
			&errorBlob
		);
		DETAILED_ASSERT_HRESULT(result, L"%hs\n", errorBlob->GetBufferPointer())

		result = this->resources->device->CreatePixelShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), 
			NULL, 
			&this->starfieldShader.pixelShader
		);
		ASSERT_HRESULT(result)
	}

	void compileShaders() {
		this->compileSpriteShaders();
		this->compileStarfieldShaders();
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
				&this->spriteShader.infoBuffer
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
		HRESULT result = this->resources->device->CreateTexture2D(
			&depthStencilResourceDescription, 
			NULL, 
			&depthStencil
		);
		ASSERT_HRESULT(result)

		D3D11_DEPTH_STENCIL_DESC depthStencilDescription = {};
		depthStencilDescription.DepthEnable = true;
		depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		result = this->resources->device->CreateDepthStencilState(
			&depthStencilDescription, 
			&this->depthStencilState
		);
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
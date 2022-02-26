#pragma once

#include <cassert>
#include <combaseapi.h>
#include <wincodec.h>
#include <d3d11.h>
#include <malloc.h>

#include "common/asset_definitions.hpp"
#include "common/load_queue.hpp"
#include "platform/windows/directx_resources.hpp"
#include "platform/windows/utils.hpp"
#include "platform/windows/sprite_vertex.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

typedef LoadQueue<TextureAssetId, 8> TextureLoadQueue;

class Dx3dSpriteLoader {
protected:
	IWICImagingFactory *imagingFactory;
	DirectXResources *resources;

public:
	~Dx3dSpriteLoader() {
		this->unload();
		RELEASE_COM_OBJ(this->imagingFactory)
	}

	void initialise(DirectXResources *resources) {
		this->resources = resources;

		HRESULT result = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof(IWICImagingFactory), 
			(void**)&this->imagingFactory
		);
		ASSERT_HRESULT(result)
	}

	// TODO(steven): Load in a seperate thread
  void load(TextureLoadQueue *loadQueue) {
		if (loadQueue->length == 0) {
			return;
		}

		// NOTE(steven): Increase size of buffer if needed
		BYTE *buffer = (BYTE*)malloc(100000000);

		for (TextureAssetId assetId : *loadQueue) {
			Dx3dSpriteResource &spriteResource = resources->spriteResources[(size_t)assetId];
			if (spriteResource.loaded) {
				continue;
			}

			LPCWSTR fileName = textureNames[(size_t)assetId];

			IWICBitmapDecoder *bitmapDecoder;
			HRESULT result = imagingFactory->CreateDecoderFromFilename(
				fileName, 
				NULL, 
				GENERIC_READ, 
				WICDecodeMetadataCacheOnLoad, 
				&bitmapDecoder
			);
			ASSERT_HRESULT(result)

			IWICBitmapFrameDecode *frameDecode;
			result = bitmapDecoder->GetFrame(0, &frameDecode);
			ASSERT_HRESULT(result)

			WICPixelFormatGUID wicPixelFormat;
			result = frameDecode->GetPixelFormat(&wicPixelFormat);
			ASSERT_HRESULT(result)

			DXGI_FORMAT dxgiFormat;
			const bool formatConverted = this->getDxgiFormat(&wicPixelFormat, &dxgiFormat);
			const UINT bitsPerPixel = this->getBitsPerPixel(wicPixelFormat);

			UINT width = 0, height = 0;
			frameDecode->GetSize(&width, &height);

			spriteResource.vertexBuffer = this->createVertexBuffer(width, height);

			const UINT stride = bitsPerPixel / 8; 
			const UINT rowStride = stride * width;
			const UINT bufferSize = width * height * stride;
			this->createTextureBuffer(
				frameDecode, 
				buffer, 
				bufferSize, 
				rowStride, 
				wicPixelFormat, 
				formatConverted
			);

			spriteResource.texture2d = this->createTexture2d(buffer, dxgiFormat, width, height, rowStride);
			spriteResource.texture2dView = this->createTexture2dView(spriteResource.texture2d, dxgiFormat);
			spriteResource.loaded = true;

			RELEASE_COM_OBJ(bitmapDecoder)
			RELEASE_COM_OBJ(frameDecode)
		}

		loadQueue->clear();
		free(buffer);
	}

protected:
	WICPixelFormatGUID convertWic(const WICPixelFormatGUID &pixelFormat) const {
		if (pixelFormat == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
		if (pixelFormat == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray; 
		if (pixelFormat == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray; 
		if (pixelFormat == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf; 
		if (pixelFormat == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat; 
		if (pixelFormat == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
		if (pixelFormat == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
		if (pixelFormat == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA; 
		if (pixelFormat == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf; 
		if (pixelFormat == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat; 
		if (pixelFormat == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat; 
		if (pixelFormat == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat; 
		if (pixelFormat == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat; 
		if (pixelFormat == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat; 
		if (pixelFormat == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat32bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
		if (pixelFormat == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
		if (pixelFormat == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf; 
#endif
		return pixelFormat;
	}

	DXGI_FORMAT wic2DxgiFormat(const WICPixelFormatGUID &pixelFormat) const {
		if (pixelFormat == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (pixelFormat == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
		if (pixelFormat == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppRGBE) return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		if (pixelFormat == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
		if (pixelFormat == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
		if (pixelFormat == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
		if (pixelFormat == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
		if (pixelFormat == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
		if (pixelFormat == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
		if (pixelFormat == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;
		if (pixelFormat == GUID_WICPixelFormat96bppRGBFloat) return DXGI_FORMAT_R32G32B32_FLOAT;

		return DXGI_FORMAT_UNKNOWN;
	}

	ID3D11Buffer *createVertexBuffer(UINT width, UINT height) const {
		f32 halfWidth = (f32)width * 0.5;
		f32 halfHeight = (f32)height * 0.5;
		SpriteVertex vertices[] = {
			{ Vec3<f32>(-halfWidth, -halfHeight, 0.0f), Vec2<f32>(0.0f, 1.0f) },
			{ Vec3<f32>(-halfWidth, halfHeight, 0.0f), Vec2<f32>(0.0f, 0.0f) },
			{ Vec3<f32>(halfWidth, -halfHeight, 0.0f), Vec2<f32>(1.0f, 1.0f) },
			{ Vec3<f32>(halfWidth, halfHeight, 0.0f), Vec2<f32>(1.0f, 0.0f) },
		};

		D3D11_BUFFER_DESC bufferDescription = {};
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(vertices);
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = vertices;

		ID3D11Buffer *vertexBuffer;
		HRESULT result = this->resources->device->CreateBuffer(
			&bufferDescription, 
			&subresourceData, 
			&vertexBuffer
		);
		ASSERT_HRESULT(result)

		return vertexBuffer;
	}

	ID3D11Texture2D *createTexture2d(
		BYTE *buffer, 
		const DXGI_FORMAT dxgiFormat, 
		const UINT width, 
		const UINT height,
		const UINT rowStride
	) const {
		D3D11_TEXTURE2D_DESC textureDescription = {};
		textureDescription.Width = width;
		textureDescription.Height = height;
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = dxgiFormat;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = buffer;
		subresourceData.SysMemPitch = rowStride;

		ID3D11Texture2D *texture2d;
		HRESULT result = this->resources->device->CreateTexture2D(
			&textureDescription, 
			&subresourceData, 
			&texture2d
		);
		ASSERT_HRESULT(result)

		return texture2d;
	}

	ID3D11ShaderResourceView *createTexture2dView(
		ID3D11Texture2D *texture2d, 
		DXGI_FORMAT dxgiFormat
	) const {
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription = {};
		resourceViewDescription.Format = dxgiFormat;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MipLevels = 1;

		ID3D11ShaderResourceView *texture2dView;
		HRESULT result = this->resources->device->CreateShaderResourceView(
			texture2d, 
			&resourceViewDescription, 
			&texture2dView
		);
		ASSERT_HRESULT(result)

		return texture2dView;
	}

	void createTextureBuffer(
		IWICBitmapFrameDecode *frameDecode, 
		BYTE *buffer, 
		const UINT bufferSize, 
		const UINT rowStride, 
		const WICPixelFormatGUID &wicPixelFormat, 
		const bool formatConverted
	) const {
		if (formatConverted) {
			IWICFormatConverter *formatConverter;
			HRESULT result = imagingFactory->CreateFormatConverter(&formatConverter);
			ASSERT_HRESULT(result)

			result = formatConverter->Initialize(
				frameDecode, 
				wicPixelFormat, 
				WICBitmapDitherTypeErrorDiffusion, 
				0, 
				0, 
				WICBitmapPaletteTypeCustom
			);
			ASSERT_HRESULT(result)

			result = formatConverter->CopyPixels(NULL, rowStride, bufferSize, buffer);
			ASSERT_HRESULT(result)

			RELEASE_COM_OBJ(formatConverter);
		} else {
			HRESULT result = frameDecode->CopyPixels(NULL, rowStride, bufferSize, buffer);
			ASSERT_HRESULT(result)
		}
	}

	UINT getBitsPerPixel(const WICPixelFormatGUID &wicPixelFormat) const {
		IWICComponentInfo *componentInfo;
		HRESULT result = imagingFactory->CreateComponentInfo(wicPixelFormat, &componentInfo);
		ASSERT_HRESULT(result)

		IWICPixelFormatInfo *formatInfo;
		result = componentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), (void**)&formatInfo); 
		ASSERT_HRESULT(result)

		UINT bitsPerPixel;
		result = formatInfo->GetBitsPerPixel(&bitsPerPixel);
		ASSERT_HRESULT(result)

		RELEASE_COM_OBJ(componentInfo)
		RELEASE_COM_OBJ(formatInfo)

		return bitsPerPixel;
	}

	bool getDxgiFormat(WICPixelFormatGUID *wicPixelFormat, DXGI_FORMAT *dxgiFormat) const {
		bool wicConverted = false;
		WICPixelFormatGUID wic = *wicPixelFormat;
		DXGI_FORMAT dxgi = *dxgiFormat;

		dxgi = this->wic2DxgiFormat(wic);
		if (dxgi == DXGI_FORMAT_UNKNOWN) {
			wic = this->convertWic(wic);
			wicConverted = true;

			dxgi = this->wic2DxgiFormat(wic);
			assert(dxgi != DXGI_FORMAT_UNKNOWN);
		}

		*wicPixelFormat = wic;
		*dxgiFormat = dxgi;

		return wicConverted;
	}

	void unload() {
		for (Dx3dSpriteResource &resource : this->resources->spriteResources) {
			RELEASE_COM_OBJ(resource.texture2d)
			RELEASE_COM_OBJ(resource.texture2dView)
			RELEASE_COM_OBJ(resource.vertexBuffer)
			resource = {};
		}
	}
};
#pragma once

#include <cassert>
#include <combaseapi.h>
#include <wincodec.h>
#include <d3d11.h>
#include <malloc.h>

#include "platform/windows/utils.cpp"

struct Dx3dSpriteInfo {
	ID3D11Texture2D *texture2d;
	ID3D11ShaderResourceView *texture2dView;
};

class Dx3dSpriteLoader {
protected:
	ID3D11Device *device;

public:
	void initialise(ID3D11Device *device) {
		this->device = device;
	}

  Dx3dSpriteInfo load(LPCWSTR fileName) {
		Dx3dSpriteInfo spriteInfo = {};

		IWICImagingFactory *imagingFactory;
		CoCreateInstance(
			CLSID_WICImagingFactory, 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof(IWICImagingFactory), 
			(void**)&imagingFactory
		);

		IWICBitmapDecoder *bitmapDecoder;
		imagingFactory->CreateDecoderFromFilename(
			fileName, 
			NULL, 
			GENERIC_READ, 
			WICDecodeMetadataCacheOnLoad, 
			&bitmapDecoder
		);

		IWICBitmapFrameDecode *frameDecode;
		bitmapDecoder->GetFrame(0, &frameDecode);

		WICPixelFormatGUID wicPixelFormat;
		frameDecode->GetPixelFormat(&wicPixelFormat);

		UINT bitsPerPixel = 0;
		{
			IWICComponentInfo *componentInfo;
			imagingFactory->CreateComponentInfo(wicPixelFormat, &componentInfo);

			IWICPixelFormatInfo *formatInfo;
			componentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), (void**)&formatInfo); 

			formatInfo->GetBitsPerPixel(&bitsPerPixel);

			RELEASE_COM_OBJ(componentInfo)
			RELEASE_COM_OBJ(formatInfo)
		}

		UINT width = 0, height = 0;
		frameDecode->GetSize(&width, &height);

		const UINT stride = bitsPerPixel / 8; 
		const UINT bufferSize = width * height * stride;
		BYTE *buffer = (BYTE*)malloc(bufferSize);
		frameDecode->CopyPixels(NULL, stride * width, bufferSize, buffer);

		const DXGI_FORMAT dxgiFormat = this->wic2DxgiFormat(wicPixelFormat);
		assert(dxgiFormat != DXGI_FORMAT_UNKNOWN);

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
		subresourceData.SysMemPitch = width * stride;

		this->device->CreateTexture2D(
			&textureDescription, 
			&subresourceData, 
			&spriteInfo.texture2d
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription = {};
		resourceViewDescription.Format = dxgiFormat;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MipLevels = 1;

		this->device->CreateShaderResourceView(
			spriteInfo.texture2d, 
			&resourceViewDescription, 
			&spriteInfo.texture2dView
		);

		RELEASE_COM_OBJ(imagingFactory)
		RELEASE_COM_OBJ(bitmapDecoder)
		RELEASE_COM_OBJ(frameDecode)
		free(buffer);

		return spriteInfo;
	}

	void unload(Dx3dSpriteInfo *spriteInfoBuffer, u16 bufferLength) {
		for (u16 i = 0; i < bufferLength; i++) {
			Dx3dSpriteInfo &info = spriteInfoBuffer[i];
			RELEASE_COM_OBJ(info.texture2d)
			RELEASE_COM_OBJ(info.texture2dView)
		}
	}

protected:
	DXGI_FORMAT wic2DxgiFormat(const WICPixelFormatGUID &pixelFormat) const {
		// TODO(steven): Why can't this be a switch statement? But it doesn't matter too much
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
};
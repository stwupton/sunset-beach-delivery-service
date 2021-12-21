#pragma once

#include <d3d11.h>

#include "common/asset_definitions.hpp"

struct Dx3dSpriteResource {
	ID3D11Texture2D *texture2d;
	ID3D11ShaderResourceView *texture2dView;
	ID3D11Buffer *vertexBuffer;
};

struct DirectXResources {
	ID3D11Device *device;
	Dx3dSpriteResource spriteResources[(size_t)TextureAssetId::_length];
};
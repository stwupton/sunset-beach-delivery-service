#pragma once

#include <xaudio2.h>
#include "common/asset_definitions.hpp"

//struct SoundResource {
//	/*ID3D11Texture2D* texture2d;
//	ID3D11ShaderResourceView* texture2dView;
//	ID3D11Buffer* vertexBuffer;*/
//};

struct SoundResources {
	/*ID3D11Device* device;*/
	/*Dx3dSpriteResource spriteResources[(size_t)TextureAssetId::_length];*/
	IXAudio2SourceVoice* voices[(size_t)SoundAssetId::_length] = {};
};
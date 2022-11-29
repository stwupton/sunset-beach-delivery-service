#pragma once

#include "types/core.hpp"

#define GET_ASSET_PATH(_path) L"" ASSET_PATH _path

enum class TextureAssetId : u8 {
	ship,
	enemyShip,
	background,
	marketPlace1,
	_length
};

namespace _TextureAssetFileNames {
	const wchar_t *ship = GET_ASSET_PATH("img/ship.png");
	const wchar_t *enemyShip = GET_ASSET_PATH("img/enemy_ship.png");
	const wchar_t *background = GET_ASSET_PATH("img/starry_background.jpg");
	const wchar_t *marketPlace1 = GET_ASSET_PATH("img/marketPlace1.jpg");
};

const wchar_t *textureNames[] = {
	_TextureAssetFileNames::ship,
	_TextureAssetFileNames::enemyShip,
	_TextureAssetFileNames::background,
	_TextureAssetFileNames::marketPlace1
};

enum class SoundAssetId : u8 {
	left,
	right,
	stereo,
	doomDeath,
	cha_ching,
	wahoo
};

namespace _SoundAssetFileNames {
	const wchar_t *left = GET_ASSET_PATH("music/Left.wav");
	const wchar_t *right = GET_ASSET_PATH("music/Right.wav");
	const wchar_t *stereo = GET_ASSET_PATH("music/Stereo.wav");
	const wchar_t *doomDeath = GET_ASSET_PATH("music/sound1.wav");
	const wchar_t *cha_ching = GET_ASSET_PATH("music/cha_ching.wav");
	const wchar_t *wahoo = GET_ASSET_PATH("music/wahoo.wav");
};

const wchar_t *soundNames[] = {
	_SoundAssetFileNames::left,
	_SoundAssetFileNames::right,
	_SoundAssetFileNames::stereo,
	_SoundAssetFileNames::doomDeath,
	_SoundAssetFileNames::cha_ching,
	_SoundAssetFileNames::wahoo
};

enum class MusicAssetId : u8 {
	mars,
	none
};

namespace _MusicAssetFileNames {
	// Source: https://www.free-stock-music.com/arthur-vyncke-approaching-mars.html
	const wchar_t *mars = GET_ASSET_PATH("music/mars.wav");
};

const wchar_t *musicNames[] = {
	_MusicAssetFileNames::mars
};
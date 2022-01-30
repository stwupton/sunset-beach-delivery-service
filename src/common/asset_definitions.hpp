#pragma once

#include "types/core.hpp"

enum class TextureAssetId : u8 {
	ship,
	enemyShip,
	background,
	_length
};

namespace _TextureAssetFileNames {
	const wchar_t *ship = L"assets/img/ship.png";
	const wchar_t *enemyShip = L"assets/img/enemy_ship.png";
	const wchar_t *background = L"assets/img/starry_background.jpg";
};

const wchar_t *textureNames[] = {
	_TextureAssetFileNames::ship,
	_TextureAssetFileNames::enemyShip,
	_TextureAssetFileNames::background
};

enum class SoundAssetId : u8 {
	left,
	right,
	stereo,
	doomDeath
};

namespace _SoundAssetFileNames {
	const wchar_t *left = L"assets/music/Left.wav";
	const wchar_t *right = L"assets/music/Right.wav";
	const wchar_t *stereo = L"assets/music/Stereo.wav";
	const wchar_t *doomDeath = L"assets/music/sound1.wav";
};

const wchar_t *soundNames[] = {
	_SoundAssetFileNames::left,
	_SoundAssetFileNames::right,
	_SoundAssetFileNames::stereo,
	_SoundAssetFileNames::doomDeath
};

enum class MusicAssetId : u8 {
	mars,
	none
};

namespace _MusicAssetFileNames {
	// Source: https://www.free-stock-music.com/arthur-vyncke-approaching-mars.html
	const wchar_t *mars = L"assets/music/mars.wav";
};

const wchar_t *musicNames[] = {
	_MusicAssetFileNames::mars
};
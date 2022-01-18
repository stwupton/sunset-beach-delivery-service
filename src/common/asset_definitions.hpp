#pragma once

#include "types/core.hpp"

enum class TextureAssetId : u8 {
	ship,
	enemyShip,
	background,
	_length
};

namespace _TextureAssetFileNames {
	static const wchar_t* ship = L"assets/img/ship.png";
	static const wchar_t* enemyShip = L"assets/img/enemy_ship.png";
	static const wchar_t* background = L"assets/img/starry_background.jpg";
};

static const wchar_t* textureNames[] = {
	_TextureAssetFileNames::ship,
	_TextureAssetFileNames::enemyShip,
	_TextureAssetFileNames::background
};

enum class SoundAssetId : u8 {
	left,
	right,
	stereo,
	_length
};

namespace _SoundAssetFileNames {
	static const wchar_t* left = L"assets/img/music/Left.wav";
	static const wchar_t* right = L"assets/music/Right.wav";
	static const wchar_t* stereo = L"assets/music/Stereo.wav";
};

static const wchar_t* soundNames[] = {
	_SoundAssetFileNames::left,
	_SoundAssetFileNames::right,
	_SoundAssetFileNames::stereo
};
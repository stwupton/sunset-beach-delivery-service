#pragma once

#include "types/core.hpp"

enum class TextureAssetId : u8 {
	ship,
	enemyShip,
	background,
	_length
};

namespace _TextureAssetFileNames {
	const wchar_t* ship = L"assets/img/ship.png";
	const wchar_t* enemyShip = L"assets/img/enemy_ship.png";
	const wchar_t* background = L"assets/img/starry_background.jpg";
};

const wchar_t* textureNames[] = {
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
	const wchar_t* left = L"assets/img/music/Left.wav";
	const wchar_t* right = L"assets/music/Right.wav";
	const wchar_t* stereo = L"assets/music/Stereo.wav";
};

const wchar_t* soundNames[] = {
	_SoundAssetFileNames::left,
	_SoundAssetFileNames::right,
	_SoundAssetFileNames::stereo
};

enum class MusicAssetId : u8 {
	mars,
	_length
};

namespace _MusicAssetFileNames {
	const wchar_t* mars = L"assets/img/music/mars.wav";
};

const wchar_t* musicNames[] = {
	_MusicAssetFileNames::mars
};
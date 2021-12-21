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
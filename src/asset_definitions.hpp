#pragma once

#include "types/core.hpp"

enum AssetId : u8 {
	ship,
	enemyShip,
	background,
	_length
};

namespace _AssetFileNames {
	const wchar_t *ship = L"assets/img/ship.png";
	const wchar_t *enemyShip = L"assets/img/enemy_ship.png";
	const wchar_t *background = L"assets/img/starry_background.jpg";
};

const wchar_t *fileNames[] = {
	_AssetFileNames::ship, 
	_AssetFileNames::enemyShip, 
	_AssetFileNames::background
};
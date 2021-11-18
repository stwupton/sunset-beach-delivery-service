#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/load_queue.cpp"
#include "common/projectile.hpp"
#include "common/ship_target.hpp"
#include "common/weapon.hpp"
#include "common/ui_element_buffer.cpp"
#include "types/array.hpp"

typedef Array<ShipTarget, 20> ShipTargets;
typedef Array<Weapon, 20> Weapons;
typedef Array<Sprite, 10> SpriteBuffer;
typedef Array<Projectile, 100> Projectiles;

struct GameState {
	Input input;
	LoadQueue loadQueue;
	ShipTargets shipTargets;
	Weapons weapons;
	Projectiles projectiles;
	SpriteBuffer sprites;
	UIElementBuffer uiElements;
};
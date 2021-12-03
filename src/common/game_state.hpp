#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/event.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/load_queue.cpp"
#include "common/projectile.hpp"
#include "common/ship.hpp"
#include "common/ship_target.hpp"
#include "common/weapon.hpp"
#include "common/ui_element_buffer.cpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer; 

struct GameState {
	Input input;
	LoadQueue loadQueue;
	Events events;
	Array<Ship, 2> ships;
	Array<Projectile, 100> projectiles;
	Array<AimlessProjectile, 100> aimlessProjectiles;
	Array<Sprite, 10> sprites;
	UIElementBuffer uiElements;
};
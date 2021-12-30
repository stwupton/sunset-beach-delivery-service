#pragma once

#include <cmath>

#include "common/editor_state.hpp"
#include "common/asset_definitions.hpp"
#include "common/event.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/load_queue.hpp"
#include "common/projectile.hpp"
#include "common/ship.hpp"
#include "common/ship_target.hpp"
#include "common/templates.hpp"
#include "common/weapon.hpp"
#include "common/ui_element_buffer.hpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer; 
typedef LoadQueue<TextureAssetId, 8> TextureLoadQueue;

struct GameState {
	Templates templates;
	Input input;
	TextureLoadQueue textureLoadQueue;
	Events events;
	Array<Ship, 2> allyShips;
	Array<Ship, 2> enemyShips;
	Array<Projectile, 100> projectiles;
	Array<AimlessProjectile, 100> aimlessProjectiles;
	Array<Sprite, 10> sprites;
	UIElementBuffer uiElements;

#ifdef DEBUG
	EditorState editorState;
#endif
};
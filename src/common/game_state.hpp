#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/load_queue.cpp"
#include "common/ship_target.hpp"
#include "common/ui_element_buffer.cpp"
#include "types/array.hpp"

typedef Array<ShipTarget, 20> ShipTargets;
typedef Array<Sprite, 10> SpriteBuffer;

struct GameState {
	Input input;
	LoadQueue loadQueue;
	ShipTargets shipTargets;
	SpriteBuffer sprites;
	UIElementBuffer uiElements;
};
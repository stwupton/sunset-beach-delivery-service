#pragma once

#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/ui_element.hpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer;
typedef Array<UIElement, 10> UIElementBuffer;

struct GameState {
	SpriteBuffer sprites;
	UIElementBuffer uiElements;
	Input input;
};
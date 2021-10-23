#include "common/sprite.hpp"
#include "common/input.hpp"

struct GameData {
	Sprite sprites[10] = {};
	u8 spriteLength = 0;
	Input input;
};
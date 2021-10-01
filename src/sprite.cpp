#pragma once

#include "types.hpp"

struct Sprite {
	Vec3<f32> position;
	Vec2<f32> scale;
	f32 angle;
	void *textureReference;
};
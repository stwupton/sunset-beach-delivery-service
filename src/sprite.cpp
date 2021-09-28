#pragma once

#include "types.hpp"

struct Sprite {
	Mat4x4<f32> transform;
	void *textureReference;
};
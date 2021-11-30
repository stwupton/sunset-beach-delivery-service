#pragma once

#include "common/window_config.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

Vec2<f32> gameToScreen(const Vec3<f32> &x) {
	return Vec2<f32>(
		screenWidth * 0.5f + x.x,
		screenHeight * 0.5f - x.y
	);
}
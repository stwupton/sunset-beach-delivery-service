#pragma once

#include "types/core.hpp"
#include "types/vector.hpp"

bool boxCollision(Vec2<f32> point, Vec2<f32> position, f32 width, f32 height) {
	return (
		point.x > position.x && point.x < position.x + width &&
		point.y > position.y && point.y < position.y + height
	);
}
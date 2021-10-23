#pragma once

#include "types/core.hpp"
#include "types/vector.hpp"

struct Input {
	Vec2<f64> mouse;
	Vec2<f64> previousMouse;
	bool primaryButtonDown;

	Vec2<f64> mouseMovement() const {
		return Vec2<f64>(
			this->mouse.x - this->previousMouse.x,
			this->mouse.y - this->previousMouse.y
		);
	}
};
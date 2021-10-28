#pragma once

#include "types/core.hpp"
#include "types/vector.hpp"

struct MouseButtonState { 
	bool down; 
	Vec2<f32> start; 
};

struct Input {
	Vec2<f32> mouse;
	Vec2<f32> previousMouse;
	MouseButtonState primaryButton;

	Vec2<f64> mouseMovement() const {
		return Vec2<f64>(
			this->mouse.x - this->previousMouse.x,
			this->mouse.y - this->previousMouse.y
		);
	}
};
#pragma once

#include "types/core.hpp"
#include "types/vector.hpp"

struct ButtonState { 
	bool down;
	bool wasDown;
	Vec2<f32> start; 
	Vec2<f32> end; 
};

struct Input {
	Vec2<f32> mouse;
	Vec2<f32> previousMouse;
	ButtonState primaryButton;

	Vec2<f64> mouseMovement() const {
		return Vec2<f64>(
			this->mouse.x - this->previousMouse.x,
			this->mouse.y - this->previousMouse.y
		);
	}
};
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
	wchar_t keyDown = '\0';
	Vec2<f32> mouse;
	Vec2<f32> previousMouse;
	ButtonState primaryButton;

	Vec2<f32> mouseMovement() const {
		return mouse - previousMouse;
	}
};
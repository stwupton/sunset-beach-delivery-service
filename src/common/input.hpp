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
};
#pragma once

#include "types/array.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"

enum UIType {
	text,
	line
};

struct UITextData {
	string16<100> text;
	f32 fontSize; 
	Vec2<f32> position;
	f32 width, height;
};

struct UILineData {
	f32 thickness;
	Vec2<f32> start;
	Vec2<f32> end;
};

struct UIElement {
	UIType type;
	union {
		UITextData text;
		UILineData line;
	};
};
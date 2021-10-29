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

	// NOTE(steven): Data should always be the size of the largest ui data structure
	char data[sizeof(UITextData)];
};
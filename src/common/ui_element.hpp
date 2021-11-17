#pragma once

#include "types/array.hpp"
#include "types/core.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"

enum UIType: u8 {
	text,
	line,
	circle
};

struct UICommonData {
	Rgba color;
};

struct UITextData : UICommonData {
	string16<100> text;
	f32 fontSize; 
	Vec2<f32> position;
	f32 width, height;
};

struct UILineData : UICommonData {
	f32 thickness;
	Vec2<f32> start;
	Vec2<f32> end;
};

enum UICircleStyle : u8 {
	solid,
	dotted
};

struct UICircleData : UICommonData {
	f32 thickness;
	f32 radius;
	Vec2<f32> position;
	UICircleStyle style = UICircleStyle::solid;
};

struct UIElement {
	UIType type;
	union {
		UICommonData common;
		UITextData text;
		UILineData line;
		UICircleData circle;
	};
};
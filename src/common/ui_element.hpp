#pragma once

#include "types/array.hpp"
#include "types/core.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"
#include "utils/collision.hpp"

enum class UIType: u8 {
	text,
	line,
	circle,
	button
};

struct UICommonData {
	Rgba color;
};

enum class UITextAlignment {
	start,
	middle,
	end
};

struct UITextData : UICommonData {
	string16<100> text;
	f32 fontSize; 
	Vec2<f32> position;
	f32 width, height;
	UITextAlignment horizontalAlignment = UITextAlignment::start;
	UITextAlignment verticalAlignment = UITextAlignment::start;
};

struct UILineData : UICommonData {
	f32 thickness;
	Vec2<f32> start;
	Vec2<f32> end;
};

enum class UICircleStyle : u8 {
	solid,
	dotted
};

struct UICircleData : UICommonData {
	f32 thickness;
	f32 radius;
	Vec2<f32> position;
	UICircleStyle style = UICircleStyle::solid;
};

struct UIButtonLabelData : UICommonData {
	f32 fontSize;
	string16<32> text;
};

struct UIButtonData : UICommonData {
	UIButtonLabelData label;
	Vec2<f32> position;
	f32 width, height;

	bool clicked(ButtonState state) const {
		return (
			state.wasDown && 
			!state.down &&
			boxCollision(state.start, this->position, this->width, this->height) &&
			boxCollision(state.start, this->position, this->width, this->height)
		);
	}
};

struct UIElement {
	UIType type;
	union {
		UICommonData common;
		UITextData text;
		UILineData line;
		UICircleData circle;
		UIButtonData button;
	};
};